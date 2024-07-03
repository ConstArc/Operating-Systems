#pragma once

#include "../common/shmem.h"
#include "../common/record.h"
enum {ACCOUNTANT, LOGGER};

// Initializes -f, -s flags for the accountant and makes some filtering as well
void accountant_args_init(int argc, char* argv[], int n_args, char** logfile_name, char** shm_path);

// Initializes -s flag for the logger and makes some filtering as well
void logger_args_init(int argc, char* argv[], int n_args, char** shm_path);

// A simple print function that prints into [logfd], the registry kept in shared memory
// pointed by [shm_ptr] and the active/blocked pid array of readers and the
// active/blocked pid array of writers. The print is done in a formatted manner
void print_registry(shmem_segment* shm_ptr, FILE* logfd, int log_type);