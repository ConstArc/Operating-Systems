#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include "../include/inputUtils.h"
#include "../include/processUtils.h"
#include "../include/logs.h"
#include "../include/record.h"

#define FILE_CODE 1         // Logging purposes
#define TOTAL_N_ARGS 9      // Validation of argv

// Initialiaze global variables for signal counting here
int workloader_sig = 0;
int sorter_sig     = 0;

// Modified signal handlers for SIGUSR1 and SIGUSR2 
static void succesful_workloader_exit();
static void succesful_sorter_exit();

// This function simply opens the given file, counts all the records within it and closes it
static size_t records_count(char* DataFile);
char find_first_sort_algo(char* sorting1, char* sorting2);

static void PrintTimeMeasurements(int NumofChildren, child_args* children_args, int** pipe_r);
static void PrintSignals(void);
static void RunWorkloader(int workloader, child_args* children_args, char* DataFile, int** pipe_r, char* s1, char* s2);

int main(int argc, char* argv[]) {

    char* DataFile = NULL;
    char* sort1;
    char* sort2;
    int   NumofChildren;

    // Initialize and make a simple initial validation of the command line args to be used
    if(!validArgs(argc, argv, &DataFile, &NumofChildren,  &sort1, &sort2, TOTAL_N_ARGS)) {
        freeInput(DataFile, sort1, sort2);
        exit(EXIT_FAILURE);
    }

    // Overwrite SIGUSR1, SIGUSR2 signal handling routines
    signal(SIGUSR1, succesful_workloader_exit);
    signal(SIGUSR2, succesful_sorter_exit);

    // Get total number of records of given file
    int n_records = records_count(DataFile);

    // Create read-set of pipes to read result data from mergers (workloaders) to coordinator
    int** pipe_r;
    pipes_init(&pipe_r, NumofChildren, FILE_CODE);

    // Create an array of child_args (workloader arguments). This is for ease of handling them in both
    // the splitting phase and the merging (of the results) phase
    child_args* children_args;
    children_args_init(&children_args, NumofChildren, FILE_CODE);

    // Initialize sorting algorithms to pass to workloaders
    char s1[] = {'0', '\0'};
    char s2[] = {'0', '\0'};
    s1[0] = find_first_sort_algo(sort1, sort2);
    if(s1[0] == 'h')
        s2[0] = 'm';
    else
        s2[0] = 'h';

    // The splitting process
    pid_t workloader_pid;
    int   w_start = 0;              // we will change this variable over the for-loop and assign its value for the next workloader's start
    int   n_records_assigned = 0;   // keeping track of how many records have been assigned alredy to workloaders
    for(int workloader = 0; workloader < NumofChildren; workloader++) {

        children_args[workloader].n_children = NumofChildren - workloader;  // Find the number of sorters of given workloader

        // Find the start and capacity of given workloader, in terms of number of records
        set_child_record_bounds(workloader, &(children_args[workloader].start), w_start, &(children_args[workloader].capacity), n_records, NumofChildren, n_records_assigned);
        w_start            += children_args[workloader].capacity;           // Update the starting point of records, for next iteration
        n_records_assigned += children_args[workloader].capacity;           // Increase the number of records assigned so far

        workloader_pid = fork();                                            // Execute fork and check if everything went ok
        if(workloader_pid < 0) {
            ERROR_FORK(FILE_CODE);
            exit(EXIT_FAILURE);
        }

        if(workloader_pid != 0)                                             // Coordinator Process (Parent)
            RunSplitter(workloader, pipe_r, children_args, FILE_CODE, workloader_pid);

        if(workloader_pid == 0)                                             // Workloader Process  (Child)
            RunWorkloader(workloader, children_args, DataFile, pipe_r, s1, s2);
    }

    // The merging process
    // Read and merge results from children processes and save the final result
    record* final_result = RunMerger(NumofChildren, children_args, pipe_r, FILE_CODE);

    // Print final result
    for(int i = 0; i < n_records; i++)
        record_print(final_result[i]);

    PrintTimeMeasurements(NumofChildren, children_args, pipe_r);            // Time measurements from sorters

    // Wait for all workloaders to complete, just in case
    wait_children(children_args, NumofChildren, FILE_CODE);

    PrintSignals();                                                         // Signals from workloaders and sorters

    // Cleanup
    free(final_result);                         // Space for final result
    children_args_destroy(children_args);       // Space for children_args 
    pipes_destroy(pipe_r, NumofChildren);       // Space for pipe_r
    freeInput(DataFile, sort1, sort2);          // Space for input

    exit(EXIT_SUCCESS); 
}


// -------------------- Signal modification -------------------- //
void succesful_workloader_exit() {
    signal(SIGUSR1, succesful_workloader_exit);
    workloader_sig++;
}

void succesful_sorter_exit() {
    signal(SIGUSR2, succesful_sorter_exit);
    sorter_sig++;
}

// ----------------- Utilities of coordinator  ----------------- //
size_t records_count(char* DataFile) {

    int fd = open(DataFile, O_RDONLY);
    if (fd == -1) {
        ERROR_OPEN(FILE_CODE);
        exit(EXIT_FAILURE);
    }

    /* calculate the number of records in the file */
    size_t size = (size_t) lseek(fd, 0, SEEK_END);
    
    close(fd);
    return (size_t) size / sizeof(record);
}

char find_first_sort_algo(char* sorting1, char* sorting2) {
    // If heapsort is passed as the first sorting algorithm
    if(strcmp(sorting1, "heapsort") == 0) {
        return 'h';
    }
    // Else if heapsort is passed as the second sorting algorithm
    else if(strcmp(sorting2, "heapsort") == 0){
        return 'm';
    }

    // Shouldn't have reached here, problem from user's input
    fprintf(stderr, "[%d] Error | Wrong sorting algorithm passed at command line\n", getpid());
    exit(EXIT_FAILURE);
}

// ----------- Basic functionalities of coordinator ------------ //
void RunWorkloader(int workloader, child_args* children_args, char* DataFile, int** pipe_r, char* s1, char* s2) {
    assert(DataFile != NULL);
    assert(pipe_r != NULL);

    prepare_pipe(pipe_r, workloader);                   // Redirection of standard output to pipe

    // Overlaying the calling process here with the executable: workloader
    execl("./output/workloader", "workloader", DataFile, itoa(children_args[workloader].n_children), 
            itoa(children_args[workloader].start), itoa(children_args[workloader].capacity), s1, s2, NULL);
    
    ERROR_EXEC(FILE_CODE, workloader);                  // Shouldn't have reached here
    exit(EXIT_FAILURE);
}

void PrintTimeMeasurements(int NumofChildren, child_args* children_args, int** pipe_r) {
    double cpu_time;
    double real_time;
    for(int workloader = 0; workloader < NumofChildren; workloader++) {
        for(int sorter = 0; sorter < children_args[workloader].n_children; sorter++) {
            // Read cpu_time
            Read(pipe_r[workloader][READ], &cpu_time,  sizeof(double), FILE_CODE);
            // Read real_time
            Read(pipe_r[workloader][READ], &real_time,  sizeof(double), FILE_CODE);
            printf("Sorter %3d/%-3d of workloader %3d/%-3d: Run time was %lf sec (REAL time) although we used the CPU for %lf sec (CPU time).\n", 
                    sorter+1, children_args[workloader].n_children, workloader+1, NumofChildren, real_time, cpu_time);
        }
    }
}

void PrintSignals(void) {
    printf("Number of SIGUSR1 signals received at root (coordinator): %d\n", workloader_sig);
    printf("Number of SIGUSR2 signals received at root (coordinator): %d\n", sorter_sig);
}
