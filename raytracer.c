/* Default libraries */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Helper libraries */
#include "linmath.h"

/* My libraries */
#include "linmath_ext.h"
#include "raytracer.h"
#include "scene.h"

/* Variables for debugging the math */
static int DEBUG_COORDINATE_X = 200;
static int DEBUG_COORDINATE_Y = 200;
static int DEBUG_RAY_IMAGE = 0;

/* 
 * Gets the eye position for the camera based on the field of view
 * Since field of view is more intuitive, we do some tmath to position our eye behind the screen.]
 * This also has the benefit of scaling to whatever imageWidth and imageHeigh we use
 */
void GetEyePosition(float* eyePos, const int imageWidth, const int imageHeight, const int fieldOfView)
{
    const float pi = 3.1415926f;
    
    /* To get the depth of the camera, we're gonna solve based on the FOV and imageWidth using trig */
    float fovInRadians = ((float)fieldOfView/360.0f) * pi * 2;
    float theta = pi - (pi/2) - fovInRadians; 
    float dist = tan(theta) * (imageWidth/2);

    /* Return the eye position. Remember that the camera is behind the scene, so -dist is required */
    eyePos[0] = imageWidth / 2;
    eyePos[1] = imageHeight / 2;
    eyePos[2] = -dist;
}

struct Ray InitRay()
{
    struct Ray ray;
    vec3_zero(ray.origin);
    vec3_zero(ray.direction);
    ray.validRay = 0;
    return ray;
}


/* 
 * Calculate intersections for spheres 
 * Will return "2" if the ray completely misses
 *     - outNewRay and outCollisionNormalRay will be invalid
 * Will return "1" if the ray is bounces. 
 *     - outNewRay is the bounced array. 
 *     - outCollisionNormal ray is populated
 *     - distance is populated
 * Will return "0" if the ray is fully absorbed by the material. 
 *     - outNewRay is will be invalid.
 *     - outCollisionNormal ray is populated 
 *     - distance is populated
 */
int CalculateCircleCollision(struct Ray* originalRay, float* sphereCenter, float sphereRadius, struct Ray* outNewRay, struct Ray* outCollisionNormalRay, float* outDistance)
{
    /*
     * So we can represent a sphere like X^2 + Y^2 + Z^2 = R^2
     * and the originalRay like U + Vx.
     * Combining both and formatting it as a quadratic equation with x being x in Vx above,
     * we can solve this using the quadratic formula and the disciminant will tell us
     * if we have a solution
     */
    vec3 deltaVec;
    vec3_sub(deltaVec, (*originalRay).origin, sphereCenter);
    
    /* Quadratic formula solving variables */
    float a = vec3_mul_inner((*originalRay).direction, (*originalRay).direction);
    float b = 2.0f * vec3_mul_inner((*originalRay).direction, deltaVec);
    float c = vec3_mul_inner(deltaVec, deltaVec) - (sphereRadius * sphereRadius);
    float discrim = b*b - 4.0f*a*c;

    /* Default behaviour is invalid rays */
    (*outNewRay).validRay = 0;
    (*outCollisionNormalRay).validRay = 0;
    
    if(DEBUG_RAY_IMAGE) 
        printf("discrim: %.2f\n", discrim);

    /* If discriminant < 0, no solution. Ray misses. */
    if(discrim < 0) {
        return 2;
    }

    /* Otherwise solve quadratic for both roots */
    float sol_a = (-b - sqrt(discrim)) / (2.0f * a);
    float sol_b = (-b + sqrt(discrim)) / (2.0f * a);

    /* Take the smaller of the too (if needed) */
    float final_sol = sol_a;
    if(sol_b < sol_a)
        final_sol = sol_b;
    
    if(DEBUG_RAY_IMAGE)
        printf("quadratic solution 1: %.2f | solution 2: %.2f | final: %.2f\n", 
            sol_a, sol_b, final_sol);
    
    /* This shouldn't happen. This means the sphere is behind the ray */
    if(final_sol < 0) {
        if(DEBUG_RAY_IMAGE)
            printf("ray is behind camera! (ray misses)\n");
        return 2;
    }

    /* 
     * Calculate the point of intersection.
     * Since we calculated x (_final_sol) for U + Yx, 
     * we can get the exact point of intersection!
     */
    vec3 hitIntersection;
    vec3_scale(hitIntersection, (*originalRay).direction, final_sol);
    vec3_add(hitIntersection, hitIntersection, (*originalRay).origin);
    if(DEBUG_RAY_IMAGE) {
        printf("collisionPoint: ");
        vec3_print(hitIntersection, 1);
    }

    /* calculate the normal from the intersection point */
    vec3 hitNormal;
    vec3_sub(hitNormal, hitIntersection, sphereCenter);
    vec3_norm(hitNormal, hitNormal);
    if(DEBUG_RAY_IMAGE) {
        printf("collision normal vector: ");
        vec3_print(hitNormal, 1);
    }

    /* Update output collision normal ray */
    vec3_dup((*outCollisionNormalRay).direction, hitNormal);
    vec3_dup((*outCollisionNormalRay).origin, hitIntersection);
    (*outCollisionNormalRay).validRay = 1;
    *outDistance = final_sol;

    /* flipped normal */
    vec3 flippedNormal;
    vec3_scale(flippedNormal, hitNormal, -1.0f);

    /* Implement bouncing rays */
    vec3 reflectedVector;
    vec3_reflect(reflectedVector, (*originalRay).direction, flippedNormal);
    vec3_norm(reflectedVector, reflectedVector);
    vec3_dup((*outNewRay).origin, hitIntersection);
    vec3_dup((*outNewRay).direction, reflectedVector);
    (*outNewRay).validRay = 1;


    return 1;
    
}

