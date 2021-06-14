#ifndef SCENE_H_
#define SCENE_H_

#include "linmath.h"

/* 
 * Since scenes are static,
 * we can make it easy and statically allocate everything
 */
#define NUM_CIRCLES 4
#define NUM_PLANES 5
#define NUM_LIGHTS 3

/* Represents a circle primative */
struct SceneCircle {
    vec3 origin;
    float radius;
};

/* Represents a plane primative */
struct ScenePlane {
    vec3 origin;
    vec3 normal;
};

/* Represents a scene light */
struct SceneLight {
    vec3 position;
    float intensity;
    vec3 color;
};

/* Represents a scene */
struct Scene {
    struct SceneCircle circles[NUM_CIRCLES];
    struct ScenePlane planes[NUM_PLANES];
    struct SceneLight lights[NUM_LIGHTS];
};

struct Scene NewScene();

#endif