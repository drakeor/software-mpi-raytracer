/*
 * I did not priginally write these functions. They simply convert an image to a BMP
 * They are modified to work with my code though.
 * Source: https://stackoverflow.com/questions/2654480/writing-bmp-image-in-pure-c-c-without-other-libraries
 */
#ifndef RENDER_BMP_H_
#define RENDER_BMP_H_
#define BYTES_PER_PIXEL 3

#include "linmath.h"
#include "raytracer.h"

void generateBitmapImage(unsigned char* image, int height, int width, char* imageFileName);
//void generateBitmapImage(struct Pixel* image, int height, int width, char* imageFileName);
unsigned char* createBitmapFileHeader(int height, int stride);
unsigned char* createBitmapInfoHeader(int height, int width);

#endif