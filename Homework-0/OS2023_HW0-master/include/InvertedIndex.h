#pragma once
#include <stdbool.h>
#include <stdio.h>
#include "Global.h"

typedef struct invterted_index* InvertedIndex;

// ------------------------------ INVERTED INDEX ------------------------------ //

// Creates an inverted index
InvertedIndex inv_index_create(void);

// Inserts a voter into the inverted index
void inv_index_insert(const InvertedIndex INV_INDEX, const Voter V);

// Prints the number of voters who have voted, with zipcode = Zipcode
void inv_index_n_voters_zipcode(const InvertedIndex INV_INDEX, int Zipcode);

// Prints the zipcodes along with the number of voters with those zipcodes, 
// in a decreasing order
void inv_index_zipcodes_n_voters(const InvertedIndex INV_INDEX);

// Destroys the inverted index and frees allocated memory
void inv_index_destroy(const InvertedIndex INV_INDEX);
