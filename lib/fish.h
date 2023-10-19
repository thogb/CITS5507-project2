/**
 * @file fish.h
 * 
 * Contains the struct definition of type Fish.
 * 
 * @author Tao Hu
*/

#ifndef FISH_H
#define FISH_H

#include <math.h>
#include "position.h"
#include "sim_util.h"

#define FISH_WEIGHT_MAX_SCALE 2
// assuming fish weight in grams, 10 gram represnet a breed of very small fish.
// and 20kg represents a very large type of fish, with max weight of 40kg or
// 40000.0f
#define FISH_INIT_WEIGHT_MIN 10.0f
#define FISH_INIT_WEIGHT_MAX 20000.0f
#define FISH_SWIM_MIN -0.1f
#define FISH_SWIM_MAX 0.1f

/**
 * @brief Represent a fish in the simulation.
 * 
 * Contains information about a fish.
 */
typedef struct Fish
{
    Position position;
    float distanceFromOrigin;
    float initialWeight;
    float weight;
    float deltaF;
} Fish;

/**
 * Initializes a fish object with the given position.
 *
 * @param fish a pointer to the Fish object to be initialized
 * @param position the position of the fish
 *
 * @return void
 */
void fish_init(Fish* fish, Position position) {
    fish->position = position;
    fish->distanceFromOrigin = position_distance_from_zero(fish->position);
    fish->initialWeight = rand_float(FISH_INIT_WEIGHT_MIN, FISH_INIT_WEIGHT_MAX);
    fish->weight = fish->initialWeight;
    fish->deltaF = 0.0f;
}

/**
 * @brief Performs a fish's swim in the simulation
 * 
 * The input fish will swim in both x and y direction randomly depending on the
 * value defined by FISH_SWIM_MIN and FISH_SWIM_MAX.
 * 
 * @param fish the fish that will perofmr a swim
 * @return Position the new position of the fish
 */
float fish_swim(Fish* fish, Position newPosition) {
    Position position = fish->position;
    // position_increment(
    //     &(fish->position), 
    //     rand_float(FISH_SWIM_MIN, FISH_SWIM_MAX), 
    //     rand_float(FISH_SWIM_MIN, FISH_SWIM_MAX)
    // );
    fish->position = newPosition;    

    // update distanceFromOrigin, will be used later on
    fish->distanceFromOrigin = position_distance_from_zero(fish->position);
    
    // delta f is the difference in objective function before and after the fish
    //  swims. This can be simplified to be the difference between the distance 
    // from origin before and after the fish swims.
    //
    // This is because the objective function is the sum of all the distance
    // from origin of all fish in the simulation. If only the difference of one 
    // fish is interested, then the rest will cancel out.
    fish->deltaF = fabsf(
        fish->distanceFromOrigin -
        position_distance_from_zero(position));

    return fish->deltaF;
}

/**
 * Updates the weight of a fish based on the weight gain. The fish's weight will
 *  not go below 0.0 and will not go above its initial weight * 
 * FISH_WEIGHT_MAX_SCALE.
 *
 * @param fish A pointer to the Fish object.
 * @param maxDeltaF The maximum deltaF of all the fish.
 */
void fish_eat(Fish* fish, float maxDeltaF) {
    float newWeight = fish->weight + (fish->deltaF / maxDeltaF);
    fish->weight = min_float(
        max_float(
            newWeight,
            FISH_INIT_WEIGHT_MIN
        ),
        fish->initialWeight * FISH_WEIGHT_MAX_SCALE
    );
}

#endif
