/**
 * @file sim_util.h
 * 
 * Contains helping functions that will be commonly used in this project
 * 
 * @author Tao Hu
*/

#ifndef SIM_UTIL_H
#define SIM_UTIL_H

#include <stdlib.h>

/**
 * Generates a random float between min and max
 *
 * @param min the minimum value for the random float
 * @param max the maximum value for the random float
 *
 * @return a random float between the minimum and maximum values
 */
float rand_float(float min, float max) {
    float randFloat = (float) rand() / (float) RAND_MAX;
    return min + randFloat * (max - min);
}

/**
 * Generates a random float between min and max
 *
 * @param seed the seed for the random number generator
 * @param min the minimum value for the random float
 * @param max the maximum value for the random float
 *
 * @return a random float between the minimum and maximum values
 */
float rand_r_float(unsigned int * seed, float min, float max) {
    float randFloat = (float) rand_r(seed) / (float) RAND_MAX;
    return min + randFloat * (max - min);
}

/**
 * Returns the maximum value between two floats.
 *
 * @param a the first float value
 * @param b the second float value
 *
 * @return the maximum value between a and bs
 */
float max_float(float a, float b) {
    return a > b ? a : b;
}

/**
 * Returns the minimum of two float values.
 *
 * @param a the first float value
 * @param b the second float value
 *
 * @return the smaller of the two float values
 */
float min_float(float a, float b) {
    return a < b ? a : b;
}

/**
 * Checks if a given value is within a specified range.
 *
 * @param value the value to check
 * @param min the minimum value of the range (inclusive)
 * @param max the maximum value of the range (inclusive)
 *
 * @return true if the value is within the specified range, false otherwise
 */
int f_is_between(float val, float min, float max) {
    return (val >= min && val <= max);
}

#endif