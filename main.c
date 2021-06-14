/* Standard libs */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

/* Helper libs */
#include "linmath.h"
#include "render_bmp.h"

/* My libs */
#include "linmath_ext.h"
#include "raytracer.h"

/* Include OpenMP (if needed) */
#ifdef USE_OPENMP
#include "omp.h"
#endif

/* Include MPI (if needed) */
#ifdef USE_MPI
#include <mpi.h>
#endif

int main()
{
    /* Image sizes. 1920x1080 is proper, but 1072 I used for benchmarking
       as 1080 is not divisible by 16. */
    const int width = 1920;
    const int height = 1080;
    //const int height = 1072;
    const int fov = 30;

    /* Timing */
#ifdef USE_OPENMP
    double begin = omp_get_wtime();
#else
    clock_t begin = clock();
#endif

#ifdef USE_MPI
     // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    // Find out rank, size
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    printf("I am MPI process %d of %d\n", world_rank, world_size);

    if(height % world_size != 0) {
        printf("image height of %d needs to be divisible by world_size (%d)", height, world_size);
        exit(1);
    }
#endif

    /* Camera Position */
    vec3 eyePos;
    GetEyePosition(eyePos, width, height, fov);
    printf("Eye position is calculated at %f:%f:%f for image of size %d:%d\n", eyePos[0], eyePos[1], eyePos[2], width, height);

    /* 
     * For each pixel in our image, calculate the ray and populate the image
     * with that pixel color
     */
    int i, j, k;

    /* allocate memory. need this dynamic memory or the stack will overflow. */
    float * rawImage = (float *)malloc(height * width * sizeof(vec3));

#ifdef USE_MPI
    int buffer_size = height / world_size;
    float * rawImageBuffer = (float*)malloc(buffer_size * width * sizeof(vec3));
    for (i = 0; i < buffer_size; i++) {
        for (j = 0; j < width; j++) {
            vec3 imageLocation = {(world_rank * buffer_size) + i, j, 0};
            vec3 finalOutput = {0, 0, 0};
            TraceRay(eyePos, imageLocation, finalOutput);
            vec3_dup(&rawImageBuffer[(i*width + j)*3], finalOutput);
        }
    }
    printf("Process %d finished raytracing\n", world_rank);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Gather(rawImageBuffer, buffer_size * width * 3, MPI_FLOAT, 
        rawImage, buffer_size * width * 3, MPI_FLOAT, 0, MPI_COMM_WORLD);
#else
#ifdef USE_OPENMP
    #pragma omp parallel for num_threads(OPENMP_THREAD_AMOUNT) private(i, j) schedule(guided)
#endif
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            vec3 imageLocation = {i, j, 0};
            vec3 finalOutput = {0, 0, 0};
            TraceRay(eyePos, imageLocation, finalOutput);
            vec3_dup(&rawImage[(i*width + j)*3], finalOutput);
        }
    }
#endif

    /* Grab the value range to scale our image by (0-255) */
    float maxLightingValue = 0.0f;

#ifdef USE_MPI
    MPI_Barrier(MPI_COMM_WORLD);
    if(world_rank == 0)
        printf("Calculating maximum lighting value...\n");

    for (i = 0; i < buffer_size; i++) {
        for (j = 0; j < width; j++) {
            for(k = 0; k < 3; k++) {
                maxLightingValue = fmax(maxLightingValue, rawImageBuffer[(i*width + j)*3 + k]);
            }
        }
    }

    /* Grab the maximum lighting value from all the processes and calculate the max one */
    float maxLightValuesAll[world_size];
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Gather(&maxLightingValue, 1, MPI_FLOAT, 
        maxLightValuesAll, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    if(world_rank == 0) {
        printf("Finding maximum lighting value...\n");
        for(i = 0; i < world_size; i++)
        {
            printf("Process %d's max lighting value is %f\n", i, maxLightValuesAll[i]);
            maxLightingValue = fmax(maxLightingValue, maxLightValuesAll[i]);
        }
    }

    /* Send real maximum lighting value to all the processes */
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Bcast(&maxLightingValue, 1, MPI_FLOAT,
        0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    if(world_rank == 0)
        printf("Global maximum lighting value is %f\n", maxLightingValue);

#else
    printf("Calculating maximum lighting value...\n");
#ifdef USE_OPENMP
    #pragma omp parallel for num_threads(OPENMP_THREAD_AMOUNT) private(i, j) reduction(max:maxLightingValue)
#endif
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            for(k = 0; k < 3; k++) {
                maxLightingValue = fmax(maxLightingValue, rawImage[(i*width + j)*3 + k]);
            }
        }
    }
#endif

    /* allocate memory for the final picture */
    unsigned char* image = (unsigned char*)malloc(width * height * 3);

#ifdef USE_MPI

    /* Clamp our lighting values to our 24-bit values for the bitmap */
    unsigned char* imageBuffer = (unsigned char*)malloc(buffer_size * width * 3);
    for (i = 0; i < buffer_size; i++) {
        for (j = 0; j < width; j++) {
            vec3 newPixel;
            vec3_scale(newPixel, &rawImageBuffer[(i*width + j)*3], 255.0f/maxLightingValue);
            imageBuffer[((i*width + j)*3)] = (unsigned char) newPixel[2];
            imageBuffer[((i*width + j)*3)+1] = (unsigned char) newPixel[1];
            imageBuffer[((i*width + j)*3)+2] = (unsigned char) newPixel[0];
        }
    }

    /* Gather all the final data to the root process */
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Gather(imageBuffer, buffer_size * width * 3, MPI_CHAR, 
        image, buffer_size * width * 3, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    /* All other processes terminate at this point */
    if(world_rank != 0) {
        MPI_Finalize();
        return 0;
    }
#else 
    printf("Maximum lighting value: %.2f \n", maxLightingValue);
    /* Clamp our lighting values to our 24-bit values for the bitmap */
#ifdef USE_OPENMP
    #pragma omp parallel for num_threads(OPENMP_THREAD_AMOUNT) private(i, j)
#endif
    for (i = 0; i < height; i++) {
        vec3 newPixel;
        for (j = 0; j < width; j++) {
            vec3_scale(newPixel, &rawImage[(i*width + j)*3], 255.0f/maxLightingValue);
            image[((i*width + j)*3)] = (unsigned char) newPixel[2];
            image[((i*width + j)*3)+1] = (unsigned char) newPixel[1];
            image[((i*width + j)*3)+2] = (unsigned char) newPixel[0];
        }
    }
#endif

clock_t start_saveimg = clock();
printf("Generating final output image...\n");
    /* Generate a human viewable image to a bitmap */
    generateBitmapImage(image, height, width, "rendered.bmp");
    printf("Image generated!!\n");
clock_t end_saveimg = clock();
double time_spent_saveimg = (double)(end_saveimg - start_saveimg) / CLOCKS_PER_SEC;
printf("Img generation Processing Time: %.4f seconds\n", time_spent_saveimg);

#ifdef USE_OPENMP
    double end = omp_get_wtime();
    printf("OpenMP Processing Time: %.4f seconds\n", end - begin);
#else
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Total Processing Time: %.4f seconds\n", time_spent);
#endif


    /* free memory  */
#ifdef USE_MPI
    free(rawImageBuffer);
    free(imageBuffer);
    MPI_Finalize();
#endif
    free(rawImage);
    free(image);

}