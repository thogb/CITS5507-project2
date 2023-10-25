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
C_FILE_NAME="sim_mpi"

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
#       $1: the fish amount
#       $2: the simulation steps
#       $3: the number of processes
#       $4: the number of threads per process
function run_simulation {
    OUT_FILE="${OUT_DIR}/exp_$1_$2.txt"

    # Define the actual number of thread to be used
    export OMP_NUM_THREADS=$4
    # SLRUM_CPUS_PER_TASK is 128, just largest possible assigned to this task
    # Data from the program is appended to the correct output file
    srun -N $3 -n $3 -c $SLURM_CPUS_PER_TASK $C_FILE_NAME $1 $2 >> $OUT_FILE
}

# Run the experiment with differnt thread amount
# Params:
#       $1: the fish amount
#       $2: the simulation steps
#       $3: number of process
function run_experiment_thread_amount {
    run_experiment_params_set $1 $2 $3 2
    run_experiment_params_set $1 $2 $3 8
    run_experiment_params_set $1 $2 $3 32
    run_experiment_params_set $1 $2 $3 128
}

# Run the experiment with a different set of process num
# Params:
#       $1: the fish amount
#       $2: the simulation steps
function run_experiment_process {
    run_experiment_thread_amount $1 $2 2
    run_experiment_thread_amount $1 $2 3
    run_experiment_thread_amount $1 $2 4
}

# Run the experiment with different thread schedule method
# Params:
#       $1: the fish amount
#       $2: the simulation steps
function run_experiment_thread_schedule {
    OUT_FILE="${OUT_DIR}/exp_$1_$2.txt"

    # The file exists, means the experiement has run before. This check is 
    # useful when one submitted job runs out of time and require sto resubmit.
    if [[ -f OUT_FILE ]]
    then
        return 1
    fi

    SCHEDULE_METHOD=$SCHEDULE_METHOD_STATIC
    compile_program $SCHEDULE_METHOD
    run_experiment_process $1 $2

    SCHEDULE_METHOD=$SCHEDULE_METHOD_GUIDED
    compile_program $SCHEDULE_METHOD
    run_experiment_process $1 $2

    # Dynamic becomes very slow for some unknown reason, stop when too much 
    # computation
    if [[ $SCHEDULE_METHOD == $SCHEDULE_METHOD_DYNAMIC && $(($3 * $3)) -ge 100000000 ]]
    then
        return 1
    fi
    
    SCHEDULE_METHOD=$SCHEDULE_METHOD_DYNAMIC
    compile_program $SCHEDULE_METHOD
    run_experiment_process $1 $2
}

# Run all experiements by combination of fish amount and simulation steps
function run_all_experiment {
    # Fish amounts
    run_experiment_thread_schedule 100000 100
    run_experiment_thread_schedule 500000 100
    run_experiment_thread_schedule 1000000 100
    run_experiment_thread_schedule 5000000 100
    run_experiment_thread_schedule 10000000 100

    # Lower time step and fish amounts
    run_experiment_thread_schedule 10000000 10
    run_experiment_thread_schedule 30000000 10
    run_experiment_thread_schedule 50000000 10
    run_experiment_thread_schedule 100000000 10
    run_experiment_thread_schedule 300000000 10
    run_experiment_thread_schedule 500000000 10

    # Simulation steps
    run_experiment_thread_schedule 10000000 10
    run_experiment_thread_schedule 10000000 20
    run_experiment_thread_schedule 10000000 30
    run_experiment_thread_schedule 10000000 20
    run_experiment_thread_schedule 10000000 20
    run_experiment_thread_schedule 10000000 20
    run_experiment_thread_schedule 10000000 70
    run_experiment_thread_schedule 10000000 20
    run_experiment_thread_schedule 10000000 90
}

echo "Starting all experiments"
run_experiment_thread_schedule
echo "Finished all experiments"

for fileName in $(ls $OUT_DIR)
do
    ./raw_to_csv.sh "${OUT_DIR}/${fileName}" "${OUT_DIR_CSV}/${fileName}.csv"   
done
