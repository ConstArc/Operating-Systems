#include "../../include/common/global.h"
#include "../../include/logging/loggingUtils.h"
#include "../../include/common/inputUtils.h"

#define LINE_LENGTH 7

static void print_pid_arr(shmem_segment* shm_ptr, FILE* fd, int proc_type);

void accountant_args_init(int argc, char* argv[], int n_args, char** logfile_name, char** shm_path) {
    if(!validNumberOfArgs(argc, n_args)) {
        perror("Wrong number of arguments passed");
        exit(EXIT_FAILURE);
    }

	for(int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            str_init(argc, argv, logfile_name, 'f', &i);
			continue;
		}
        if (strcmp(argv[i], "-s") == 0) {
            str_init(argc, argv, shm_path, 's', &i);
            continue;
        }

		fprintf(stderr, "Non recognized command-line argument: %s\n", argv[i]);
        exit(EXIT_FAILURE);
	}
}

void logger_args_init(int argc, char* argv[], int n_args, char** shm_path) {
    if(!validNumberOfArgs(argc, n_args)) {
        perror("Wrong number of arguments passed");
        exit(EXIT_FAILURE);
    }

	for(int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            str_init(argc, argv, shm_path, 's', &i);
            continue;
        }

		fprintf(stderr, "Non recognized command-line argument: %s\n", argv[i]);
        exit(EXIT_FAILURE);
	}
}

void print_registry(shmem_segment* shm_ptr, FILE* logfd, int log_type) {
    assert(shm_ptr != NULL);

    fflush(logfd);


    if(log_type == ACCOUNTANT)
        fprintf(logfd, "\t<======================================= ACCOUNTANT =======================================>\n\n");
    else if(log_type == LOGGER)
        fprintf(logfd, "\n\t<========================================= LOGGER =========================================>\n\n");

    if(log_type == ACCOUNTANT) {
        fprintf(logfd, "- Message:\n");
        fprintf(logfd, "%s", shm_ptr->proc_registry.msg);
        memset(shm_ptr->proc_registry.msg, '\0', MSG_LENGHT);
        fprintf(logfd, "\n");
    }

    fprintf(logfd, "- Readers array of pids:\n");
    print_pid_arr(shm_ptr, logfd, READER);
    
    fprintf(logfd, "\n\n");
    fprintf(logfd, "- Writers array of pids:\n");
    print_pid_arr(shm_ptr, logfd, WRITER);

    fprintf(logfd, "\n\n");
    fprintf(logfd, "- Number of readers: %d\n", shm_ptr->proc_registry.n_processes[READER]);
    fprintf(logfd, "- Number of writers: %d\n", shm_ptr->proc_registry.n_processes[WRITER]);
    fprintf(logfd, "- Number of records processed by readers: %d\n", shm_ptr->proc_registry.n_records_by_type[READER]);
    fprintf(logfd, "- Number of records processed by writers: %d\n", shm_ptr->proc_registry.n_records_by_type[WRITER]);
    fprintf(logfd, "- Total number of records processed: %d\n", shm_ptr->proc_registry.n_records_processed);

    fprintf(logfd, "\t<==========================================================================================>\n\n");
}


// A simple print function that prints into [logfd] in a formatted manner, the
// active/blocked pids of [proc_type] processes kept in shared memory, which is
// pointed by [shm_ptr].
void print_pid_arr(shmem_segment* shm_ptr, FILE* logfd, int proc_type) {
    int  counter = 0;       // Counter needed for new line
    char state   = 'B';     // State is either going to be B (blocked) or A (active)

    for (int i = 0; i < MAX_CONCURRENT_PROCESSES; i++) {
        if (shm_ptr->conc_proc_arr[i].index == -1 || shm_ptr->conc_proc_arr[i].proc_type != proc_type)
            continue;
        
        if(shm_ptr->conc_proc_arr[i].blocked_by == 0)
            state = 'A';

        if(shm_ptr->conc_proc_arr[i].blocked_by > 0)
            state = 'B';

        counter++;
        fprintf(logfd, "| [%d/%c] ", shm_ptr->conc_proc_arr[i].pid, state);

        // Check if new line is needed
        if ((counter % LINE_LENGTH) == 0) {
            counter = 0;
            fprintf(logfd, "|\n");
        }
    }

    if (counter != 0)
        fprintf(logfd, "|\n");
}
