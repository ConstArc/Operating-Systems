#include "../../include/common/global.h"
#include "../../include/common/shmem.h"
#include "../../include/common/synchroUtils.h"
#include "../../include/common/processUtils.h"

static void init_index(shmem_segment* shm_ptr, int* index);
static proc_info* init_proc_info(shmem_segment* shm_ptr, int* index, int proc_type, range wanted_range);
static bool ranges_conflict(range* R1, range* R2);

static void stats_entry(shmem_segment* shm_ptr, proc_info* caller, int diff);
static void stats_exit(shmem_segment* shm_ptr, proc_info* caller, int capacity, int prev_balance, int new_balance, double total_time);

static void check_range_entry(shmem_segment* shm_ptr, proc_info* caller);
static void check_range_exit(shmem_segment* shm_ptr, proc_info* caller);


void check_range_and_lock(shmem_segment* shm_ptr, int* index, int proc_type, range wanted_range, int diff, timer start_tm) {
    assert(shm_ptr != NULL);
    assert(proc_type == WRITER || proc_type == READER);
    assert(wanted_range.start > -1 && wanted_range.finish > -1);

    sem_wait(&(shm_ptr->mutex));
        // Initialize process info of caller process and get a pointer to it
        proc_info* caller = init_proc_info(shm_ptr, index, proc_type, wanted_range);
        
        // Check the shared memory to see if you can continue to process the wanted range or not
        check_range_entry(shm_ptr, caller);

        // Update the stats of the system and wakeup the accountant to log them
        stats_entry(shm_ptr, caller, diff);
    sem_post(&(shm_ptr->mutex));

    // Check if there was at least one problematic synchro case with your wanted range
    if(caller->blocked_by > 0)
        // If so, put yourself into sleep (wait)
        sem_wait(&(shm_ptr->conc_proc_arr[caller->index].waiting_sem));

    // By this point, we have entered the CS and we are processing
    // the range of records within the [wanted_range]

    // Save the waiting time in our struct
    caller->waiting_time = elapsed_time(start_tm);
}

void check_range_and_unlock(shmem_segment* shm_ptr, int index, int capacity, int prev_balance, int new_balance, double total_time) {
    assert(shm_ptr != NULL);

    proc_info* caller = &(shm_ptr->conc_proc_arr[index]);

    sem_wait(&(shm_ptr->mutex));
        // Check if there were any processes blocked, waiting for you to finish your job and wake them up
        check_range_exit(shm_ptr, caller);

        // Mark your cell as unoccupied, since we are about to leave (notice we haven't given the mutex yet)
        shm_ptr->conc_proc_arr[index].index = -1;

        // Update the stats of the system and wakeup the accountant to log them
        stats_exit(shm_ptr, caller, capacity, prev_balance, new_balance, total_time);
    sem_post(&(shm_ptr->mutex));
}

// Implements the inspection of the concurrent processes array of shared memory segment, to see
// if the caller's wanted range falls into a problematic synchro scenario, in which case, it 
// increases the caller's blocked_by-counter by 1.
// Arguments:
//      > [shm_ptr]: pointer to shared memory segment struct
//      > [caller]:  pointer to proc_info struct of the caller process
void check_range_entry(shmem_segment* shm_ptr, proc_info* caller) {
    for(int i = 0; i < MAX_CONCURRENT_PROCESSES; i++) {

        // If the cell is not occupied (index = -1) or it is our cell
        if(shm_ptr->conc_proc_arr[i].index == -1 || i == caller->index)
            continue;

        // If there isn't any conflicts of ranges
        if(!ranges_conflict( &(shm_ptr->conc_proc_arr[i].wanted_range), &(caller->wanted_range)))
            continue;
        
        // If our priority is greater (that means we are "older" for the system)
        if(shm_ptr->conc_proc_arr[i].priority < caller->priority)
            continue;

        // By this point, if we are a writer, we fall into a problematic synchro case,
        // therefore we increase our blocked_by counter by 1
        if(caller->proc_type == WRITER)
            caller->blocked_by++;

        // By this point, if we are a reader and the other conflicting process is a writer,
        // we fall into a problematic synchro case, thus we add 1 to the blocked_by counter
        if(caller->proc_type == READER && shm_ptr->conc_proc_arr[i].proc_type == WRITER)
            caller->blocked_by++;
    }
}

