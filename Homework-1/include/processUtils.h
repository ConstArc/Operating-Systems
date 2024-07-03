#pragma once
#include "record.h"

enum {READ, WRITE};

typedef struct Child_Args {
    pid_t   pid;
    int     start;
    int     capacity;
    int     n_children;
    record* result;
} child_args;

// A wrapper function for read(), it iteratively stores bytes from 
// fd into dest, until all n_bytes are succesfully read
void Read(int fd, void* dest, size_t n_bytes, int file_code);

// Creates an array of lenght n_children of pipes
void pipes_init(int*** pipes, int n_children, int file_code);
// Destroys the above array and frees allocated space
void pipes_destroy(int** pipes, int n_children);
// Sets up a redirection of the standard output of given child
// and performs other pipe related procedures to setup the pipe
// before the child's exec*
void prepare_pipe(int** pipe, int child);

// Creates an array of lenght n_children of child_args
void children_args_init(child_args** children_args, int n_children, int file_code);
// Destroys the above array and frees allocated space
void children_args_destroy(child_args* children_args);

// This function contains the code run in the splitting phase of the coordinator and workloader
void RunSplitter(int child, int** pipe_r, child_args* children_args, int file_code, pid_t child_pid);
// This function contains the code run in the merging phase of the coordinator and workloader
record* RunMerger(int n_children, child_args* children_args, int** pipe_r, int file_code);

// Sets the start and capacity of a given child
void set_child_record_bounds(int child, int* child_start, int start, int* child_capacity, int parent_capacity, int n_children, int n_records);

// Wait for all n_children processes to finish
void wait_children(child_args* children_args, int n_children, int file_code);