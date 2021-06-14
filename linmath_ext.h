/* 
 * The provided linmath library misses a few functions so
 * I've added a few here
 */

#ifndef LINMATH_EXT_H
#define LINMATH_EXT_H

/* makes it easy to print a vector3 */
void vec3_print(float* vecName, int includeNewLine);

/* zeroes out a vector3 */
void vec3_zero(float *vecName);


#endif