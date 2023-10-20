/**
 * @file mpi_mp_test.c
 * 
 * Performs the simplified version of the fish school search simulation. The 
 * simulation will be ran on multiple processes using MPI for message passing. 
 * Each process will also be ran using multiple threads using OMP.
 * 
 * @author Tao Hu
 */

#include <stdio.h>
#include <time.h>
#include <mpi.h>
#include <omp.h>

#include "../lib/fish_lake.h"
#include "../lib/sim_util.h"
#include "../lib/work_parition.h"
#include "../lib/mpi_util.h"

#define SIMULATION_STEPS 100
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

#define MASTER_RANK 0

int main(int argc, char *argv[])
{
    // The fishlake containing all fishes, global
    FishLake* fishLake;
    // The fishlake containg a subset of all fishes, used by worker processes, 
    // local
    FishLake* localFishLake;
    // Substitution for fishlake->fishes, the worker processes do not intialise 
    // fishLake. Hence no access to fishLake->fishes when using Gatherv
    Fish* allFishes;
    WorkPartition* workPartition;

    // The global amount of fishes
    int fishAmount = atoi(argv[1]);
    // Number of times the simulation will run
    int simulationSteps = SIMULATION_STEPS;
    // The seed to be used
    unsigned int randSeed = time(NULL);
    // Start time of simulation
    double start;
    // End time of simulation
    double end;
    // Duration of the simulation
    double elapsed_secs;
    // The final calculated barycentre
    float barycentre;
    // Used for easier access, instead of using localLake->fishes
    Fish* fishes;
    // Used by OMP to calculated the local objective value
    float objectiveValue;
    // Used by OMP to calculate the local sum of distance * weight
    float sumOfDistWeight;

    // dfo = distance from the origin
    // The barycentre equation is the sum of dfo times weight divided by 
    // the objective function values, which is the sum of the distance dfo
    // Both is a summation problem and can be stored together and send over to 
    // all processes through a single MPI_Allreduce call.
    
    // Represents the numerator and the denominator of the barycentre equation. 
    // The first value is numerator the second value is the denominator
    float globalBarycenterVals[2];
    float localBarycenterVals[2];

    float globalMaxDeltaf;
    float localMaxDeltaf;

    int pRank;
    int wSize;

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

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pRank);
    MPI_Comm_size(MPI_COMM_WORLD, &wSize);

    // Initialise the custom data types with MPI
    mpi_util_init_all_types();

    if (wSize < 2) {
        printf("Program requires at least 2 processes\n");
        return 1;
    }
    
    // The master process intialises all the fishes.
    if (pRank == MASTER_RANK) {
        printf("Program running with %d processes\n", wSize);
        
        // Intialising all the fishes
        fishLake = fish_lake_new(
            fishAmount, 
            FISH_LAKE_WIDTH, 
            FISH_LAKE_HEIGHT);
        fish_lake_init_fishes(fishLake);
        printf("Initialised the fish lake\n");

    }

    printf("Process %d is running with %d thread\n", pRank, omp_get_max_threads());
    start = omp_get_wtime();
    // Create the work parition information
    workPartition = work_parition_new(wSize, fishAmount, pRank);
    if (pRank == MASTER_RANK) {
        for (int i = 0; i < workPartition->paritionCount; i++)
        {
            printf("Process %d is assigned workload of %d fishes with offset "
            "of %d\n", i, workPartition->sizes[i], workPartition->offsets[i]);
        }
    }

    // Intialise the local fish lake based on the parition size of each process
    localFishLake = fish_lake_new(
        workPartition->size, 
        FISH_LAKE_WIDTH, 
        FISH_LAKE_HEIGHT);

    // Worker process does not intialise the fishLake so fishlake->fishes would 
    // cause memory segmentation fault.
    if (pRank == MASTER_RANK) allFishes = fishLake->fishes;
    
    // Scatterv is used to send uneven amount of partition data to different 
    // worker processes
    MPI_Scatterv(
        allFishes,
        workPartition->sizes,
        workPartition->offsets,
        MPI_SIM_FISH,
        localFishLake->fishes,
        workPartition->size,
        MPI_SIM_FISH,
        MASTER_RANK,
        MPI_COMM_WORLD
    );

    fishes = localFishLake->fishes;
    // Just making sure every process also gets differnt seed.
    randSeed += 500 * pRank;

    // === Start of simulation ===

    // The simulation start with t or i = 0 representing the first time step. 
    // But before this, fish are all initialised with random weight and position 
    for (int i = 0; i < simulationSteps; i++)
    {
        objectiveValue = 0;
        sumOfDistWeight = 0;
        localMaxDeltaf = INT32_MIN;

        // calc the value of objective function
        #pragma omp parallel for schedule(S_METHOD) reduction(+: objectiveValue)
        for (int i = 0; i < workPartition->size; i++)
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
        for (int i = 0; i < workPartition->size; i++)
        {
            sumOfDistWeight += fishes[i].distanceFromOrigin * fishes[i].weight;
        }

        localBarycenterVals[0] = sumOfDistWeight;
        localBarycenterVals[1] = objectiveValue;

        // The barycentre can only be calculated if all the values are available
        //  This is a summation problem, so the MPI_Allreduce can be used.
        MPI_Allreduce(
            localBarycenterVals,
            globalBarycenterVals,
            2,
            MPI_FLOAT,
            MPI_SUM,
            MPI_COMM_WORLD
        );

        barycentre = globalBarycenterVals[0] / globalBarycenterVals[1];

        // every fish will first swim so deltaF can be calculated
        #pragma omp parallel firstprivate(randSeed)
        {
            randSeed += omp_get_thread_num();

            #pragma omp for schedule(S_METHOD)
            for (int j = 0; j < workPartition->size; j++) {
                // The fish will swim and keep track of the old position, which is
                // used to calculate delta f (the change in objective function).
                // Each fish also stores a deltaF maxDeltaF is updated.
                float deltaF = fish_lake_fish_swim(localFishLake, &(fishes[j]), &randSeed);
            }
        }

        // calculate maxDeltaF
        #pragma omp parallel for schedule(S_METHOD) reduction(max: localMaxDeltaf)
        for (int i = 0; i < workPartition->size; i++)
        {
            localMaxDeltaf = max_float(localMaxDeltaf, fishes[i].deltaF);
        }

        // Find the global max deltaf, which is required for fish eat.
        MPI_Allreduce(
            &localMaxDeltaf,
            &globalMaxDeltaf,
            1,
            MPI_FLOAT,
            MPI_MAX,
            MPI_COMM_WORLD
        );

        // every fish will eat, which requires maxDeltaF
        #pragma omp parallel for schedule(S_METHOD)
        for (int i = 0; i < workPartition->size; i++)
        {
            fish_eat(&(fishes[i]), globalMaxDeltaf);
        }
    }

    // === End of simulation ===
    end = omp_get_wtime();
    elapsed_secs = end - start;
    if (pRank == MASTER_RANK) {
        printf("omp_get_max_threads=%d, schedule=%s, time_taken=%f\n", 
            omp_get_max_threads(), S_METHOD_STR, elapsed_secs);

    }
    
    // Gatherv would allow the master process to gather the data back
    MPI_Gatherv(
        localFishLake->fishes,
        workPartition->size,
        MPI_SIM_FISH,
        allFishes,
        workPartition->sizes,
        workPartition->offsets,
        MPI_SIM_FISH,
        MASTER_RANK,
        MPI_COMM_WORLD
    );

    // === Clean ups by freeing up all memories ===
    // Master process free all fishes
    if (pRank == MASTER_RANK) {
        fish_lake_free(fishLake);
    }

    work_parition_free(workPartition);
    fish_lake_free(localFishLake);

    //MPI gives warning for not freeing commited types
    mpi_util_free_all_types();    
    MPI_Finalize();
    return 0;
}
