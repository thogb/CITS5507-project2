/**
 * @file first_deliverable.c
 * 
 * Perform a simple message passing of a list of the fish structures. Two output
 *  files are created one after fish intialisation and one after receiving data 
 * from worker processes.
 * 
 * @author Tao Hu
 */

#define TARGET_OS_IPHONE 0

#include <stdio.h>
#include <mpi.h>

#include "../lib/fish_lake.h"
#include "../lib/sim_util.h"
#include "../lib/work_parition.h"

// Define odd number of fishes to test the edge case
#define FISH_AMOUNT 3153
#define MASTER_RANK 0
#define FISH_LAKE_WIDTH 200.0f
#define FISH_LAKE_HEIGHT 200.0f

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
    FishLake* fishLake;
    FishLake* localFishLake;
    int pRank;
    int wSize;
    WorkPartition* workParition;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pRank);
    MPI_Comm_size(MPI_COMM_WORLD, &wSize);

    if (wSize < 2) {
        printf("Program requires at least 2 processes\n");
        return 1;
    }
    
    // The master process intialises all the fishes.
    if (pRank == MASTER_RANK) {
        fishLake = fish_lake_new(
        FISH_AMOUNT, 
        FISH_LAKE_WIDTH, 
        FISH_LAKE_HEIGHT);
        printf("Program running with %d processes\n", wSize);
        fish_lake_init_fishes(fishLake);
        printf("Initialised the fish lake\n");
        // Write the fish position to out1.txt
        write_fish_positions("out1.txt", fishLake);
        printf("Wrote the fish positions to out1.txt\n");
        // Clear the posistion of the fishes for corrrect validation later on
        for (int i = 0; i < fishLake->fish_amount; i++)
        {
            fishLake->fishes[i].position.x = 0.0f;
            fishLake->fishes[i].position.y = 0.0f;
        }
        printf("Cleared the position of the fishes\n");
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

    // Master process free all fishes
    if (pRank == MASTER_RANK) {
        fish_lake_free(fishLake);
    }

    work_parition_free(workParition);
    fish_lake_free(localFishLake);

    MPI_Finalize();
    return 0;
}
