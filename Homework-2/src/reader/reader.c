#include "../../include/common/global.h"
#include "../../include/reader/readerUtils.h"
#include "../../include/common/processUtils.h"
#include "../../include/common/shmem.h"
#include "../../include/common/inputUtils.h"
#include "../../include/common/synchroUtils.h"

#define MAX_TOTAL_ARGS 11


int main(int argc, char* argv[]) {

    // Since you got into the system, start the timer
    timer start_tm;
    start_timer(&start_tm);

    char* file_name;
    int   rec_index_start;
    int   rec_index_finish;
    int   max_time;
	char* shm_path;
    
    // Initialize command line arguments
    reader_args_init(argc, argv, MAX_TOTAL_ARGS, &file_name, &rec_index_start, &rec_index_finish, &max_time, &shm_path);

    // Open the shared memory segment and get a pointer to it
    int shm_fd;
	shmem_segment* shm_ptr;
	shmem_open(&shm_fd, shm_path, &shm_ptr);

    // Prepare the file you are going to read
    int fd;
    int offset = rec_index_start * sizeof(record);
    prepare_file(&fd, file_name, offset, O_RDONLY);

    // Initialize capacity, meaning the number of records you'll read
    int capacity;
	reader_capacity_init(&capacity, rec_index_finish, rec_index_start);

    // Initialize the array that shall hold pointers to the records you'll read
    size_t  n_bytes = sizeof(record) * capacity;
    record* records = malloc(n_bytes);

    sem_wait(&(shm_ptr->conc_proc));            // Potentially wait here if the system is full

    // Try to read the range you want
    int index;
    range wanted_range = {rec_index_start, rec_index_finish};
    check_range_and_lock(shm_ptr, &index, READER, wanted_range, -1, start_tm);

    // Reading is done here
    Read(fd, records, n_bytes);

    // Print records and initialize the sum of balances
    int sum_balance = 0;
    reader_print_records(&sum_balance, capacity, records);

    reader_print_avg_balance(sum_balance, capacity);

    sleep(gen_num(max_time, 0));                // "Work" for some time

    // Let it be known that you have finished working
    check_range_and_unlock(shm_ptr, index, capacity, -1, -1, elapsed_time(start_tm));

    sem_post(&(shm_ptr->conc_proc));            // Since you are exiting the system

    shut_file(fd);

	// Closing shmem
	shmem_close(shm_fd, shm_ptr);

	// Cleanup
	free(records);
    free_args(file_name, shm_path);

    return EXIT_SUCCESS;
}
