#include "scene.h"

/*
 * We generate the same scene everytime.
 * This makes it easier to benchmark for the project.
 * Since it's small enough for this program,
 * we can get away with generating it on the fly
 */
struct Scene NewScene() 
{
    struct Scene scene;

    /* lights */
    {
        const int index = 0;
        vec3 position = {-100, 1300, -250};
        vec3_dup(scene.lights[index].position, position);
        vec3 color = {225, 100, 70};
        vec3_dup(scene.lights[index].color, color);
        scene.lights[index].intensity = 20000;
    }

    {
        const int index = 1;
        vec3 position = {500, 400, 0};
        vec3_dup(scene.lights[index].position, position);
        vec3 color = {102, 100, 255};
        vec3_dup(scene.lights[index].color, color);
        scene.lights[index].intensity = 12000;
    }

    {
        const int index = 2;
        vec3 position = {300, 1000, 500};
        vec3_dup(scene.lights[index].position, position);
        vec3 color = {20, 255, 20};
        vec3_dup(scene.lights[index].color, color);
        scene.lights[index].intensity = 1000;
    }

    /* circles */
    {
        const int index = 0;
        vec3 origin = {1000.0f, 500.0f, 600.0f};
        vec3_dup(scene.circles[index].origin, origin);
        scene.circles[index].radius = 300.0f;
    }

    {
        const int index = 1;
        vec3 origin = {-400.0f, 500.0f, 800.0f};
        vec3_dup(scene.circles[index].origin, origin);
        scene.circles[index].radius = 400.0f;
    }
    
    {
        const int index = 2;
        vec3 origin = {100.0f, 1400.0f, 600.0f};
        vec3_dup(scene.circles[index].origin, origin);
        scene.circles[index].radius = 300.0f;
    }

    {
        const int index = 3;
        vec3 origin = {-200.0f, 500.0f, 700.0f};
        vec3_dup(scene.circles[index].origin, origin);
        scene.circles[index].radius = 50.0f;
    }


    /* planes */
    {
        const int index = 0;
        vec3 origin = {0, 0, 950.0f};
        vec3_dup(scene.planes[index].origin, origin);
        vec3 normal = {0, 0, 1};
        vec3_norm(normal, normal);
        vec3_dup(scene.planes[index].normal, normal);
    }

    {
        const int index = 1;
        vec3 origin = {0, 0, 1000.0f};
        vec3_dup(scene.planes[index].origin, origin);
        vec3 normal = {0, -1, 1};
        vec3_norm(normal, normal);
        vec3_dup(scene.planes[index].normal, normal);
    }

    {
        const int index = 2;
        vec3 origin = {400.0f, 1700.0f, 1000.0f};
        vec3_dup(scene.planes[index].origin, origin);
        vec3 normal = {0, 1, 1};
        vec3_norm(normal, normal);
        vec3_dup(scene.planes[index].normal, normal);
    }

    {
        const int index = 3;
        vec3 origin = {700.0f, 0, 1000.0f};
        vec3_dup(scene.planes[index].origin, origin);
        vec3 normal = {1, 0, 1};
        vec3_norm(normal, normal);
        vec3_dup(scene.planes[index].normal, normal);
    }

    {
        const int index = 4;
        vec3 origin = {700.0f, 0, 1000.0f};
        vec3_dup(scene.planes[index].origin, origin);
        vec3 normal = {1, 0, 1};
        vec3_norm(normal, normal);
        vec3_dup(scene.planes[index].normal, normal);
    }

    return scene;
}