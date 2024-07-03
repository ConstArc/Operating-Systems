#include "../../include/common/global.h"
#include "../../include/common/processUtils.h"
#include "../../include/common/inputUtils.h"
#include "../../include/common/shmem.h"
#include "../../include/reader/readerUtils.h"

void reader_args_init(int argc, char** argv, int max_n_args, char** file_name, int* rec_start, int* rec_finish, int* time, char** shm_path) {
    if(!validNumberOfArgs(argc, max_n_args) && !validNumberOfArgs(argc, max_n_args-2)) {
		perror("Wrong number of arguments passed");
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            str_init(argc, argv, file_name, 'f', &i);
            continue;
        } 
        if (strcmp(argv[i], "-l") == 0) {
            num_init(argc, argv, rec_start, 'l', &i, 1);
            (*rec_start)--;         // indexing starts from 0

            if (strcmp(argv[i+1], ",") == 0) {
                i++;
                num_init(argc, argv, rec_finish, 'l', &i, 1);
                (*rec_finish)--;    // indexing starts from 0
                if( ((*rec_finish) == 0) || ((*rec_finish) == (*rec_start)) )
                    *rec_finish = *rec_start;
            }
            else
                *rec_finish = *rec_start;

            continue;
        }
        if (strcmp(argv[i], "-d") == 0) {
            num_init(argc, argv, time, 'd', &i, 0);
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

void reader_capacity_init(int* capacity, int finish, int start) {
    *capacity = 1;
	if(finish != start)
		*capacity = finish - start + 1;

    assert(*capacity > 0);
}

void reader_print_records(int* sum_balance, int capacity, record* records) {
    pid_t pid = getpid();
    for(int i=0; i < capacity; i++) {
        record_print(records[i], pid, READER);
        (*sum_balance) += record_get_balance(records[i]);
    }
}

void reader_print_avg_balance(int sum_balance, int capacity) {
    double avg_balance = ((double) sum_balance) / ((double) capacity);
    pid_t pid = getpid();

    fflush(stdout);
    printf("[%d] | READER | Average of balances of records read: %.4f\n", pid, avg_balance);
}