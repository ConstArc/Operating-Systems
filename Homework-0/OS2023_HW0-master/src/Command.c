#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "../include/Command.h"
#include "../include/DataBase.h"
#include "../include/Voter.h"
#include "../include/utils.h"

#define INPUT_SIZE 768          // buffer size for input from the user

// Initializing the global variable for total bytes freed at exit
// here:
size_t bytes_freed = 0;

static void cmd_l(const char*, const DataBase, int token_count);		// l command
static void cmd_i(const char*, const DataBase, int token_count);		// i command
static void cmd_m(const char*, const DataBase, int token_count);		// m command
static void cmd_bv(const char*, const DataBase, int token_count);		// bv command
static void cmd_v(const DataBase, int token_count);						// v command
static void cmd_perc(const DataBase, int token_count);					// perc commmand
static void cmd_o(const DataBase, int token_count);						// o command
static void cmd_z(const char*, const DataBase, int token_count);		// z command
static bool cmd_exit(const DataBase, int token_count);					// exit command


// This function executes the various commands, one at a time, by checking initially the token passed
// (first argument of command) to match it with the appropriate command.
// Parameters:
//        token:        In order to check which command is to be executed and take the next arguments
//        DB:           The database of the program
//        token_count:  The number of arguments of the given command
static bool exec_cmd(const char* token, const DataBase DB, int token_count);

// This function's sole purpose is to initialize the database with the file name given from the user
// from the command line arguments
static void init_file_db(const DataBase DB, char* file_name);

// ------------------------------ COMMAND ------------------------------ //

void RunDB(char* file_path, int m, int bucket_size, float LOAD_THRESHOLD) {

    if(file_path == NULL) {										// make a first check
        printf("File could not be opened\n");					// for the file_path given
        exit(EXIT_FAILURE);
    }

    DataBase db = database_create(m, bucket_size, LOAD_THRESHOLD);		// database creation
    init_file_db(db, file_path);								        // filling the database with initial file

    // allocate memory for the input
	char* input = malloc(sizeof(char) * (INPUT_SIZE + 1)); 	            // +1 for the string terminating character
    if(input == NULL) {
        fprintf(stderr, "Error: Memory allocation failure | While allocating memory for: input.\n");
        exit(EXIT_FAILURE);
    }

    while(1) {
        printf("Give input: ");

        if (fgets(input, INPUT_SIZE+1, stdin) != NULL) {

            // In case the user presses enter without writing anything else
            if(strcmp(input, "\n") == 0) {
                printf("Malformed Input\n");
                continue;
            }

            // Checking if input exceeded the buffer size
            if (strchr(input, '\n') == NULL)  {
                fprintf(stderr, "Input:\n \"%s...\"\n\texceeded default buffer size set to %d.\n", input, INPUT_SIZE-1);
                clearInput();
                continue;
            }

            // Trimming the new line character from the input
            trimInput(input);

            int token_count = tokenCount(input);                // counting the number of words in input

            // Tokenizing the command with whitespace delimeter. Token now will
            // point to the first argument of the command given
            char* token = strtok(input, " ");

            // Make a first filter for the first argument and the total tokens count
            if (token == NULL || *token == '\0' || token_count < 1) {
                printf("Malformed Input\n");
                continue;
            }

            // Finally, if the above filters have passed, call execute command
            // function with the appropriate parameters
            if(exec_cmd(token, db, token_count) == false)
                break;
        }
        else
            fprintf(stderr, "Error reading input.\n");
    }

    free(input);
    bytes_freed += sizeof(char) * (INPUT_SIZE+1);
}


void init_file_db(const DataBase DB, char* file_path) {
    assert(DB != NULL);

    database_insert_file(DB, file_path);

    // We won't be needing file_path any longer, so free allocated space
    free(file_path);
}

bool exec_cmd(const char* token, const DataBase DB, int token_count) {
    assert(DB != NULL);

    if(strcmp(token, "l") == 0) {
        cmd_l(token, DB, token_count);
        return true;
    }
    if(strcmp(token, "i") == 0) {
        cmd_i(token, DB, token_count);
        return true;
    }
    if(strcmp(token, "m") == 0) {
        cmd_m(token, DB, token_count);
        return true;
    }
    if(strcmp(token, "bv") == 0) {
        cmd_bv(token, DB, token_count);
        return true;
    }
    if(strcmp(token, "v") == 0) {
        cmd_v(DB, token_count);
        return true;
    }
    if(strcmp(token, "perc") == 0) {
        cmd_perc(DB, token_count);
        return true;
    }
    if(strcmp(token, "z") == 0) {
        cmd_z(token, DB, token_count);
        return true;
    }
    if(strcmp(token, "o") == 0) {
        cmd_o(DB, token_count);
        return true;
    }
    if(strcmp(token, "exit") == 0) {
        if(cmd_exit(DB, token_count))
            return false;
        return true;
    }

    printf("Malformed Input\n");
    return true;
}

