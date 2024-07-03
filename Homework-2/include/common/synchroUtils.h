#pragma once


// > Initializes the index which is the index of the cell of the concurrent processes array of
//   the shared memory that the caller process shall occupy.
// > Initializes proc_info for the caller process.
// > Checks for any problematic synchro-scenarios (particular range-conflicts) with the wanted
//   range and the ranges that other processes are already working with. If there is none such
//   conflict, the process may continue with its execution. Else, the process puts itself into
//   sleep and waits to be woken up.
// > Updates the stats of the system and wakes up the accountant to log them via a handshake
// > Initializes the waiting time of caller process (worker) once it get into its critical section.
// Arguments:
//    > [shm_ptr]:      pointer to shared memory segment struct
//    > [*index]:       the cell index that's going to be initialized
//    > [proc_type]:    the process type (reader/writer)
//    > [wanted_range]: the range that the caller wants to work with
//    > [diff]:         if the caller is a writer, diff is the value of
//                      -v flag (for logging purposes)
//    > [start_tm]:     the time instance of the initial admission of the caller
//                      into the system
void check_range_and_lock(shmem_segment* shm_ptr, int* index, int proc_type, range wanted_range, int diff, timer start_tm);


// > Checks for any problematic synchro-scenarios (particular range-conflicts) with the range of the
//   caller that they just worked with and the wanted ranges of the waiting/blocked processes.
//   If there is any, the caller will try to deblock them. If those processes were waiting for the
//   caller only, then the caller will also wake them up from sleep.
// > Caller leaves its cell from concurrent processes array.
// > Updates the stats of the system and wakes up the accountant to log them via a handshake
// Arguments:
//    > [shm_ptr]:      pointer to shared memory segment struct
//    > [index]:        the cell index that's going to be left unoccupied
//    > [capacity]:     the number of records that were processed
//    > [prev_balance]: if caller is a writer, prev_balance is the balance of the record
//                      before it was updated by the writer (for logging purposes)
//    > [new_balance]:  if caller is a writer, new_balance is the balance of the record
//                      after it was updated by the writer (for logging purposes)
//    > [total_time]:   the elapsed time from the time instance of the initial admission of the
//                      caller into the system until the time instance of calling this function
//                      (for logging purposes)
void check_range_and_unlock(shmem_segment* shm_ptr, int index, int capacity, int prev_balance, int new_balance, double total_time);