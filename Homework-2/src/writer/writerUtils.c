#include "../../include/common/global.h"
#include "../../include/common/shmem.h"
#include "../../include/writer/writerUtils.h"
#include "../../include/common/inputUtils.h"


void writer_args_init(int argc, char** argv, int n_args, char** file_name, int* recid, int* value, int* time, char** shm_path) {
    if(!validNumberOfArgs(argc, n_args)) {
		perror("Wrong number of arguments passed");
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            str_init(argc, argv, file_name, 'f', &i);
            continue;
        }
        if (strcmp(argv[i], "-l") == 0) {
            num_init(argc, argv, recid, 'l', &i, 1);
            (*recid)--;
            continue;
        }
        if (strcmp(argv[i], "-d") == 0) {
			num_init(argc, argv, time, 'd', &i, 0);
			continue;
        }
        if (strcmp(argv[i], "-v") == 0) {
            num_init(argc, argv, value, 'v', &i, -1);
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
