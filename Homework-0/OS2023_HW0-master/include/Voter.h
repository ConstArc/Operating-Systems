#pragma once
#include <stdbool.h>
#include "Global.h"

// ------------------------------ VOTER ------------------------------ //

// Creates a voter
Voter voter_create(int Pin, const char* Name, const char* LastName, int PostalCode);

// Checks if voter V has voted
bool voter_has_voted(const Voter V);

// Marks a voter to have voted
void voter_vote(const Voter V);

// Gets the pin of the voter V
int voter_get_pin(const Voter V);

// Gets the pin of V
int voter_get_zip(const Voter V);

// Prints in a simple format the voter
void voter_print(const Pointer voter);

// Prints only the pin from the voter
void voter_print_pin(const Pointer voter);

// Destroys voter and frees allocated memory
void voter_destroy(const Pointer voter);
