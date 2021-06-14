#include <stdio.h>
#include "linmath_ext.h"

/* makes it easy to print a vector3 */
void vec3_print(float* vecName, int includeNewLine)
{
    printf("(%.2f, %.2f, %.2f)", vecName[0], vecName[1], vecName[2]);
    if(includeNewLine)
        printf("\n");
}

/* zeroes out a vector3 */
void vec3_zero(float *vecName)
{
    vecName[0] = 0.0f;
    vecName[1] = 0.0f;
    vecName[2] = 0.0f;
}
