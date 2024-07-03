#include "../../include/common/global.h"
#include "../../include/logging/loggingUtils.h"
#include "../../include/common/inputUtils.h"

#define TOTAL_ARGS 5


int main(int argc, char* argv[]) {
    char* logfile;
    char* shm_path;
    accountant_args_init(argc, argv, TOTAL_ARGS, &logfile, &shm_path);

    int shm_fd;
	shmem_segment* shm_ptr;
	shmem_open(&shm_fd, shm_path, &shm_ptr);

    FILE* logfd = fopen(logfile, "a");
    if(logfd == NULL)
        errExit("fopen");

    while(true) {
        // Put yourself into sleep and wait for someone to wake you up
        sem_wait(&(shm_ptr->accountant));

        // Check if we are to be terminated
        if(shm_ptr->accountant_terminated)
            break;

        // Print the state of the system (stats) into file with fd = logfd
        print_registry(shm_ptr, logfd, ACCOUNTANT);

        // Signal the one who woke us up, that the job is done (Handshake)
        sem_post(&(shm_ptr->accountant_job_done));
    }

    if(fclose(logfd) == -1)
		errExit("close");

    // Cleanup
    free_args(logfile, shm_path);

    return EXIT_SUCCESS;
}
