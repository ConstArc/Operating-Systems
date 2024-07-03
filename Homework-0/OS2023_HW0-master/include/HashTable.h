#pragma once
#include <stdbool.h>
#include "Global.h"

typedef struct hash_table* HashTable;

// ------------------------------ HASH TABLE ------------------------------ //

// Creates a HashTable that makes use of linear hashing
HashTable hash_table_create(int initial_size, int bucket_size, float load_threshold);

// Inserts a voter into the HashTable
void hash_table_insert(const HashTable HT, const Voter V);

// Searches for a voter within the HashTable, by using their pin
Voter hash_table_search(const HashTable HT, int Pin);

// Checks if a user exists within the HashTable, by using their pin
bool hash_table_exists(const HashTable HT, int Pin);

// Checks if HashTable is empty
bool is_hash_table_empty(const HashTable HT);

// Returns the number of voters -within the HashTable- who have voted
int hash_table_n_voters_voted(const HashTable HT);

// If found in the HashTable, marks voter with pin = Pin as has_voted
Voter hash_table_mark_voter_voted(const HashTable HT, int Pin);

// Returns the percentage of voters -within the HashTable- who have voted
float hash_table_perc(const HashTable HT);

// Destroys the HashTable and frees allocated memory
void hash_table_destroy(HashTable HT);


// --------------- HASH TABLE TESTING --------------- //

// This function is only for testing purposes.
// It prints out the pins of voters in the same way they are
// stored within the HashTable.
void hash_table_insert_test_print(HashTable HT);