#include "../../include/common/global.h"
#include "../../include/logging/loggingUtils.h"
#include "../../include/common/inputUtils.h"

#define TOTAL_ARGS 3


int main(int argc, char* argv[]) {
    char* shm_path;
    logger_args_init(argc, argv, TOTAL_ARGS, &shm_path);

    int shm_fd;
	shmem_segment* shm_ptr;
	shmem_open(&shm_fd, shm_path, &shm_ptr);

    sem_wait(&(shm_ptr->mutex));
        print_registry(shm_ptr, stdout, LOGGER);
    sem_post(&(shm_ptr->mutex));

    // Cleanup
    free_args(NULL, shm_path);

    return EXIT_SUCCESS;
}
