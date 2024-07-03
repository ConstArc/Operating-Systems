#pragma once
#include <stdbool.h>
#include <stdio.h>
#include "Global.h"

typedef struct database* DataBase; 

// ------------------------------ DATABASE ------------------------------ //

// Creates a DataBase
DataBase database_create(int initial_size, int bucket_size, float load_threshold);

// Opens the file with file_name, reads and saves line by line its contents (the voters)
// into the DB
void database_insert_file(const DataBase DB, char* file_name);

// Opens the file with file_name, reads line by line its contents (the voters) and marks
// as "voted" each of these voters (that also exist within the DB) in the DB.
void database_voters_file_voted(const DataBase DB, const char* file_name);

// Inserts a voter V into the DB
void database_insert(const DataBase DB, Voter V);

// Searchs for a voter V within the DB
Voter database_search(const DataBase DB, int Pin);

// Checks if a voter with pin = Pin exists in the DataBase
bool database_exists(const DataBase DB, int Pin);

// Marks a voter (if found in the DB) with pin = Pin, to have voted
void database_mark_voter_voted(const DataBase DB, int Pin);

// Returns the number of voters who have voted
int database_n_voters_voted(const DataBase DB);

// Prints the zipcodes along with the number of voters with those zipcodes, 
// in a decreasing order
void database_zipcodes_n_voters(const DataBase DB);

// Prints the number of voters who have voted, with zipcode = Zipcode
void database_n_voters_voted_zipcode(const DataBase DB, int Zipcode);

// Returns the percentage of voters who have voted in the DataBase, in comparison
// with the total voters in the DataBase
float database_perc(const DataBase DB);

// Destroys the DataBase and frees allocated memory
void database_destroy(const DataBase DB);
