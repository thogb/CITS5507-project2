#!/bin/sh

#SBATCH --account=courses0101
#SBATCH --partition=debug
#SBATCH --ntasks=4
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=16G
#SBATCH --time=00:30:00

module load openmpi/4.0.5

# Retrieved from project 1 and modified.

GCC_LIB_LINK='-lm'
GCC_OPTIONS="${GCC_LIB_LINK}"
C_FILE_NAME="mpi_mp_test"

mpicc ${C_FILE_NAME}.c -o ${C_FILE_NAME} ${GCC_OPTIONS}

srun -N 4 -n 4 -c 1 ${C_FILE_NAME}