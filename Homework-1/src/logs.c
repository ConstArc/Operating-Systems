#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include "../include/logs.h"

// Given a file code it returns the file_name of that particular file.
// File codes are defined on the top of the src files that contain a main
static char* find_file_name(int file_code);

// ----------------------------- LOGS ----------------------------- //
void LOG_OPEN(int file_code) {
    char* file_name = find_file_name(file_code);
    fprintf(stderr, "[%d] LOG   | In %s| Given file was opened succesfully\n", getpid(), file_name);
    free(file_name);
}

void LOG_READ(int file_code) {
    char* file_name = find_file_name(file_code);
    fprintf(stderr, "[%d] LOG   | In %s| Given file was read   succesfully\n", getpid(), file_name);
    free(file_name);
}

void LOG_WRITE(int file_code) {
    char* file_name = find_file_name(file_code);
    fprintf(stderr, "[%d] LOG   | In %s| Given file was written succesfully\n", getpid(), file_name);
    free(file_name);
}

void LOG_FORK(int file_code, pid_t child_pid) {
    char* file_name = find_file_name(file_code);
    fprintf(stderr, "[%d] LOG   | In %s| Succesfully forked a child process: [%d]\n", getpid(), file_name, child_pid);
    free(file_name);
}

void LOG_WAIT(int file_code, int status, int child_index, pid_t child_pid) {
     char* file_name = find_file_name(file_code);
    fprintf(stderr, "[%d] LOG   | In %s| Child process: [%d] with index: %3d," 
                    " terminated with Status: %d\n", getpid(), file_name, child_pid, child_index, status);
    free(file_name);
}


// ---------------------------- ERRORS -------------------------- //
void ERROR_MALLOC(int file_code) {
    char* file_name = find_file_name(file_code);
    fprintf(stderr, "[%d] ERROR | In %s| Malloc failure\n", getpid(), file_name);
    free(file_name);
}

void ERROR_OPEN(int file_code) {
    char* file_name = find_file_name(file_code);
    perror("open failed");
    fprintf(stderr, "[%d] ERROR | In %s| Couldn't open given file\n", getpid(), file_name);
    free(file_name);
}

void ERROR_READ(int file_code) {
    char* file_name = find_file_name(file_code);
    perror("read failed");
    fprintf(stderr, "[%d] ERROR | In %s| Read didn't work correctly\n", getpid(), file_name);
    free(file_name);
}

void ERROR_WRITE(int file_code) {
    char* file_name = find_file_name(file_code);
    perror("write failed");
    fprintf(stderr, "[%d] ERROR | In %s| write didn't work correctly\n", getpid(), file_name);
    free(file_name);
}

void ERROR_FORK(int file_code) {
    char* file_name = find_file_name(file_code);
    perror("fork failed");
    fprintf(stderr, "[%d] ERROR | In %s| Failed to fork\n", getpid(), file_name);
    free(file_name);
}

void ERROR_WAIT(int file_code, int status, int child, pid_t child_pid) {
    char* file_name = find_file_name(file_code);
    perror("wait failed");
    fprintf(stderr, "[%d] ERROR | In %s| Child process: [%d] with index: %d," 
                    " terminated with Status: %d\n", getpid(), file_name, child_pid, child, status);
    free(file_name);
}

void ERROR_EXEC(int file_code, int child) {
    char* file_name = find_file_name(file_code);
    perror("exec failed");
    fprintf(stderr, "[%d] ERROR | In %s| Exec failure for child process: [%d] with index: %d", 
                        getppid(), file_name, getpid(), child); 
    free(file_name);
}

// --------------------- Utility of logger  --------------------- //
char* find_file_name(int file_code) {
    char* file_name;
    switch (file_code) {
        case 1:
            file_name = malloc(sizeof("coordinator.c  "));
            return strcpy(file_name, "coordinator.c  ");
        case 2:
            file_name = malloc(sizeof("workloader.c   "));
            return strcpy(file_name, "workloader.c   "); 
        case 3:
            file_name = malloc(sizeof("mergesort.c    "));
            return strcpy(file_name, "mergesort.c    "); 
        case 4:
            file_name = malloc(sizeof("heapsort.c     "));
            return strcpy(file_name, "heapsort.c     "); 
        case 5:
            file_name = malloc(sizeof("inputUtils.c   "));
            return strcpy(file_name, "inputUtils.c   "); 
        case 6:
            file_name = malloc(sizeof("processUtils.c "));
            return strcpy(file_name, "processUtils.c ");
        case 7:
            file_name = malloc(sizeof("record.c       "));
            return strcpy(file_name, "record.c       ");  
        default:
            file_name = malloc(sizeof("- Not Found -  "));
            return strcpy(file_name, "- Not Found -  ");
    }
}