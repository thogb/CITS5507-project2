#!/bin/sh

#SBATCH --account=courses0101

#SBATCH --partition=debug

#SBATCH --ntasks=1

#SBATCH --ntasks-per-node=1

#SBATCH --cpus-per-task=128

#SBATCH --time=00:10:00

module load gcc

FISH_AMOUNT=1000
GCC_LIB_LINK='-lm'
GCC_OPTIONS="${GCC_LIB_LINK}"

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
    export OMP_NUM_THREADS=$1
    srun -c $SLURM_CPUS_PER_TASK simulation_parallel $FISH_AMOUNT >> $OUT_FILE
}

function run_experiment {
    GCC_OPTIONS="${GCC_LIB_LINK} -D $1"

    gcc $GCC_OPTIONS -fopenmp simulation_parallel.c -o simulation_parallel

    echo "Running experiment: FISH_AMOUNT=${FISH_AMOUNT}, schedule=$1"

    run_simulation 1
    run_simulation 2
    run_simulation 4
    run_simulation 8
    run_simulation 16
    run_simulation 32
    run_simulation 64
    run_simulation 128
}

touch $OUT_FILE

# Run the experiment
# Compile sequential
gcc $GCC_OPTIONS simulation_sequential.c -o simulation_sequential

# Run sequential
srun -c 1 simulation_sequential $FISH_AMOUNT >> $OUT_FILE

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
