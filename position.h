/**
 * @file position.h
 * 
 * Contains struct definition of type Position and function to modify position
 * pointers.
 * 
 * @author Tao Hu
*/

#ifndef POSITION_H
#define POSITION_H

#include <math.h>

/**
 * @brief Represents a 2d position.
 * 
 * Contains a x and y value.
 */
typedef struct Position
{
    float x;
    float y;
} Position;

/**
 * Calculates the Euclidean distance of a given position from the origin (0, 0) 
 * in a 2D plane.
 *
 * @param position A Position object containing the x and y coordinates.
 *
 * @return The distance from the origin to the given position.
 */
float position_distance_from_zero(Position position) {
    float x = position.x;
    float y = position.y;
    return sqrt(x * x + y * y);
}

/**
 * Increments the position of a given Position object by the specified x and y 
 * values.
 *
 * @param position a pointer to the Position object to be modified
 * @param x the amount to increment the x coordinate.
 * @param y the amount to increment the y coordinate.
 */
void position_increment(Position* position, float x, float y) {
    position->x = position->x + x;
    position->y = position->y + y;
}

#endif
