/**
 * @file fish_lake.h
 * 
 * Contains the struct definition of type FishLake and functions to create, 
 * modify and free from heap.
 * 
 * @author Tao Hu
*/

#ifndef FISH_LAKE_H
#define FISH_LAKE_H

#include <stdlib.h>
#include "fish.h"

/**
 * @brief Fishlake in the simulation.
 * 
 * Contains 
 */
typedef struct FishLake
{
    float coord_min_x;
    float coord_max_x;
    float coord_min_y;
    float coord_max_y;
    int fish_amount;
    Fish* fishes;
} FishLake;

/**
 * Creates a new instance of FishLake with the specified fish amount.
 *
 * @param fish_amount the amount of fish in the lake
 *
 * @return a pointer to the newly created FishLake instance
 */
FishLake* fish_lake_new(
    int fish_amount,
    float width,
    float height
    ) {
    FishLake* fishLake = (FishLake*) malloc(sizeof(FishLake));

    float half_width = width / 2.0f;
    float half_height = height / 2.0f;

    fishLake->fish_amount = fish_amount;
    fishLake->fishes = (Fish*) malloc(fish_amount * sizeof(Fish));
    fishLake->coord_min_x = -half_width;
    fishLake->coord_max_x = half_width;
    fishLake->coord_min_y = -half_height;
    fishLake->coord_max_y = half_height;

    // give each fish a random coordinate
    for (int i = 0; i < fish_amount; i++) {
        Position pos = {
            rand_float(
            fishLake->coord_min_x,
            fishLake->coord_max_x
            ),
            rand_float(
            fishLake->coord_min_y,
            fishLake->coord_max_y
            )};
        // initialise the fish, also sets a random weight
        fish_init(&(fishLake->fishes[i]), pos);
    }

    return fishLake;
}

/**
 * Frees the memory allocated for a FishLake object.
 *
 * @param fishLake the pointer to the FishLake object to be freed
 */
void fish_lake_free(FishLake* fishLake) {
    free(fishLake->fishes);
    free(fishLake);
}

/**
 * The fish lake responsible for controlling how a fish swims in the lake. The 
 * new position of the fish will be checked against the boundaries. If one of 
 * the coordinate is outside the boundaries, the position will be set to the 
 * coordinates of the old position.
 *
 * @param fishLake a pointer to the FishLake object containing the fish
 * @param fish a pointer to the fish
 *
 * @return The deltaF produced after the fish swims
 */
float fish_lake_fish_swim(FishLake* fishLake, Fish* fish, unsigned int * seed) {
    Position position = fish->position;
    Position newPosition = position;
    position_increment(
        &newPosition, 
        rand_r_float(seed, FISH_SWIM_MIN, FISH_SWIM_MAX), 
        rand_r_float(seed, FISH_SWIM_MIN, FISH_SWIM_MAX)
    );

    if (!f_is_between(newPosition.x, fishLake->coord_min_x, fishLake->coord_max_x)) {
        newPosition.x = position.x;
    }

    if (!f_is_between(newPosition.y, fishLake->coord_min_y, fishLake->coord_max_y)) {
        newPosition.y = position.y;
    }

    return fish_swim(fish, newPosition);
}

#endif