void cmd_l(const char* token, const DataBase DB, int token_count) {
    if(token_count != 2) {
        printf("Malformed Input\n");
        return;
    }

    // pin
    token = strtok(NULL, " ");
    if(!isPositiveIntegerNumber(token)) {
        printf("Malformed Pin\n");
        return;
    }
    int pin = atoi(token);

    Voter v = database_search(DB, pin);
    if(v == NULL) {
        printf("Participant %d not in cohort\n", pin);
        return;
    }

    voter_print(v);
}

void cmd_i(const char* token, const DataBase DB, int token_count) {
    if(token_count != 5) {
        printf("Malformed Input\n");
        return;
    }

    // pin
    token = strtok(NULL, " ");
    if(!isPositiveIntegerNumber(token)) {
        printf("Malformed Input\n");
        return;
    }
    int pin = atoi(token);

    if(database_exists(DB, pin)) {
        printf("%d already exist\n", pin);
        return;
    }

    // last name
    token = strtok(NULL, " ");
    if(token == NULL) {
        printf("Malformed Input\n");
        return;
    }
    const char* lname = token;
    
    // first name
    token = strtok(NULL, " ");
    if(token == NULL) {
        printf("Malformed Input\n");
        return;
    }
    const char* fname = token;

    // zipcode
    token = strtok(NULL, " ");
    if(!isPositiveIntegerNumber(token)) {
        printf("Malformed Input\n");
        return;
    }
    int zipcode = atoi(token);

    if(database_exists(DB, pin)) {
        printf("%d already exist\n", pin);
        return;
    }

    Voter v = voter_create(pin, fname, lname, zipcode);
    database_insert(DB, v);
    printf("Inserted %d %s %s %d N\n", pin, lname, fname, zipcode);
}   

void cmd_m(const char* token, const DataBase DB, int token_count) {
    if(token_count != 2) {
        printf("Malformed Input\n");
        return;
    }

    // pin
    token = strtok(NULL, " ");
    if(!isPositiveIntegerNumber(token)) {
        printf("Malformed Input\n");
        return;
    }
    int pin = atoi(token);

    database_mark_voter_voted(DB, pin);
}

void cmd_bv(const char* token, const DataBase DB, int token_count) {
    if(token_count != 2) {
        printf("Malformed Input\n");
        return;
    }

    // file
    token = strtok(NULL, " ");
    if(token == NULL) {
        printf("%s could not be opened\n", token);
        return;
    }
    
    database_voters_file_voted(DB, token);
}

void cmd_v(const DataBase DB, int token_count) {
    if(token_count != 1) {
        printf("Malformed Input\n");
        return;
    }
    
    printf("Voted So Far %d\n", database_n_voters_voted(DB));
}

void cmd_perc(const DataBase DB, int token_count) {
    if(token_count != 1) {
        printf("Malformed Input\n");
        return;
    }

    printf("%.4f\n", database_perc(DB));
}

void cmd_o(const DataBase DB, int token_count) {
    if(token_count != 1) {
        printf("Malformed Input\n");
        return;
    }

    database_zipcodes_n_voters(DB);
}

void cmd_z(const char* token, const DataBase DB, int token_count) {
    if(token_count != 2) {
        printf("Malformed Input\n");
        return;
    }

    // zipcode
    token = strtok(NULL, " ");
    if(!isPositiveIntegerNumber(token)) {
        printf("Malformed Zipcode\n");
        return;
    }

    int zipcode = atoi(token);
    
    database_n_voters_voted_zipcode(DB, zipcode);
}

bool cmd_exit(const DataBase DB, int token_count) {
    if(token_count != 1) {
        printf("Malformed Input\n");
        return false;
    }

    database_destroy(DB);
    printf("%ld of Bytes Released\n", bytes_freed);
    return true;
}