// Implements the inspection of the concurrent processes array of shared memory segment, to see
// if the caller's wanted range has any conflicts with the waiting/blocked worker processes and
// if it finds a problematic synchro scenario with a blocked process, it decreases the blocked_by
// -counter of the blocked process by 1. If that counter reaches 0, the corresponding process is
// woken up.
// Arguments:
//      > [shm_ptr]: pointer to shared memory segment struct
//      > [caller]:  pointer to proc_info struct of the caller process
void check_range_exit(shmem_segment* shm_ptr, proc_info* caller) {
    for(int i = 0; i < MAX_CONCURRENT_PROCESSES; i++) {

        // If the cell is not occupied (index = -1) or it is our cell
        if(shm_ptr->conc_proc_arr[i].index == -1 || i == caller->index)
            continue;

        // If there isn't any conflicts of ranges
        if(!ranges_conflict(&(shm_ptr->conc_proc_arr[i].wanted_range), &(caller->wanted_range)))
            continue;

        // If our priority is smaller (that means we are "younger" for the system)
        if(shm_ptr->conc_proc_arr[i].priority > caller->priority)
            continue;

        // If we are a WRITER that means there was a problematic synchro case
        if(caller->proc_type == WRITER) {
            assert(shm_ptr->conc_proc_arr[i].blocked_by > 0);

            // Decrease the blocked_by counter of the waiting process by one, since we are leaving
            shm_ptr->conc_proc_arr[i].blocked_by--;

            assert(shm_ptr->conc_proc_arr[i].blocked_by >= 0);          // Just for sanity check
        }

        // If we are a reader and the blocked process is a writer (problematic synchro
        // case) we deblock the writer by one, since we are leaving now
        else if(caller->proc_type == READER && shm_ptr->conc_proc_arr[i].proc_type == WRITER) {
            assert(shm_ptr->conc_proc_arr[i].blocked_by > 0);

            // Decrease the blocked_by counter of the waiting process by one
            shm_ptr->conc_proc_arr[i].blocked_by--;

            assert(shm_ptr->conc_proc_arr[i].blocked_by >= 0);          // Just for sanity check

        }
        else continue;

        // If decreased blocked_by-counter reaches 0, that means this process was only
        // waiting for us, thus we proceed to wake it up
        if(shm_ptr->conc_proc_arr[i].blocked_by == 0)
            sem_post(&(shm_ptr->conc_proc_arr[i].waiting_sem));
    }
}

// > Initializes the message field of the registry in the shared memory used by
// the accountant. Within the message, several useful information are added
// relevant to the caller's progress.
// > Wakes up the accountant to log the stats of the system and waits for them
// to signal back that they have done the job, in a "handshake" manner.
// Arguments:
//      > [shm_ptr]: pointer to shared memory segment struct
//      > [caller]:  pointer to proc_info struct of the caller process
//      > [diff]:    if caller is a writer, then the diff is the value
//      passed to them from -v flag. Else, it is -1. Used for logging purposes.
void stats_entry(shmem_segment* shm_ptr, proc_info* caller, int diff) { 
    if(caller->proc_type == READER) {
        if(caller->wanted_range.start != caller->wanted_range.finish)
            sprintf(shm_ptr->proc_registry.msg, " READER with pid: [%d] just ENTERED the system and wants to read records\n"
                        " from record id: %d to record id: %d.\n", caller->pid, caller->wanted_range.start+1, caller->wanted_range.finish+1);
        else
            sprintf(shm_ptr->proc_registry.msg, " READER with pid [%d] just ENTERED the system and wants to read record\n"
                        " with record id: %d.\n", caller->pid, caller->wanted_range.start+1);
    }

    if(caller->proc_type == WRITER)
        sprintf(shm_ptr->proc_registry.msg, " WRITER with pid: [%d] just ENTERED the system and wants to write on record\n"
                    " with record id: %d and update its balance by: %d.\n", caller->pid, caller->wanted_range.start+1, diff);

    // Make a handshake with the accountant to log the stats
    sem_post(&(shm_ptr->accountant));           // Wake up the accountant
    sem_wait(&(shm_ptr->accountant_job_done));  // Wait here until he tells you he's done the logging
}

