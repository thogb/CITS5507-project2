#!/bin/sh

#SBATCH --account=courses0101
#SBATCH --partition=debug
#SBATCH --ntasks=4
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=128
#SBATCH --mem-per-cpu=16G
#SBATCH --time=00:30:00

module load openmpi/4.0.5

# Retrieved from project 1 and modified.

GCC_LIB_LINK='-lm'
GCC_OPTIONS="${GCC_LIB_LINK}"
C_FILE_NAME="sim_mpi.c"

SCHEDULE_METHOD_STATIC="S_STATIC"
SCHEDULE_METHOD_GUIDED="S_GUIDED"
SCHEDULE_METHOD_DYNAMIC="S_DYNAMIC"
SCHEDULE_METHOD=$SCHEDULE_EMTHOD_STATIC


OUT_DIR="exp_data"
OUT_DIR_CSV="${OUT_DIR}_csv"

if [[ ! -d "$OUT_DIR" ]]
then
    mkdir $OUT_DIR
fi

if [[ ! -d "$OUT_DIR_CSV" ]]
then
    mkdir $OUT_DIR_CSV
fi

# out file name made up of experiment parameters
# 1000 fish and 10 simulation steps
OUT_FILE="${OUT_DIR}/exp_1000_10"

# Compile the program again based on the parameter
# Params:
#       $1: the thread schedule method
function compile_program {
    GCC_OPTIONS="${GCC_LIB_LINK} -D $1"
    mpicc "${C_FILE_NAME}.c" -o $C_FILE_NAME $GCC_OPTIONS -fopenmp
}

# Run a single simulation and save the data into output file.
# Params:
#       $1: number of processes
#       $2: number of threads per process
#       $3: number of fish in the simulation
#       $4: number of simulation steps
function run_simulation {
    OUT_FILE="${OUT_DIR}/exp_$3_$4"

    # Already have the data, not running again
    if [[ -f OUT_FILE ]]
    then
        return 1
    fi

    # Dynamic becomes very slow for some unknown reason, stop when too much 
    # computation
    if [[ $SCHEDULE_METHOD == $SCHEDULE_METHOD_DYNAMIC && $(($3 * $3)) -ge 100000000 ]]
    then
        return 1
    fi

    # Define the actual number of thread to be used
    export OMP_NUM_THREADS=$2
    # SLRUM_CPUS_PER_TASK is 128, just largest possible assigned to this task
    srun -N $1 -n $1 -c $SLURM_CPUS_PER_TASK $C_FILE_NAME $3 $4 >> $OUT_FILE
}

# Run experiment with a set of simulation inputs
# Params:
#       $1: number of process
#       $2: number of threads per process
function run_experiment_params_set {
    # Fish amounts
    run_simulation $1 $2 100000 100
    run_simulation $1 $2 500000 100
    run_simulation $1 $2 1000000 100
    run_simulation $1 $2 5000000 100
    run_simulation $1 $2 10000000 100

    # Lower time step and fish amounts
    run_simulation $1 $2 10000000 10
    run_simulation $1 $2 30000000 10
    run_simulation $1 $2 50000000 10
    run_simulation $1 $2 100000000 10
    run_simulation $1 $2 300000000 10
    run_simulation $1 $2 500000000 10

    # Simulation steps
    run_simulation $1 $2 10000000 10
    run_simulation $1 $2 10000000 20
    run_simulation $1 $2 10000000 30
    run_simulation $1 $2 10000000 20
    run_simulation $1 $2 10000000 20
    run_simulation $1 $2 10000000 20
    run_simulation $1 $2 10000000 70
    run_simulation $1 $2 10000000 20
    run_simulation $1 $2 10000000 90
}

# Run the experiment
# Params:
#       $1: number of process
function run_experiment_thread_amount {
    run_experiment_params_set $1 2
    run_experiment_params_set $1 8
    run_experiment_params_set $1 32
    run_experiment_params_set $1 128
}

# Run the experiment with a different set of process num
function run_experiment_process {
    run_experiment_thread_amount 2
    run_experiment_thread_amount 3
    run_experiment_thread_amount 4
}

# Run the experiment with different thread schedule method
function run_experiment_thread_schedule {
    SCHEDULE_METHOD=$SCHEDULE_METHOD_STATIC
    compile_program $SCHEDULE_METHOD
    run_experiment_process
    
    SCHEDULE_METHOD=$SCHEDULE_METHOD_DYNAMIC
    compile_program $SCHEDULE_METHOD
    run_experiment_process

    SCHEDULE_METHOD=$SCHEDULE_METHOD_GUIDED
    compile_program $SCHEDULE_METHOD
    run_experiment_process
}

echo "Starting all experiments"
run_experiment_thread_schedule
echo "Finished all experiments"

for fileName in $(ls $OUT_DIR)
do
    raw_to_csv "${OUT_DIR}/${fileName}" "${OUT_DIR_CSV}/${fileName}.csv"   
done
