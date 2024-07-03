#include "../../include/common/global.h"
#include "../../include/common/processUtils.h"
#include "../../include/common/shmem.h"
#include "../../include/common/inputUtils.h"
#include "../../include/common/synchroUtils.h"
#include "../../include/writer/writerUtils.h"
#include "../../include/common/record.h"

#define TOTAL_ARGS 11


int main(int argc, char* argv[]) {

    // Since you got into the system, start the timer
    timer start_tm;
    start_timer(&start_tm);

    char* file_name;
    int   rec_index;
    int   value;
    int   max_time;
	char* shm_path;

    // Initialize command line arguments
    writer_args_init(argc, argv, TOTAL_ARGS, &file_name, &rec_index, &value, &max_time, &shm_path);    

    // Open the shared memory segment and get a pointer to it
    int shm_fd;
	shmem_segment* shm_ptr;
	shmem_open(&shm_fd, shm_path, &shm_ptr);

    // Prepare the file you are going to write
    int fd;
    int offset = rec_index * sizeof(record);
    prepare_file(&fd, file_name, offset, O_RDWR);

    sem_wait(&(shm_ptr->conc_proc));                        // Potentially wait here if the system is full

    // Try to write the record you want to
    int index;
    range wanted_range = {rec_index, rec_index};
    check_range_and_lock(shm_ptr, &index, WRITER, wanted_range, value, start_tm);

    record Rec;
    Read(fd, &Rec, sizeof(record));                             // Read the record you want to update
    int prev_balance = Rec.balance;                             // Take its old balance for logging

    record_balance_update(&Rec, value);                         // Update its balance here

    if (lseek(fd, -sizeof(record), SEEK_CUR) == -1)             // Move fd's file position back to the start
        errExit("lseek");                                       // of the record
    
    if (write(fd, &Rec, sizeof(record)) == -1)                  // Writing is done here. Writes the updated
        errExit("write");                                       // record into the file

    if (lseek(fd, -sizeof(record), SEEK_CUR) == -1)             // Move fd's file position back to the start
        errExit("lseek");                                       // of the record, one last time

    record R;
    Read(fd, &R, sizeof(record));                               // Now, read the updated record from the file

    record_print(R, getpid(), WRITER);                          // Print the updated record

    sleep(gen_num(max_time, 0));                                // "Work" for some time

    int new_balance = R.balance;                                // Get its new balance
    check_range_and_unlock(shm_ptr, index, 1, prev_balance, new_balance, elapsed_time(start_tm));

    sem_post(&(shm_ptr->conc_proc));                            // Since you are exiting the system

    shut_file(fd);

	// Closing shmem
	shmem_close(shm_fd, shm_ptr);

	// Cleanup
    free_args(file_name, shm_path);

    return EXIT_SUCCESS;
}
