#!/bin/sh

#SBATCH --account=courses0101
#SBATCH --partition=debug
#SBATCH --ntasks=4
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=128
#SBATCH --mem-per-cpu=16G
#SBATCH --time=00:10:00

module load openmpi/4.0.5

# Retrieved from project 1 and modified.

FISH_AMOUNT=1000
GCC_LIB_LINK='-lm'
GCC_OPTIONS="${GCC_LIB_LINK}"
C_FILE_NAME="sim_mpi.c"

if [[ ! -z $1 ]]
then
    FISH_AMOUNT=$1
fi

OUT_DIR="exp_data"

if [[ ! -d "$OUT_DIR" ]]
then
    mkdir $OUT_DIR
fi

OUT_FILE="${OUT_DIR}/exp_${FISH_AMOUNT}"

function run_simulation {
    # Define the actual number of thread to be used
    export OMP_NUM_THREADS=$1
    # SLRUM_CPUS_PER_TASK is 128, just largest possible assigned to this task
    srun -c $SLURM_CPUS_PER_TASK $C_FILE_NAME $FISH_AMOUNT >> $OUT_FILE
}

function run_experiment {
    GCC_OPTIONS="${GCC_LIB_LINK} -D $1"

    mpicc $GCC_OPTIONS -fopenmp "${C_FILE_NAME}.c" -o $C_FILE_NAME

    echo "Running experiment: FISH_AMOUNT=${FISH_AMOUNT}, schedule=$1"

    run_simulation 8
    run_simulation 32
    run_simulation 128
}

touch $OUT_FILE

# Run the experiment with static schedule
run_experiment S_STATIC

# Run the experiment with dynamic schedule
if [[ -z $2 ]]
then
    run_experiment S_DYNAMIC
fi

# Run the experiment with guided schedule
run_experiment S_GUIDED

echo "Experiment finished"

./raw_to_csv.sh $OUT_FILE
rm $OUT_FILE

echo "Saved experiment data into ${OUT_FILE}.csv"
