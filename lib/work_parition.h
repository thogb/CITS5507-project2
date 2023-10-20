/**
 * @file work_parition.h
 * 
 * Contains struct definition to store the data on workload paritioned between 
 * all processes.
 * 
 * @author Tao Hu
*/

#ifndef H_WORK_PARITION
#define H_WORK_PARITION

#include <stdlib.h>

/**
 * @brief Used to help the parition of workload to all the processes.
*/
typedef struct workParition
{
    // Represents the displs used in scatterv, offsets for list of processes
    int* offsets;
    // Represents the sendcoutns used in scatterv, a list of parition size for
    // processes
    int* sizes;
    // The number of paritions or processes
    int paritionCount;
    // The sum of all size, representing the work to be partitioned
    int totalSize;
    // The max possible allocated size to a process. Used to intialise work
    int maxSize;
    // The rank or id of the process using this work parition
    int rank;
    // The offset for this procecss
    int offset;
    // The size of work for this process
    int size;
} WorkPartition;

/**
 * Creates a new WorkPartition object.
 *
 * @param partitionCount The number of partitions to create.
 * @param totalSize The total size of the work.
 * @param rank The rank of the subject process.
 *
 * @return A pointer to the newly created WorkPartition object.
 */
WorkPartition* work_parition_new(
    int partitionCount,
    int totalSize,
    int rank) {
    int reminder = totalSize % partitionCount;
    int size = totalSize / partitionCount;
    int currOffset = 0;

    WorkPartition* workParition = (WorkPartition*)malloc(sizeof(WorkPartition));
    workParition->offsets = (int*)malloc(sizeof(int) * partitionCount);
    workParition->sizes = (int*)malloc(sizeof(int) * partitionCount);
    workParition->paritionCount = partitionCount;
    workParition->totalSize = totalSize;

    // Fill up the offsets and sizes. The reminder will be evenly consumed based
    //  on the order of the parition workers.
    for (int i = 0; i <  partitionCount; i++)
    {
        workParition->offsets[i] = currOffset;
        workParition->sizes[i] = size;

        if (reminder > 0) {
            reminder--;
            workParition->sizes[i]++;
        }

        currOffset += workParition->sizes[i];
    }

    // Fill the information that is specific to the subject worker/process
    workParition->maxSize = size + 1;
    workParition->rank = rank;
    workParition->offset = workParition->offsets[rank];
    workParition->size = workParition->sizes[rank];
    return workParition;
}

/**
 * Free the memory allocated for the work partition.
 *
 */
void work_parition_free(WorkPartition* workParition) {
    free(workParition->offsets);
    free(workParition->sizes);
    free(workParition);
}

#endif