/* 
 * Calculate intersections for planes 
 * Will return "2" if the ray completely misses
 *     - outNewRay and outCollisionNormalRay will be invalid
 * Will return "1" if the ray is bounces. 
 *     - outNewRay is the bounced array. 
 *     - outCollisionNormal ray is populated
 *     - distance is populated
 * Will return "0" if the ray is fully absorbed by the material. 
 *     - outNewRay is will be invalid.
 *     - outCollisionNormal ray is populated 
 *     - distance is populated
 */
int CalculatePlaneCollision(struct Ray* originalRay, float* planeOrigin, float* planeNormal, struct Ray* outNewRay, struct Ray* outCollisionNormalRay, float* outDistance)
{
    outNewRay->validRay = 0;
    outCollisionNormalRay->validRay = 0;

    float denominator = vec3_mul_inner(planeNormal, originalRay->direction);

    if(DEBUG_RAY_IMAGE) {
        printf("plane normal:");
        vec3_print(planeNormal, 1);
        printf("original ray denominator:");
        vec3_print(originalRay->direction,1 );
        printf("plane denominator: %.2f\n", denominator);

    }

    /* infinite or no solutions. regardless, ray misses */
    if(denominator < 0.0001f) 
        return 2;
    vec3 deltaPosition;
    vec3_sub(deltaPosition, planeOrigin, originalRay->origin);
    float numerator = vec3_mul_inner(deltaPosition, planeNormal);
    
    /* solve x for U + Vx which also is the distance */
    *outDistance = numerator / denominator;
    vec3 collisionPoint;
    vec3_scale(collisionPoint, originalRay->direction, *outDistance);
    vec3_add(collisionPoint, collisionPoint, originalRay->origin);

    /* collision normal ray is easy: just the plane ray */
    /* flip the normal though */
    vec3 flippedPlaneNormal;
    vec3_scale(flippedPlaneNormal, planeNormal, -1.0f);
    vec3_dup(outCollisionNormalRay->direction, flippedPlaneNormal);
    vec3_dup(outCollisionNormalRay->origin, collisionPoint);
    outCollisionNormalRay->validRay = 1;

    /* Handle reflections */
    vec3 reflectedVector;
    vec3_reflect(reflectedVector, (*originalRay).direction, flippedPlaneNormal);
    vec3_norm(reflectedVector, reflectedVector);
    vec3_dup((*outNewRay).origin, collisionPoint);
    vec3_dup((*outNewRay).direction, reflectedVector);
    (*outNewRay).validRay = 1;

    return 1;
}

void CalculateLighting(struct Ray collisionPointNormal, float* outRayColor, float* outputReflectedPhotons)
{
    struct Scene scene = NewScene();
    
    int i;
    vec3 finalColor = {0, 0, 0};

    for(i = 0; i < NUM_LIGHTS; i++) {

        /* calculate direction and distance to our point light source */
        vec3 lightDirection;
        vec3_sub(lightDirection, scene.lights[i].position, collisionPointNormal.origin);
        float distanceToLightSource = vec3_len(lightDirection);
        vec3_norm(lightDirection, lightDirection);

        if(DEBUG_RAY_IMAGE) {
            printf("light direction: ");
            vec3_print(lightDirection, 1);
        }

        /* calculate light intensity based off the inverse square law */
        vec3 lightIntensityVec;
        float lightIntensityDenominator = 4.0f * 3.14159f * distanceToLightSource * distanceToLightSource;
        vec3_scale(lightIntensityVec, scene.lights[i].color, (scene.lights[i].intensity / lightIntensityDenominator));
        if(DEBUG_RAY_IMAGE) {
            printf("light intensity vector: ");
            vec3_print(lightIntensityVec, 1);
        }

        /* render material color based on albeto and collision normal from the collision */
        const float albeto = 0.2f;
        vec3 appliedColor;
        if(DEBUG_RAY_IMAGE) {
            printf("single ray photons %.3f: ", *outputReflectedPhotons);
            vec3_print(lightIntensityVec, 1);
        }
        vec3_scale(appliedColor, lightIntensityVec, ((albeto) / 3.14159f));
        vec3_scale(appliedColor, appliedColor, *outputReflectedPhotons);
        

        /* Apply diffuse angle and add to final lighting */
        float diffuseAngle = vec3_mul_inner(lightDirection, collisionPointNormal.direction);
        vec3 angledColor;
        vec3_scale(angledColor, appliedColor, fmax(0.0f, diffuseAngle));
        vec3_add(finalColor, finalColor, angledColor);

        if(DEBUG_RAY_IMAGE) {
            printf("diffuse angle: %f\n", diffuseAngle);
            printf("applied color: ");
            vec3_print(appliedColor, 1);
            printf("final color: ");
            vec3_print(finalColor, 1);
        }
    }

    /* set the final color*/
    vec3_dup(outRayColor, finalColor);
}

