#ifndef RAYTRACER_H_
#define RAYTRACER_H_

/* easier to manipulate values this way */
/* I had issues with gcc padding so declaring this as packed helped and made it work
   with the image rendering functions */
struct Pixel {
    unsigned char blue;
    unsigned char green;
    unsigned char red;
} __attribute__((__packed__));


/* Easy struct to represent a ray */
struct Ray {
    vec3 origin;
    vec3 direction;
    int validRay;
};

/* How many times a ray is allowed to reflect */
#define MAX_RAY_REFLECTIONS 20

void GetEyePosition(float* eyePos, const int imageWidth, const int imageHeight, const int fieldOfView);

struct Ray InitRay();

int CalculateCircleCollision(struct Ray* originalRay, float* sphereCenter, float sphereRadius, struct Ray* outNewRay, struct Ray* outCollisionNormalRay, float* outDistance);

int CalculatePlaneCollision(struct Ray* originalRay, float* planeOrigin, float* planeNormal, struct Ray* outNewRay, struct Ray* outCollisionNormalRay, float* outDistance);

void CalculateLighting(struct Ray collisionPointNormal, float* outRayColor, float* outputReflectedPhotons);

void TraceRay(float* eyePos, float* screenPixel, float* outRayColor);

#endif 