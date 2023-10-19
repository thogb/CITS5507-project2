/**
 * @file simulation_sequential.c
 * 
 * Program to perform a very simplified version of the fish school search 
 * simulation using only one thread.
 * 
 * @author Tao Hu
 */

#include <time.h>
#include <stdio.h>
#include "fish_lake.h"
#include "sim_util.h"

#define SIMULATION_STEPS 100
// #define FISH_AMOUNT 100000
#define FISH_LAKE_WIDTH 200.0f
#define FISH_LAKE_HEIGHT 200.0f

int main(int argc, char const *argv[])
{
    // Since the number of fishes are allocated on the heap at runtime, fish 
    // amount can be dynamic. It would be easier to run the expirement with 
    // the fish amount variable as an program argument.
    if (argc < 2) {
        printf("Require fish amount as the only argument\n Usage: \
         ./simulation_sequential <fish amount>\n");
        return 1;
    }
    
    int fishAmount = atoi(argv[1]);

    if (fishAmount <= 0) {
        printf("Invalid fish amount as argument\n");
        return 1;
    }

    int simulationSteps = SIMULATION_STEPS;

    if (argc >= 3) {
        int argSimulationSteps = atoi(argv[2]);
        if (argSimulationSteps > 0) {
            simulationSteps = argSimulationSteps;
        }
    }

    unsigned int randSeed = time(NULL);

    FishLake* fishLake = fish_lake_new(
        fishAmount, 
        FISH_LAKE_WIDTH, 
        FISH_LAKE_HEIGHT);

    clock_t start = clock();

    // The simulation start with t or i = 0 representing the first time step. 
    // But before this, fish are all initialised with random weight and position 
    for (int i = 0; i < simulationSteps; i++)
    {
        // deltaF should always be greater than 0, so -1.0 is enough
        float maxDeltaF = -1.0f;
        Fish* fishes = fishLake->fishes;

        float objectiveValue = 0.0f;
        float baryCentre = 0.0f;
        float sumOfDistWeight = 0.0f;

        // calc the value of objective function
        for (int i = 0; i < fishAmount; i++)
        {
            objectiveValue += fishes[i].distanceFromOrigin;
        }
        // calculate barycenter of the fish school. In the real simulation I am 
        // guessing this is needed to find the direction for the fish to swim 
        // towards. Hence, barycenter is calculated before the fish swims in 
        // each time step. The equation also uses W(t) to represent the fish 
        // weight used in the barycenter calculation. The following eat and swim
        //  will both be producing W(t+1) and Position(t+1)
        for (int i = 0; i < fishAmount; i++)
        {
            sumOfDistWeight += fishes[i].distanceFromOrigin * fishes[i].weight;
        }

        baryCentre = sumOfDistWeight / objectiveValue;

        // every fish will first swim so deltaF can be calculated
        for (int j = 0; j < fishAmount; j++) {
            // The fish will swim and keep track of the old position, which is
            // used to calculate delta f (the change in objective function).
            // Each fish also stores a deltaF maxDeltaF is updated.
            float deltaF = fish_lake_fish_swim(fishLake, &(fishes[j]), &randSeed);
            maxDeltaF = max_float(maxDeltaF, deltaF);
        }

        // every fish will eat, which requires maxDeltaF
        for (int i = 0; i < fishAmount; i++)
        {
            fish_eat(&(fishes[i]), maxDeltaF);
        }
    }
    
    clock_t end = clock();
    double elapsed_secs = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Elapsed time: %f\n", elapsed_secs);

    fish_lake_free(fishLake);
    return 0;
}
