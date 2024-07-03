#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/DataBase.h"
#include "../include/HashTable.h"
#include "../include/InvertedIndex.h"
#include "../include/Voter.h"


// ------------------------------ DATABASE ------------------------------ //
typedef struct database {
    HashTable       ht;                   // The hashtable of the database
    InvertedIndex   inv_ind;              // The inverted index of the database 
} database;


DataBase database_create(int m, int bucket_size, float load_threshold) {
    DataBase DB = malloc(sizeof(database));
    DB->ht      = hash_table_create(m, bucket_size, load_threshold);
    DB->inv_ind = inv_index_create();
    return DB;
}

void database_insert_file(const DataBase DB, char* file_path) {
    assert(DB != NULL);
    assert(file_path != NULL);

    int voter_pin;
    char voter_name[256];
    char voter_surname[256];
    int voter_zipcode;

    FILE* file = fopen(file_path, "r");
    if(file == NULL) {
        printf("%s could not be opened\n", file_path);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    while (fscanf(file, "%d %255s %255s %d", &voter_pin, voter_surname, voter_name, &voter_zipcode) == 4) {

        // If voter_pin already exists in our DB, then we printout an error message,
        // and simply exit the program with EXIT_FAILURE.
        if(hash_table_exists(DB->ht, voter_pin)) {
            fprintf(stderr, "Error: In initial file insertion. Pin: %d has duplicate appearances. File is %s.\n", voter_pin, file_path);
            fclose(file);
            exit(EXIT_FAILURE);
        }

        Voter v = voter_create(voter_pin, voter_name, voter_surname, voter_zipcode);
        hash_table_insert(DB->ht, v);
    }

    if (ferror(file)) {
        fprintf(stderr, "Error: In initial file insertion. File is %s.\n", file_path);
        fprintf(stderr, "Error occurred while reading the file %s.\n", file_path);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // The funtion below is only for testing purposes, simply uncomment it
    // and uncomment the function's body at the end of the file HashTable.c
    
    // hash_table_insert_test_print(DB->ht);

    fclose(file);
}

void database_voters_file_voted(const DataBase DB, const char* file_name) {
    assert(DB != NULL);
    assert(file_name != NULL);

    FILE* file = fopen(file_name, "r");
    if(file == NULL) {
        printf("%s could not be opened\n", file_name);
        return;
    }

    int voter_pin;
    char line[256];
    while (fgets(line, sizeof(line), file)) {

        if (sscanf(line, "%d", &voter_pin) != 1 || voter_pin < 0) {
            printf("Malformed Input\n");
            fclose(file);
            return;
        }

        Voter v = hash_table_mark_voter_voted(DB->ht, voter_pin);
        if(v != NULL) {
            inv_index_insert(DB->inv_ind, v);
            printf("%d Marked Voted\n", voter_pin);
        }
    }    

    if (ferror(file)) {
        fprintf(stderr, "Error occurred while reading the file\n");
        fclose(file);
        return;
    }

    fclose(file);
}

void database_insert(const DataBase DB, Voter V) {
    assert(DB != NULL && V != NULL);

    hash_table_insert(DB->ht, V);
}

Voter database_search(const DataBase DB, int pin) {
    assert(DB != NULL);

    return hash_table_search(DB->ht, pin);
}

bool database_exists(const DataBase DB, int pin) {
    assert(DB != NULL);

    return hash_table_exists(DB->ht, pin);
}

int database_n_voters_voted(const DataBase DB) {
    assert(DB != NULL);

    return hash_table_n_voters_voted(DB->ht);
}

void database_n_voters_voted_zipcode(const DataBase DB, int zipcode) {
    assert(DB != NULL);

    inv_index_n_voters_zipcode(DB->inv_ind, zipcode);
}

void database_zipcodes_n_voters(const DataBase DB) {
    assert(DB != NULL);

    inv_index_zipcodes_n_voters(DB->inv_ind);
}

void database_mark_voter_voted(const DataBase DB, int pin) {
    assert(DB != NULL);

    // Firstly, update the entry with the pin, inside the 
    // hash table
    Voter v = hash_table_mark_voter_voted(DB->ht, pin);

    // If v is NULL that means that the voter v either doesn't
    // exist, or he/she has already voted. Therefore perform 
    // insertion in our inverted index struct, only if v != NULL
    if(v != NULL) {
        inv_index_insert(DB->inv_ind, v);
        printf("%d Marked Voted\n", pin);
    }
}

float database_perc(const DataBase DB) {
    assert(DB != NULL);

    return hash_table_perc(DB->ht);
}

void database_destroy(const DataBase DB) {
    if(DB == NULL)
        return;

    inv_index_destroy(DB->inv_ind);
    hash_table_destroy(DB->ht);
    
    free(DB);
    bytes_freed += sizeof(database);
}
