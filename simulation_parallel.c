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

#ifdef __APPLE__
    #include "/opt/homebrew/Cellar/libomp/17.0.1/include/omp.h"
#else
    #include <omp.h>
#endif

#define SIMULATION_STEPS 100
// #define FISH_AMOUNT 100000
#define FISH_LAKE_WIDTH 200.0f
#define FISH_LAKE_HEIGHT 200.0f

#if defined(S_DYNAMIC)
    #define S_METHOD dynamic 
    #define S_METHOD_STR "dynamic"
#elif defined(S_GUIDED)
    #define S_METHOD guided
    #define S_METHOD_STR "guided"
#else
    #define S_METHOD static
    #define S_METHOD_STR "static"
#endif

int main(int argc, char const *argv[])
{
    int fishAmount = atoi(argv[1]);
    int simulationSteps = SIMULATION_STEPS;
    unsigned int randSeed = time(NULL);

    // Since the number of fishes are allocated on the heap at runtime, fish 
    // amount can be dynamic. It would be easier to run the expirement with 
    // the fish amount variable as an program argument.
    if (argc < 2) {
        printf("Require fish amount as the only argument\n Usage: \
         ./simulation_sequential <fish amount>\n");
        return 1;
    }

    if (fishAmount <= 0) {
        printf("Invalid fish amount as argument\n");
        return 1;
    }

    if (argc >= 3) {
        int argSimulationSteps = atoi(argv[2]);
        if (argSimulationSteps > 0) {
            simulationSteps = argSimulationSteps;
        }
    }

    FishLake* fishLake = fish_lake_new(
        fishAmount, 
        FISH_LAKE_WIDTH, 
        FISH_LAKE_HEIGHT);

    double start = omp_get_wtime();

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
        #pragma omp parallel for schedule(S_METHOD) reduction(+: objectiveValue)
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
        #pragma omp parallel for schedule(S_METHOD) reduction(+: sumOfDistWeight)
        for (int i = 0; i < fishAmount; i++)
        {
            sumOfDistWeight += fishes[i].distanceFromOrigin * fishes[i].weight;
        }

        baryCentre = sumOfDistWeight / objectiveValue;

        // every fish will first swim so deltaF can be calculated
        #pragma omp parallel firstprivate(randSeed)
        {
            randSeed += omp_get_thread_num();

            #pragma omp for schedule(S_METHOD)
            for (int j = 0; j < fishAmount; j++) {
                // The fish will swim and keep track of the old position, which is
                // used to calculate delta f (the change in objective function).
                // Each fish also stores a deltaF maxDeltaF is updated.
                float deltaF = fish_lake_fish_swim(fishLake, &(fishes[j]), &randSeed);
            }
        }

        // calculate maxDeltaF
        #pragma omp parallel for schedule(S_METHOD) reduction(max: maxDeltaF)
        for (int i = 0; i < fishAmount; i++)
        {
            maxDeltaF = max_float(maxDeltaF, fishes[i].deltaF);
        }

        // every fish will eat, which requires maxDeltaF
        #pragma omp parallel for schedule(S_METHOD)
        for (int i = 0; i < fishAmount; i++)
        {
            fish_eat(&(fishes[i]), maxDeltaF);
        }
    }
    
    double end = omp_get_wtime();
    double elapsed_secs = end - start;

    printf("omp_get_max_threads=%d, schedule=%s, time_taken=%f\n", omp_get_max_threads(), S_METHOD_STR, elapsed_secs);

    fish_lake_free(fishLake);
    return 0;
}
