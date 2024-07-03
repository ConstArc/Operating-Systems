#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "../include/logs.h"
#include "../include/inputUtils.h"

// ------------------------------ INPUT UTILS ------------------------------ //

// As a first filter, this function simply checks if the number of
// arguments is equal to the expected number of total arguments.
static bool validNumberOfArgs(int argc, int total_args) {
    if (argc != total_args) {
        printf("Not accepted number of command-line arguments.\n");
        return false;
    }
    return true;
}

// Given a string, the function below checks if the string is
// a positive integer number. Use this function with great care
// as it only works with ascii codes.
static bool isPositiveIntegerNumber(const char* str) {
    if (str == NULL || *str == '\0')
        return false;

    // if a '-' sign is in the front of the string
    // we consider it as a negative value
    if(str[0] == '-')
        return false;
    
    for (int i = 0; str[i] != '\0'; i++) {
        // in case a '+' sign is in the front of the string
        if(i == 0 && str[i] == '+')
            continue;
        
        // if the string is 0
        if(i == 0 && str[i] == '0')
            return false;

        // checks the ascii characters
        if (str[i] < '0' || str[i] > '9')
            return false;
    }

    return true;
}

// This function is responsible for reading the command line arguments passed
// from the user in the initial execution of the program. It makes some filtering
// as well, before initializing the variables: 
bool validArgs(int argc, char** argv, char** file_name, int* k, char** e1, char** e2, int max_args) {
    if(!validNumberOfArgs(argc, max_args))
        return false;

    int len;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0) {
            len = strlen(argv[i+1]);
            *file_name = malloc(len+1);
            if(*file_name == NULL) {
                perror("malloc failure");
                return false;
            }
            strcpy(*file_name, argv[i+1]);
            i++;
            continue;
        } 
        if (strcmp(argv[i], "-k") == 0) {
            if ( (i + 1 < argc) && isPositiveIntegerNumber(argv[i+1]) ) {
                *k = atoi(argv[i+1]);
                i++;
                continue;
            }
            fprintf(stderr, "Error: -k option requires a positive integer argument.\n");
            return false;
        }
        if (strcmp(argv[i], "-e1") == 0) {
            len = strlen(argv[i+1]);
            *e1 = malloc(len+1);
            if(*e1 == NULL) {
                perror("malloc failure");
                return false;
            }
            strcpy(*e1, argv[i+1]);
            i++;
            continue;
        } 
        if (strcmp(argv[i], "-e2") == 0) {
            len = strlen(argv[i+1]);
            *e2 = malloc(len+1);
            if(*e2 == NULL) {
                perror("malloc failure");
                return false;
            }
            strcpy(*e2, argv[i+1]);
            i++;
            continue;
        } 
        fprintf(stderr, "Non recognized command-line argument: %s\n", argv[i]);
        return false;
    }
    return true;
}

// This function simply converts a given positive number to a
// string by the use of snprintf
char* itoa(size_t num) {
    int str_len = snprintf(NULL, 0, "%ld", num);
    char* str = malloc(str_len + 1);
    if(str == NULL) {
        perror("malloc failure");
        exit(EXIT_FAILURE);
    }

    snprintf(str, str_len + 1, "%ld", num);
    return str;
}

// Cleanup function for input arguments
void freeInput(char* file_name, char* s1, char* s2) {
    if (file_name != NULL)
        free(file_name);
    if (s1 != NULL)
        free(s1);
    if (s2 != NULL)
        free(s2);
}
