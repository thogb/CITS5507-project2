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

int main(int argc, char const *argv[])
{
    FishLake* fishLake;
    int pRank;
    int wSize;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pRank);
    MPI_Comm_size(MPI_COMM_WORLD, &wSize);
    
    // The master process intialises all the fishes.
    if (pRank == MASTER_RANK) {
        fishLake = fish_lake_new(
        FISH_AMOUNT, 
        FISH_LAKE_WIDTH, 
        FISH_LAKE_HEIGHT);
        fish_lake_init_fishes(fishLake);
        // Write the fish position to out1.txt
        write_fish_positions("out1.txt", fishLake);
        printf("Program running on %d processes\n", wSize);
    }

    // Master process free all fishes
    if (pRank == MASTER_RANK) {
        fish_lake_free(fishLake);
    }

    MPI_Finalize();
    return 0;
}
