#include "../../include/common/global.h"
#include "../../include/initlzer/initlzerUtils.h"
#include "../../include/common/record.h"
#include "../../include/common/inputUtils.h"
#include "../../include/common/processUtils.h"

static void run_writer(const char* file_name, const char* shm_path, int n_records);
static void run_reader(const char* file_name, const char* shm_path, int n_records);


// This function is responsible for reading and initializing the command line arguments -n, -f passed
// from the user in the initial execution of the program. It makes some filtering as well, before
// initializing the variables
void initlzer_args_init(int argc, char* argv[], int n_args, char** file_name, int* n_children, int max_n_children) {
    if(argc != n_args) {
		perror("Wrong number of arguments passed");
        exit(EXIT_FAILURE);
    }

	for(int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            str_init(argc, argv, file_name, 'f', &i);
			continue;
		}
		if (strcmp(argv[i], "-n") == 0) {
            num_init(argc, argv, n_children, 'n', &i, 1);
            continue;
        }

		fprintf(stderr, "Non recognized command-line argument: %s\n", argv[i]);
        exit(EXIT_FAILURE);
	}
    if((*n_children) > max_n_children) {
        fprintf(stderr, "Maximum number of children processes allowed is: %d\n", max_n_children);
        exit(EXIT_FAILURE);
    }
}

// Opens the file with file name = [file_name] and uses lseek to count the
// number of records, where each record has a size equal to sizeof(record),
// within the opened file. Lastly it closes the file.
int records_count(char* file_name) {
    int fd = open(file_name, O_RDONLY);
    if(fd == -1)
		errExit("open");

	// Calculate the number of records within the file
    int size = (int) lseek(fd, 0, SEEK_END);
    if(size == -1)
        errExit("lseek");

    if(close(fd) == -1)
		errExit("close");

    return size/sizeof(record);
}

void run_accountant(const char* logfile_name, const char* shm_path, pid_t* accountant_pid) {
	*accountant_pid = fork();

	if(*accountant_pid < -1)
		errExit("fork");
	if(*accountant_pid == 0) {
		execl("./bin/accountant", "accountant", "-f", logfile_name, "-s", shm_path, NULL);

		errExit("execl");
	}
	else return;
}

void run_worker(const char* file_name, const char* shm_path, int n_records) {
	srand(time(NULL) ^ getpid());

	// 50% chance
	if(rand() <= RAND_MAX / 2)
		run_writer(file_name, shm_path, n_records);
	else
		run_reader(file_name, shm_path, n_records);
}

void print_stats(shmem_segment* shm_ptr) {
	printf("\n\n");

	printf("Workers finished execution normally! Printing last stats...\n\n");
	printf("- Total number of readers worked on file: %d\n", shm_ptr->proc_registry.n_processes[READER]);
	double avg_time_readers = 0.0;
	if(shm_ptr->proc_registry.n_processes[READER] != 0)
		avg_time_readers = ((double)shm_ptr->proc_registry.sum_active_proc_time[READER])/((double) shm_ptr->proc_registry.n_processes[READER]);
	printf("- Average time of readers: %.4f (sec)\n", avg_time_readers);

	printf("- Total number of writers worked on file: %d\n", shm_ptr->proc_registry.n_processes[WRITER]);
	double avg_time_writers = 0.0;
	if(shm_ptr->proc_registry.n_processes[WRITER] != 0)
		avg_time_writers = ((double)shm_ptr->proc_registry.sum_active_proc_time[WRITER])/((double) shm_ptr->proc_registry.n_processes[WRITER]);
	printf("- Average time of writers: %.4f (sec)\n", avg_time_writers);

	printf("- Maximum waiting time of worker (reader/writer): %.4f (sec)\n", shm_ptr->proc_registry.max_waiting_time);

	printf("- Total number of records processed: %d\n\n", shm_ptr->proc_registry.n_records_processed);
}

void wait_children(int n_children, pid_t* children_pid_array) {
	assert(children_pid_array != NULL);

	int status;
	for(int i = 0; i < n_children; i++) {
		waitpid(children_pid_array[i], &status, 0);
	}
}

void run_writer(const char* file_name, const char* shm_path, int n_records) {
	const char* recid = itoa(gen_num(n_records, 1));
	const char* value = itoa(gen_num(WRTR_MAX_VALUE, WRTR_MIN_VALUE));
	const char* time  = itoa(gen_num(WRTR_MAX_TIME, WRTR_MIN_TIME));
	execl("./bin/writer", "writer", "-f", file_name, "-l", recid, "-v", value, "-d", time, "-s", shm_path, NULL);

	errExit("execl");
}

void run_reader(const char* file_name, const char* shm_path, int n_records) {
	int recid = gen_num(n_records, 1);
	const char* recid_start  = itoa(recid);
	const char* recid_finish = itoa(gen_num(n_records, recid));
	const char* time 		 = itoa(gen_num(RDR_MAX_TIME, RDR_MIN_TIME));

	if(strcmp(recid_start, recid_finish) == 0)
		execl("./bin/reader", "reader", "-f", file_name, "-l", recid_start, "-d", time, "-s", shm_path, NULL);
	else
		execl("./bin/reader", "reader", "-f", file_name, "-l", recid_start, ",", recid_finish, "-d", time, "-s", shm_path, NULL);
		

	errExit("execl");
}