// > Initializes the message field of the registry in the shared memory used by
// the accountant. Within the message, several useful information are added
// relevant to the caller progress.
// > Wakes up the accountant to log the stats of the system and waits for them
// to signal back that they have done the job, in a "handshake" manner.
// Arguments:
//     > [shm_ptr]:      pointer to shared memory segment struct
//     > [caller]:       pointer to proc_info struct of the caller process
//     > [capacity]:     the number of records that were processed
//     > [prev_balance]: if caller is a writer, prev_balance is the balance of the record
//                       before it was updated by the writer
//     > [new_balance]:  if caller is a writer, new_balance is the balance of the record
//                       after it was updated by the writer
//     > [total_time]:   the elapsed time from the time instance of the initial admission of the
//                       caller into the system until the time instance of calling this function
void stats_exit(shmem_segment* shm_ptr, proc_info* caller, int capacity, int prev_balance, int new_balance, double total_time) {
    if(caller->proc_type == READER)
        sprintf(shm_ptr->proc_registry.msg, " READER with pid: [%d] just EXITED their critical section and the number of records\n"
                    " they read is: %d.\n", caller->pid, capacity);

    if(caller->proc_type == WRITER)
        sprintf(shm_ptr->proc_registry.msg, " WRITER with pid: [%d] just EXITED their critical section and now the record with\n"
                    " id: %d has a balance of: %d from: %d that it previously had.\n",
                    caller->pid, caller->wanted_range.start+1, new_balance, prev_balance);   

    // Increase number of processes that have finished with their job
    shm_ptr->proc_registry.n_processes[caller->proc_type]++;

    // Increase total number of records processed by [capacity] number of records
    shm_ptr->proc_registry.n_records_processed += capacity;

    // Increase number of records processed by particular process type by [capacity] number of records
    shm_ptr->proc_registry.n_records_by_type[caller->proc_type] += capacity;
    
    // If your waiting time is greater than the already maximum waiting time, update it with yours
    if(shm_ptr->proc_registry.max_waiting_time < caller->waiting_time)
        shm_ptr->proc_registry.max_waiting_time = caller->waiting_time;

    // Add your total time to sum of active processes time
    shm_ptr->proc_registry.sum_active_proc_time[caller->proc_type] += total_time;

    // Make a handshake with the accountant to log the stats
    sem_post(&(shm_ptr->accountant));           // Wake up the accountant
    sem_wait(&(shm_ptr->accountant_job_done));  // Wait here until he tells you he's done the logging
}

// Traverses the concurrent processes array of the shared memory segmnent
// in order to find an empty spot for the caller to occupy. Once such spot
// is found, it marks the spot as occupied, initializes the index and returns.
// Arguments:
//      > [shm_ptr]: pointer to shared memory segment struct
//      > [*index]:  pointer to the uninitialized index that's going to be initialized
void init_index(shmem_segment* shm_ptr, int* index) {
    *index = -1;
    for(int i = 0; i < MAX_CONCURRENT_PROCESSES; i++) {

        // Empty spot found
        if(shm_ptr->conc_proc_arr[i].index == -1) {
            shm_ptr->conc_proc_arr[i].index = i;
            *index = i;
            return;
        }
    }

    // If the index wasn't initialized, that means the array was full.
    // This scenario should never occur with the handling we are doing,
    // however we place an assert for a simple sanity check.
    assert(*index != -1);
}

// > Initializes the index aka the spot for the caller process in the concurrent
//   processes array of shared memory. 
// > Initializes several fields of the proc_info struct of the caller process 
//   overwriting the old ones which had possibly remained from the last occupant.
// > Returns a pointer to the now-occupied cell of concurrent proccess array which
//   is the cell at index = [index].
// Arguments:
//      > [shm_ptr]:      pointer to shared memory segment struct
//      > [*index]:       pointer to the uninitialized index that's going to be initialized
//      > [proc_type]:    process type of caller (reader or writer)
//      > [wanted_range]: wanted range that the caller wants to work with
proc_info* init_proc_info(shmem_segment* shm_ptr, int* index, int proc_type, range wanted_range) {

    init_index(shm_ptr, index);

    shm_ptr->conc_proc_arr[*index].pid        = getpid();
    shm_ptr->conc_proc_arr[*index].priority   = shm_ptr->ticket_number--;
    shm_ptr->conc_proc_arr[*index].proc_type  = proc_type;
    shm_ptr->conc_proc_arr[*index].blocked_by = 0;
    shm_ptr->conc_proc_arr[*index].wanted_range.start  = wanted_range.start;
    shm_ptr->conc_proc_arr[*index].wanted_range.finish = wanted_range.finish;

    return &(shm_ptr->conc_proc_arr[*index]);
}

// A simple function that returns true if there is an overlapping
// between the ranges R1 and R2. If there is no such conflict, it
// returns false.
bool ranges_conflict(range* R1, range* R2) {
    assert(R1 != NULL && R2 != NULL);
    assert(R1->start <= R1->finish);
    assert(R2->start <= R2->finish);

    if(R1->start <= R2->finish && R1->finish >= R2->start)
        return true;

    return false;
}
