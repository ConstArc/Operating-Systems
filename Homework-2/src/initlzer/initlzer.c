#include "../../include/common/global.h"
#include "../../include/initlzer/initlzerUtils.h"
#include "../../include/common/record.h"
#include "../../include/common/inputUtils.h"

#define TOTAL_ARGS 5
#define SHM_PATH "/shared_memory"
#define LOG_PATH "./log/Logs.log"

int main(int argc, char* argv[]) {

	char* file_name;
	int  n_children;

	// Initialize above variables from command line arguments
	initlzer_args_init(argc, argv, TOTAL_ARGS, &file_name, &n_children, MAX_PROCESSES);

	// Initialize shared memory segment
	int shmd_fd;
	shmem_segment* shm_ptr;
	shmem_init(SHM_PATH, &shmd_fd, &shm_ptr, n_children);

	// Count the number of records within the file
	int n_records = records_count(file_name);

	// Spawn accountant that shall continuously log the state of the system
	pid_t accountant_pid;
	run_accountant(LOG_PATH, SHM_PATH, &accountant_pid);
	
	// Create an array to store the pids of children
	pid_t* workers_pid_array = calloc(n_children, sizeof(pid_t));

	// Spawning children processes here
	pid_t worker_pid;
	for(int i = 0; i < n_children; i++) {
		worker_pid = fork();
		if(worker_pid < -1)
			errExit("fork");
		if(worker_pid != 0)
			workers_pid_array[i] = worker_pid;
		if(worker_pid == 0)
			run_worker(file_name, SHM_PATH, n_records);
	}

	// Waiting for all children processes to finish working
	wait_children(n_children, workers_pid_array);

	// A simple sanity check here
	assert(shm_ptr->ticket_number == 0);

	// Signal accountant that they must be terminated
	shm_ptr->accountant_terminated = true;
	sem_post(&(shm_ptr->accountant));

	// Wait for accountant
	int status;
	waitpid(accountant_pid, &status, 0);

	// Print last stats before exit
	print_stats(shm_ptr);

	// Cleanup and exit
	free(workers_pid_array);

	// Clean shared memory segment
	shmem_clean(shm_ptr, shmd_fd, SHM_PATH);
	
	free(file_name);

    return EXIT_SUCCESS;
}
