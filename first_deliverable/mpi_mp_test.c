/**
 * @file mpi_mp_test.c
 * 
 * Perform a simple message passing of a list of the fish structures. Two output
 *  files are created one after fish intialisation and one after receiving data 
 * from worker processes.
 * 
 * @author Tao Hu
 */

#include <stdio.h>
#include <mpi.h>

#include "../lib/fish_lake.h"
#include "../lib/sim_util.h"
#include "../lib/work_parition.h"
#include "../lib/mpi_util.h"

// Define odd number of fishes to test the edge case
#define FISH_AMOUNT 1111
#define MASTER_RANK 0
#define FISH_LAKE_WIDTH 200.0f
#define FISH_LAKE_HEIGHT 200.0f

/**
 * Write the positions of fish in the fish lake to a file.
 *
 * @param filename The name of the file to write the positions to.
 * @param fishLake A pointer to the FishLake struct containing the fish positions.
 */
static void write_fish_positions( char* filename, FishLake* fishLake) {
    FILE *file = fopen(filename, "w+");

    for (int i = 0; i < fishLake->fish_amount; i++) {
        Fish* fish = &fishLake->fishes[i];
        fprintf(file, "%f,%f\n", fish->position.x, fish->position.y);
    }

    fclose(file);
}

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

    int pRank;
    int wSize;
    WorkPartition* workParition;

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
        FISH_AMOUNT, 
        FISH_LAKE_WIDTH, 
        FISH_LAKE_HEIGHT);
        fish_lake_init_fishes(fishLake);
        printf("Initialised the fish lake\n");
        
        // Write the fish position to out1.txt
        write_fish_positions("out1.txt", fishLake);
        printf("Wrote the fish positions to out1.txt\n");
    }

    // Create the work parition information
    workParition = work_parition_new(wSize, FISH_AMOUNT, pRank);
    if (pRank == MASTER_RANK) {
        for (int i = 0; i < workParition->paritionCount; i++)
        {
            printf("Process %d is assigned workload of %d fishes with offset "
            "of %d\n", i, workParition->sizes[i], workParition->offsets[i]);
        }
    }

    // Intialise the local fish lake based on the parition size of each process
    localFishLake = fish_lake_new(
        workParition->size, 
        FISH_LAKE_WIDTH, 
        FISH_LAKE_HEIGHT);

    // Worker process does not intialise the fishLake so fishlake->fishes would 
    // cause memory segmentation fault.
    if (pRank == MASTER_RANK) allFishes = fishLake->fishes;
    
    // Scatterv is used to send uneven amount of partition data to different 
    // worker processes
    MPI_Scatterv(
        allFishes,
        workParition->sizes,
        workParition->offsets,
        MPI_SIM_FISH,
        localFishLake->fishes,
        workParition->size,
        MPI_SIM_FISH,
        MASTER_RANK,
        MPI_COMM_WORLD
    );

    // Clear the posistion of the fishes for better validation later on
    if (pRank == MASTER_RANK) {
        for (int i = 0; i < fishLake->fish_amount; i++)
        {
            fishLake->fishes[i].position.x = 0.0f;
            fishLake->fishes[i].position.y = 0.0f;
        }
        printf("Cleared the position of the fishes\n");
        printf("fishLake->fishes[0].position.x = %f\n", 
            fishLake->fishes[0].position.x);
    }

    // Printing just to show that the first fish postion is cleared
    printf("Process %d, first local fish positon x = %f\n", pRank, 
        localFishLake->fishes[0].position.x);

    // Gatherv would allow the master process to gather the data back
    MPI_Gatherv(
        localFishLake->fishes,
        workParition->size,
        MPI_SIM_FISH,
        allFishes,
        workParition->sizes,
        workParition->offsets,
        MPI_SIM_FISH,
        MASTER_RANK,
        MPI_COMM_WORLD
    );

    // Write the recieved fish data to out2.txt
    if (pRank == MASTER_RANK) {
        printf("Recieved data back from woker processes\n");
        write_fish_positions("out2.txt", fishLake);
        printf("Wrote output to out2.txt\n");
    }

    // === Clean ups by freeing up all memores ===

    // Master process free all fishes
    if (pRank == MASTER_RANK) {
        fish_lake_free(fishLake);
    }

    work_parition_free(workParition);
    fish_lake_free(localFishLake);

    //MPI gives warning for not freeing commited types
    mpi_util_free_all_types();    
    MPI_Finalize();
    
    return 0;
}
