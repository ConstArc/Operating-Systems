#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include "../include/record.h"
#include "../include/inputUtils.h"
#include "../include/processUtils.h"
#include "../include/logs.h"

#define FILE_CODE 2         // Logging purposes
#define TOTAL_N_ARGS 7      // Validation of argv

static void RunSorter(int sorter, char* data_file, int s_start, int s_capacity, char sort_algo, int** pipe_r, pid_t coordinator_pid);
static void PassTimeMeasurements(int n_sorters, int** pipe_r, child_args* children_args);

int main(int argc, char* argv[]) {

    if(argc != TOTAL_N_ARGS) {
        perror("Wrong number of arguments passed to workloader\n");
        exit(EXIT_FAILURE);
    }

    // Saving arguments to variables
    char* data_file = argv[1];
    int   n_sorters = atoi(argv[2]);
    int   start     = atoi(argv[3]);
    int   capacity  = atoi(argv[4]);
    char  s1        = argv[5][0];              // the first  sorting algorithm passed from the user (-e1)
    char  s2        = argv[6][0];              // the second sorting algorithm passed from the user (-e2)

    pid_t coordinator_pid = getppid();

    // Create read-set of pipes to read data from sorters to workloader
    int** pipe_r;
    pipes_init(&pipe_r, n_sorters, FILE_CODE);
    
    // Create an array of child_args (sorter arguments). This is for ease of handling them in both
    // the splitting phase and the merging (of the results) phase
    child_args* children_args;
    children_args_init(&children_args, n_sorters, FILE_CODE);

    // The splitting process
    pid_t sorter_pid;
    int   s_start = start;          // We will change this variable over the for-loop and assign its value to the next sorter's start
    int   n_records_assigned = 0;   // Keeping track of how many records have been assigned alredy to sorters
    char  sort_algo = s1;           // First child will always start with s1
    for(int sorter = 0; sorter < n_sorters; sorter++) {
        
        // Find the start and capacity of given sorter, in terms of number of records
        set_child_record_bounds(sorter, &(children_args[sorter].start), s_start, &(children_args[sorter].capacity), capacity, n_sorters, n_records_assigned);
        s_start += children_args[sorter].capacity;                  // Update the starting point of records, for next iteration
        n_records_assigned += children_args[sorter].capacity;       // Increase the number of records assigned so far
        
        sorter_pid = fork();                                        // Execute fork and check if everything went ok
        if(sorter_pid < 0) {
            ERROR_FORK(FILE_CODE);
            exit(EXIT_FAILURE);
        }

        if(sorter_pid != 0) {                                       // Splitter Process (Parent)
            RunSplitter(sorter, pipe_r, children_args, FILE_CODE, sorter_pid);
        }
        
        if(sorter_pid == 0)                                         // Sorter Process (Child) -either mergesort or heapsort
            RunSorter(sorter, data_file, children_args[sorter].start, children_args[sorter].capacity, sort_algo, pipe_r, coordinator_pid); 
        
        if(sort_algo == s1)                                         // Swap sorting algorithm
            sort_algo = s2;
        else if(sort_algo == s2)
            sort_algo = s1;
    }

    // Merge results from children processes (sorters) and save the final result
    record* final_result = RunMerger(n_sorters, children_args, pipe_r, FILE_CODE);

    // Write final result to pipe
    size_t n_bytes = sizeof(record) * capacity;
    write(STDOUT_FILENO, final_result, n_bytes);

    PassTimeMeasurements(n_sorters, pipe_r, children_args);         // Pass time measurements from sorters to coordinator

    // Wait for all sorters to complete, just in case
    wait_children(children_args, n_sorters, FILE_CODE);

    // Cleanup
    free(final_result);
    children_args_destroy(children_args);
    pipes_destroy(pipe_r, n_sorters);

    // Send signal to coordinator letting them know that we have finished succesfully
    kill(coordinator_pid, SIGUSR1);
    exit(EXIT_SUCCESS);
}


// ----------- Basic functionalities of workloader ------------ //
void RunSorter(int sorter, char* data_file, int s_start, int s_capacity, char sort_algo, int** pipe_r, pid_t coordinator_pid) {
    assert(data_file != NULL);
    assert(pipe_r != NULL);

    prepare_pipe(pipe_r, sorter);                   // Redirection of standard output to pipe

    // Overlaying the calling process here with the executable: sorter
    if(sort_algo == 'h')
        execl("./output/sorterH", "sorterH", data_file, itoa(s_start), itoa(s_capacity), itoa(coordinator_pid), NULL);
    else if(sort_algo == 'm')
        execl("./output/sorterM", "sorterM", data_file, itoa(s_start), itoa(s_capacity), itoa(coordinator_pid), NULL);

    ERROR_EXEC(FILE_CODE, sorter);                  // Shouldn't have reached here
    exit(EXIT_FAILURE);
}

void PassTimeMeasurements(int n_sorters, int** pipe_r, child_args* children_args) {
    assert(pipe_r != NULL);
    assert(children_args != NULL);
    
    double cpu_time;
    double real_time;
    for(int child = 0; child < n_sorters; child++) {
        // Read cpu_time
        Read(pipe_r[child][READ], &cpu_time,  sizeof(double), FILE_CODE);
        // Write cpu_time
        write(STDOUT_FILENO, &cpu_time, sizeof(double));

        // Read real_time
        Read(pipe_r[child][READ], &real_time,  sizeof(double), FILE_CODE);
        // Write real_time
        write(STDOUT_FILENO, &real_time, sizeof(double));
    }
}