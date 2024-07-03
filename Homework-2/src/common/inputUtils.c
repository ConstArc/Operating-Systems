#include "../../include/common/global.h"
#include "../../include/common/inputUtils.h"

static bool isValidInteger(const char* str, int option);

// As a first filter, this function simply checks if the number of
// arguments is equal to the expected number of total arguments
bool validNumberOfArgs(int argc, int total_args) {
    if (argc != total_args)
        return false;

    return true;
}

// Given a string and an option (-1, 0, 1) the function below:
// option = -1: Checks if given number is integer               ('-' or '+' or '0')
// option =  0: Checks if given number is non negative integer  ('+' or '0')
// option =  1: Checks if given number is positive integer      ('+')
bool isValidInteger(const char* str, int option) {
    if (str == NULL || *str == '\0')
        return false;

    for (int i = 0; str[i] != '\0'; i++) {
        if ( (i == 0) && str[i] == '-')
            continue;
        // checks the ascii characters
        if (str[i] < '0' || str[i] > '9')
            return false;
    }
    if(option == -1) return true;

    // if a '-' sign is in the front of the string
    // we consider it as a negative value
    if(str[0] == '-')
        return false;
    
    for (int i = 0; str[i] != '\0'; i++) {
        // in case a '+' sign is in the front of the string
        if(i == 0 && str[i] == '+')
            continue;
    }
    if(option == 0) return true;

    // if the string is 0
    if(str[0] == '0')
        return false;

    return true;
}

void str_init(int argc, char** argv, char** file_name, char flag, int* i) {
    if ((*i) + 1 >= argc) {
        fprintf(stderr, "Error: -%c flag requires a valid file name.\n", flag);
        exit(EXIT_FAILURE);
    }
    *file_name = malloc( strlen(argv[(*i) + 1]) + 1 );
    if(*file_name == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    strcpy(*file_name, argv[(*i)+1]);
    (*i)++;
}

void num_init(int argc, char** argv, int* num, char flag, int* i, int option) {
    if ( ((*i) + 1 >= argc) || !isValidInteger(argv[(*i)+1], option) ) {
        if(option == -1)
            fprintf(stderr, "Error: -%c flag requires an integer number.\n", flag);
        else if(option == 0)
            fprintf(stderr, "Error: -%c flag requires a non negative integer number.\n", flag);
        else if(option == 1)
            fprintf(stderr, "Error: -%c flag requires a positive integer number.\n", flag);
        exit(EXIT_FAILURE);
    }
    *num = atoi(argv[(*i)+1]);
    (*i)++;
}


// This function simply converts a given integer number to a
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

// Cleanup function for command line arguments
void free_args(char* file_name1, char* file_name2) {
    if (file_name1 != NULL)
        free(file_name1);
    if (file_name2 != NULL)
        free(file_name2);
}