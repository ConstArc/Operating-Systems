#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/utils.h"


// ------------------------------ UTILS ------------------------------ //

// As a first filter, this function simply checks if the number of
// arguments is equal to the expected number of total arguments.
static bool validNumberOfArgs(int argc, int total_args) {
    if (argc != total_args) {
        printf("Not accepted number of command-line arguments.\n");
        return false;
    }
    return true;
}

// Given a string, the function below discards the newline character
// and replaces it with the terminal character, thus trimming the \n.
void trimInput(char* input) {
    if (input == NULL)
        return;

    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '\n') {                 // trim the new line character
            input[i] = '\0';
            break;
        }
    }
}

// Given a string, the function below counts the number of
// different words in it (delimeter here is the whitespace).
// Use this function with great care as it only works with
// the whitespace delimeter and each word should only be 
// seperated by one and only one whitespace character.
int tokenCount(const char* input) {
    if(input == NULL || *input == '\0')
        return 0;

    int count = 1;

    for(int i = 0; input[i] != '\0'; i++) {
        if(input[i] == ' ')
            count++;
    }

    return count;
}

// Given a string, the function below checks if the string is
// a positive integer number. Use this function with great care
// as it only works with ascii codes.
bool isPositiveIntegerNumber(const char* str) {
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

void clearInput(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// This function is responsible for reading the command line arguments passed
// from the user, in initial execution of the program. It makes some filtering
// as well, before initializing the variables: 
//    file_name   : the file we are going to initialize our database with
//    bucket_size : the size of the buckets in the hashtable
//    m           : the initial size of the hashtable
bool validArgs(int argc, char** argv, char** file_name, int* bucket_size, int* m, int max_args) {
    if(!validNumberOfArgs(argc, max_args))
        return false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            int len = strlen(argv[i+1]);
            *file_name = malloc(sizeof(char) * (len+1));
            strcpy(*file_name, argv[i+1]);
            i++;
        } 
        else if (strcmp(argv[i], "-b") == 0) {
            if ( (i + 1 < argc) && isPositiveIntegerNumber(argv[i+1]) ) {
                *bucket_size = atoi(argv[i+1]);
                i++;
            }
            else {
                fprintf(stderr, "Error: -b option requires a positive integer argument.\n");
                if(*file_name != NULL)
                    free(*file_name);
                return false;
            }
        }
        else if (strcmp(argv[i], "-m") == 0) {
            if ( (i + 1 < argc) && isPositiveIntegerNumber(argv[i+1]) ) {
                *m = atoi(argv[i+1]);
                i++;
            }
            else {
                fprintf(stderr, "Error: -m option requires a positive integer argument.\n");
                if(*file_name != NULL)
                    free(*file_name);
                return false;
            }
        } 
        else {
            fprintf(stderr, "Non recognized command-line argument: %s\n", argv[i]);
            return false;
        }
    }
    return true;
}
