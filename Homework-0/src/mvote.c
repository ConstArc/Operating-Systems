#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/utils.h"
#include "../include/Command.h"

#define TOTAL_N_ARGS 7
#define LOAD_THRESHOLD 0.75


int main(int argc, char* argv[]) {

    char* file_name = NULL;
    int bucket_size;
    int m;

    if(!validArgs(argc, argv, &file_name, &bucket_size, &m, TOTAL_N_ARGS))
        return 1;

    RunDB(file_name, m, bucket_size, LOAD_THRESHOLD);
    
    return 0;
}