void TraceSingleRay(struct Ray currentRay, struct Ray* outputRay, float* outRayColor, float* outputReflectedPhotons)
{
    
    /* we're going to iterate over the scene and only get the closest collision */
    struct Scene currentScene = NewScene();

    int i;
    float minDistance = 1000000.0f;
    struct Ray minDistanceNormalRay = InitRay();
    struct Ray minDistanceOutputRay = InitRay();
    minDistanceNormalRay.validRay = 0;
    minDistanceOutputRay.validRay = 0;

    /* go through circles first */
    for(i = 0; i < NUM_CIRCLES; i++) {
        struct Ray newRay = InitRay();
        struct Ray collisionNormalRay = InitRay();
        float distanceToCollision = 0;

        float testRayResult = CalculateCircleCollision(&currentRay, currentScene.circles[i].origin, currentScene.circles[i].radius, 
            &newRay, &collisionNormalRay, &distanceToCollision);

        if(collisionNormalRay.validRay && distanceToCollision < minDistance) {
            minDistanceNormalRay = collisionNormalRay;
            minDistance = distanceToCollision;
            minDistanceOutputRay = newRay;
        }
    }

    /* go through planes next */
    for(i = 0; i < NUM_PLANES; i++) {
        struct Ray newRay = InitRay();
        struct Ray collisionNormalRay = InitRay();
        float distanceToCollision = 0;

        float testRayResult = CalculatePlaneCollision(&currentRay, currentScene.planes[i].origin, currentScene.planes[i].normal, 
            &newRay, &collisionNormalRay, &distanceToCollision);

        if(collisionNormalRay.validRay && distanceToCollision < minDistance) {
            minDistanceNormalRay = collisionNormalRay;
            minDistance = distanceToCollision;
            minDistanceOutputRay = newRay;
        }
    }

    /* Calculate lighting */
    if(minDistanceNormalRay.validRay) {
        CalculateLighting(minDistanceNormalRay, outRayColor, outputReflectedPhotons);
    }

    if(minDistanceOutputRay.validRay) {
        *outputRay = minDistanceOutputRay;
    }

    /* hacky way to temper reflections */
    const float reflective = 0.9f;
    *outputReflectedPhotons = *outputReflectedPhotons * reflective;
}

/* Sends a ray through a screen pixel */
void TraceRay(float* eyePos, float* screenPixel, float* outRayColor)
{
    /* enable debug if we are on the debug pixel */
    DEBUG_RAY_IMAGE = 0;
    if(screenPixel[0] == DEBUG_COORDINATE_X && screenPixel[1] == DEBUG_COORDINATE_Y)
        DEBUG_RAY_IMAGE = 1;

    /* calculate the direction of vector */
    vec3 direction;
    vec3_sub(direction, screenPixel, eyePos);
    vec3 norm_direction;
    vec3_norm(norm_direction, direction);

    /* 
     * Convert it to a Ray. 
     * It's important that direction is normalized (as described in the PBR book) as 
     * it's one of the common pitfalls
     */
    struct Ray currentRay = InitRay();
    vec3_dup(currentRay.origin, eyePos);
    vec3_dup(currentRay.direction, norm_direction);

    /*
     * For tracing the ray. We loop to trace each reflection of the ray too
     */
    struct Ray outputRay = InitRay();
    float outputReflectedPhotons = 1.0f;
    int i;
    for(i = 0; i < MAX_RAY_REFLECTIONS; i++) {

        /* Trace the path */
        vec3 currentColor;
        vec3_zero(currentColor);
        TraceSingleRay(currentRay, &outputRay, currentColor, &outputReflectedPhotons);
        vec3_add(outRayColor, outRayColor, currentColor);
        if(DEBUG_RAY_IMAGE) {
            printf("output photons: %.2f, reflection %d color: ", outputReflectedPhotons, i);
            vec3_print(currentColor, 1);
        }

        /* If we don't get a reflected ray, end here */
        if(!outputRay.validRay)
            break;

        /* Otherwise set currentRay to our output ray and go again */
        currentRay = outputRay;
    }

    if(DEBUG_RAY_IMAGE) {
        printf("ending outRayColor: ");
        vec3_print(outRayColor, 1);
    }
}