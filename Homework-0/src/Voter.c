#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/Voter.h"


// ------------------------------ VOTER ------------------------------ //
typedef struct voter {
    int pin;
    char* name;
    char* surname;
    int postal_code;
    char has_voted;
} voter; 


Voter voter_create(int pin, const char* name, const char* surname, int postal_code) {
    Voter V = malloc(sizeof(voter));
    assert(V != NULL);

    V->pin = pin;
    int name_len = strlen(name);
    int surname_len = strlen(surname);

    V->name = malloc(sizeof(char) * (name_len + 1));
    if(V->name == NULL) {
        fprintf(stderr, "Error: Memory allocation failure | While allocating memory for: V->name.\n");
        exit(EXIT_FAILURE);
    }
    strcpy(V->name, name);

    V->surname = malloc(sizeof(char) * (surname_len + 1));
    if(V->surname == NULL) {
        fprintf(stderr, "Error: Memory allocation failure | While allocating memory for: V->surname.\n");
        exit(EXIT_FAILURE);
    }
    strcpy(V->surname, surname);

    V->postal_code = postal_code;
    V->has_voted = 'N';

    return V;
}

bool voter_has_voted(const Voter V) {
    assert(V != NULL);

    return (V->has_voted == 'Y');
}

void voter_vote(const Voter V) {
    assert(V != NULL);

    V->has_voted = 'Y';
}

int voter_get_pin(const Voter V) {
    assert(V != NULL);

    return V->pin;
}

int voter_get_zip(const Voter V) {
    assert(V != NULL);

    return V->postal_code;
}

void voter_print(const Pointer P) {
    if(P == NULL) {
        printf("{ -- empty slot -- }\n");
        return;
    }

    Voter v = (Voter) P;
    printf("%d %s %s %d %c\n", v->pin, v->surname, v->name, v->postal_code, v->has_voted);
}

void voter_print_pin(const Pointer P) {
    if(P == NULL) {
        printf("{ -- empty slot -- }\n");
        return;
    }

    Voter v = (Voter) P;
    printf("\t%d\n", v->pin);
}

void voter_destroy(const Pointer P) {
    if(P == NULL)
        return;
    
    Voter v = (Voter) P;
    int name_len = strlen(v->name);
    int surname_len = strlen(v->surname);

    free(v->name);
    bytes_freed += sizeof(char) * (name_len + 1);

    free(v->surname);
    bytes_freed += sizeof(char) * (surname_len + 1);

    free(v); 
    bytes_freed += sizeof(voter);
}
