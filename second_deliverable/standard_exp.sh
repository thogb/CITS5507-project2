#!/bin/sh

#SBATCH --account=courses0101
#SBATCH --partition=debug
#SBATCH --ntasks=4
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=128
#SBATCH --exclusive
#SBATCH --time=00:30:00

# Seems mpicc is there by default
#module load openmpi/4.0.5

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
EXP_END_KEYWORD="Completed"

# Experiment combinations
EXP_INSTRUCTION_FILE="exp_instructions.txt"

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
    # 2 process 2 thread on large computation takes a lot of time, hence not ran
    if [[ $1 -lt 100000000 ]]
    then
        run_simulation $1 $2 $3 2
    fi

    run_simulation $1 $2 $3 8
    run_simulation $1 $2 $3 32
    run_simulation $1 $2 $3 128
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
    if [[ -f "$OUT_FILE" ]]
    then
        # The experiment is completed do not run
        if [[ $(tail -1 $OUT_FILE) == $EXP_END_KEYWORD ]]
        then
            return 1
        fi
        # Experiemnt is incomplete interuptted, remove and restart
        rm $OUT_FILE
    fi

    SCHEDULE_METHOD=$SCHEDULE_METHOD_STATIC
    compile_program $SCHEDULE_METHOD
    run_experiment_process $1 $2

    SCHEDULE_METHOD=$SCHEDULE_METHOD_GUIDED
    compile_program $SCHEDULE_METHOD
    run_experiment_process $1 $2

    # Dynamic becomes very slow for some unknown reason, stop when too much 
    # computation
    if [[ $1 -lt 5000000 ]]
    then
        SCHEDULE_METHOD=$SCHEDULE_METHOD_DYNAMIC    
        compile_program $SCHEDULE_METHOD
        run_experiment_process $1 $2
    fi

    echo $EXP_END_KEYWORD >> $OUT_FILE
}

# Run all experiements by combination of fish amount and simulation steps
function run_all_experiment {
    JOB_DONE=1
    N_OF_JOBS=$(cat $EXP_INSTRUCTION_FILE | wc -l)
    for line in $(cat $EXP_INSTRUCTION_FILE)
    do
        echo "performing experiment set: ${JOB_DONE}/${N_OF_JOBS}"
        fishAmount=$(echo $line | cut -d ',' -f 1)
        steps=$(echo $line | cut -d ',' -f 2)
        run_experiment_thread_schedule $fishAmount $steps
        JOB_DONE=$((JOB_DONE + 1))
    done
}

echo "Starting all experiments"
run_all_experiment
echo "Finished all experiments"

chmod u+x raw_to_csv.sh

for fileName in $(ls $OUT_DIR)
do
    ./raw_to_csv.sh "${OUT_DIR}/${fileName}" "${OUT_DIR_CSV}/${fileName}.csv"   
done
