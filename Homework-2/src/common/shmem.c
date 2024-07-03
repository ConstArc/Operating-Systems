#include "../../include/common/global.h"
#include "../../include/common/shmem.h"


static void registry_init(registry* proc_registry);


void shmem_init(const char* shm_path, int* shmd_fd, shmem_segment** shm_ptr, int n_children) {
    // Creating shared memory object and setting its size to the size of our structure
    *shmd_fd = shm_open(shm_path, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (*shmd_fd == -1)
        errExit("shm_open");

    if (ftruncate(*shmd_fd, sizeof(shmem_segment)) == -1)
        errExit("ftruncate");

    *shm_ptr = mmap(NULL, sizeof(shmem_segment), PROT_READ | PROT_WRITE, MAP_SHARED, *shmd_fd, 0);
    if (*shm_ptr == MAP_FAILED)
        errExit("mmap");

    registry_init(&((*shm_ptr)->proc_registry));
    memset((*shm_ptr)->proc_registry.msg, '\0', MSG_LENGHT);
    (*shm_ptr)->accountant_terminated = false;

    for(int i=0; i < MAX_CONCURRENT_PROCESSES; i++) {
        if (sem_init(&((*shm_ptr)->conc_proc_arr[i].waiting_sem), 1, 0) == -1)
            errExit("sem_init-conc_proc_arr[i]->waiting_sem");

        (*shm_ptr)->conc_proc_arr[i].proc_type           =  -1;
        (*shm_ptr)->conc_proc_arr[i].index               =  -1;
        (*shm_ptr)->conc_proc_arr[i].wanted_range.start  =  -1;
        (*shm_ptr)->conc_proc_arr[i].wanted_range.finish =  -1;
        (*shm_ptr)->conc_proc_arr[i].priority            =  -1; 
        (*shm_ptr)->conc_proc_arr[i].waiting_time        = 0.0;
    }

    (*shm_ptr)->ticket_number = n_children;

    // Initializing semaphores as process-shared, with value 1
    if (sem_init(&(*shm_ptr)->conc_proc, 1, MAX_CONCURRENT_PROCESSES) == -1)
        errExit("sem_init-mutex");

    if (sem_init(&(*shm_ptr)->mutex, 1, 1) == -1)
        errExit("sem_init-mutex");

    if (sem_init(&((*shm_ptr)->accountant), 1, 0) == -1)
        errExit("sem_init-accountant");
    if (sem_init(&((*shm_ptr)->accountant_job_done), 1, 0) == -1)
        errExit("sem_init-accountant_job_done");
}

void shmem_open(int* shm_fd, const char* shm_path, shmem_segment** shm_ptr) {
    *shm_fd = shm_open(shm_path, O_RDWR, 0);
    if (*shm_fd == -1)
        errExit("shm_open");

    *shm_ptr = mmap(NULL, sizeof(shmem_segment), PROT_READ | PROT_WRITE, MAP_SHARED, *shm_fd, 0);
    if (*shm_ptr == MAP_FAILED)
        errExit("mmap");
}

void shmem_close(int shm_fd, shmem_segment* shm_ptr) {
    // Unmapping the shared memory
    if (munmap(shm_ptr, sizeof(shmem_segment)) == -1)
        errExit("munmap");

    // Closing the shared memory file descriptor
    if (close(shm_fd) == -1)
        errExit("close");
}

void shmem_clean(shmem_segment* shm_ptr, int shmd_fd, const char* shm_path) {
    sem_destroy(&(shm_ptr->conc_proc));
    sem_destroy(&(shm_ptr->mutex));
    sem_destroy(&(shm_ptr->accountant));
    sem_destroy(&(shm_ptr->accountant_job_done));
    for(int i=0; i < MAX_CONCURRENT_PROCESSES; i++) 
        sem_destroy(&(shm_ptr->conc_proc_arr[i].waiting_sem));

    if (munmap(shm_ptr, sizeof(shmem_segment)) == -1)
        errExit("munmap");

    if (close(shmd_fd) == -1)
        errExit("close");

    if (shm_unlink(shm_path) == -1)
        errExit("shm_unlink");
}

void registry_init(registry* proc_registry) {
    (*proc_registry).n_processes[READER] = 0;
    (*proc_registry).n_processes[WRITER] = 0;
    (*proc_registry).sum_active_proc_time[READER] = 0.0;
    (*proc_registry).sum_active_proc_time[WRITER] = 0.0;

    (*proc_registry).n_records_processed = 0;
    (*proc_registry).n_records_by_type[READER] = 0;
    (*proc_registry).n_records_by_type[WRITER] = 0;

    (*proc_registry).max_waiting_time    = 0.0;
}
