/**
 * @file mpi_util.h
 * 
 * Contains custom struct definitions and functions to asssit use of MPI
 * @author Tao Hu
*/

#ifndef H_MPI_UTIL
#define H_MPI_UTIL

#include <mpi.h>
#include <stddef.h>

#include "position.h"
#include "fish.h"

// Custom MPI types
MPI_Datatype MPI_SIM_POSITION;
MPI_Datatype MPI_SIM_FISH;

/**
 * Initializes the MPI datatype for the Position struct.
 */
void mpi_util_init_type_position() {
    int blockLengths[2] = {1,1};
    MPI_Datatype types[2] = {MPI_FLOAT, MPI_FLOAT};
    MPI_Aint offsets[2];

    offsets[0] = offsetof(Position, x);
    offsets[1] = offsetof(Position, y);

    MPI_Type_create_struct(
        2,
        blockLengths,
        offsets,
        types,
        &MPI_SIM_POSITION
    );
    MPI_Type_commit(&MPI_SIM_POSITION);
}

/**
 * Initializes the MPI datatype for the Fish struct.
 */
void mpi_util_init_type_fish() {
    int blockLengths[5] = {1,1,1,1,1};
    MPI_Datatype types[5] = {
        MPI_SIM_POSITION,
        MPI_FLOAT,
        MPI_FLOAT,
        MPI_FLOAT,
        MPI_FLOAT
    };
    MPI_Aint offsets[5];

    offsets[0] = offsetof(Fish, position);
    offsets[1] = offsetof(Fish, distanceFromOrigin);
    offsets[2] = offsetof(Fish, initialWeight);
    offsets[3] = offsetof(Fish, weight);
    offsets[4] = offsetof(Fish, deltaF);

    MPI_Type_create_struct(
        5,
        blockLengths,
        offsets,
        types,
        &MPI_SIM_FISH
    );
    MPI_Type_commit(&MPI_SIM_FISH);
}

/**
 * Initializes all MPI types used in the program.
 */
void mpi_util_init_all_types() {
    mpi_util_init_type_position();
    mpi_util_init_type_fish();
}

#endif