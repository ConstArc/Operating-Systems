#pragma once

// Maximum processes that we allow to exist concurrently within the system.
#define MAX_CONCURRENT_PROCESSES 256

// Maximum number of processes allowed for -n flag of user
#define MAX_PROCESSES 20000

// Size of message char array of registry
#define MSG_LENGHT 256

enum {READER, WRITER};

// Holds several information needed for logging purposes
typedef struct {
    char   msg[MSG_LENGHT];
    int    n_processes[2];              // Number of reader processes [0] and of writer [1] that have finished 
    int    n_records_processed;         // Total number of records processed (by both readers and writers)
    int    n_records_by_type[2];        // Number of records processed by readers [0] and writers [1]
    double sum_active_proc_time[2];     // Sum of active time (real time) of readers [0] and writers [1]
    double max_waiting_time;            // Maximum waiting time from both readers, writers
} registry;

// Range holds the starting index [start] and the last index [finish] of
// the first record and the last record respectively, that a worker wants
// to work with.
typedef struct {
    int start;
    int finish;
} range;

// Helper struct for the synchronization of the workers
typedef struct {
    pid_t  pid;             // Pid of process currently occupying particular proc_info struct
    sem_t  waiting_sem;     // Personal semaphore that's used by the process to put itself into sleep
    range  wanted_range;    // The range that the process wants to work on
    int    proc_type;       // The type of the process (reader or writer)
    int    index;           // The index of the cell
    int    priority;        // The priority of the process
    int    blocked_by;      // Counter of how many processes the process needs to wait for
    double waiting_time;    // The time measurement until the process got to work on with the wanted_range
} proc_info;

// The struct of the shared memory segment 
typedef struct {
    // Registry where we keep stats
    registry proc_registry;

    // Needed for critical section
    sem_t     conc_proc;
    int       ticket_number;
    sem_t     mutex;
    proc_info conc_proc_arr[MAX_CONCURRENT_PROCESSES];
    
    // Needed for logging
    bool  accountant_terminated;
    sem_t accountant;
    sem_t accountant_job_done;
} shmem_segment;

// Initializes the shared memory segment of the program
void shmem_init(const char* shm_path, int* shmd_fd, shmem_segment** shm_ptr, int n_children);

// Destroys and cleans the shared memory segment of the program
void shmem_clean(shmem_segment* shm_ptr, int shmd_fd, const char* shm_path);\

// Opens the shared memory segment
void shmem_open(int* shm_fd, const char* shm_path, shmem_segment** shm_ptr);

// Closes the shared memory segment
void shmem_close(int shm_fd, shmem_segment* shm_ptr);
