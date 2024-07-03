#pragma once

#include "../common/shmem.h"

// These defines can be changed if needed
#define WRTR_MAX_VALUE  1000        // Maximum update of balance a writer can make
#define WRTR_MIN_VALUE -1000        // Minimum update of balance a writer can make
#define WRTR_MAX_TIME   4           // Maximum sleep time that can be given to a writer
#define WRTR_MIN_TIME   1           // Minimum sleep time that can be given to a writer
// Shouldn't be set to a value smaller than 0

#define RDR_MAX_TIME    8           // Maximum sleep time that can be given to a reader
#define RDR_MIN_TIME    1           // Minimum sleep time that can be given to a reader
// Shouldn't be set to a value smaller than 0

// Initializes the -f, -n flags and makes some initial filtering
void initlzer_args_init(int argc, char* argv[], int n_args, char** file_name, int* n_children, int max_n_children);

// Counts the number of struct record within a given file with name [file_name]
int records_count(char* file_name);

// Spawns the accountant that shall write in the file with name [logfile_name]
void run_accountant(const char* logfile_name, const char* shm_path, pid_t* accountant_pid);

// Spawns a worker (either reader or writer)
void run_worker(const char* file_name, const char* shm_path, int n_records);

// Waits for all children in children_pid_array
void wait_children(int n_children, pid_t* children_pid_array);

// Prints the stats of the system
void print_stats(shmem_segment* shm_ptr);
