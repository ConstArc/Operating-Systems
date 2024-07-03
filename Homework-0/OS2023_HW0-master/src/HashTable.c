#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/HashTable.h"
#include "../include/LinkedList.h"
#include "../include/Voter.h"

// State of the bucket
typedef enum { empty = 0, full } b_state;


// ------------------------------ STRUCTS ------------------------------ //
typedef struct bucket* Bucket;

typedef struct bucket {
    Voter*  voters_array;              // voters array
    int     b_size;                    // bucket size 
    b_state state;                     // state of the bucket (empty/full)
    Bucket  next;                      // next bucket (overflown)
} bucket;


typedef struct hash_table {
    Bucket* buckets_array;          // array of buckets (1-d vertical)
    size_t  init_size;              // initial size (variable m from the paper)
    size_t  size;                   // the current size (the number of non-overflown buckets)
    size_t  size_old;               // the size (the number of non-overflown buckets) from the previous round
    int     n_voters_voted;         // total number of voters who have voted
    int     b_size;                 // bucket size
    int     n_voters;               // number of voters
    float   lambda;                 // load factor  (variable lambda from the paper)
    size_t  p_index;                // index p (variable p from the paper)
    int     round;                  // round
    float   l_threshold;            // load threshold
} hash_table;


// ------------------------- HASH TABLE UTILS ------------------------- //

// A simple function that returns the number 2^i
static int power_of_two(int i) {
    if(i < 0)
        return 0;
    return 1 << i;
}

// As with the paper of this exercise, the hash function we are going
// to be using is the following
static int hash_function(int key, int i, int m) {
    return key % (power_of_two(i) * m);
}

// Given the Hashtable, this function calculates the load factor
static float calc_lambda(HashTable H) {
    return ((float)H->n_voters) / ((float)(H->size * H->b_size));
}


// ------------------------------ BUCKET ------------------------------ //

// A constructor for the bucket, given the bucket size
static Bucket bucket_create(int bucket_size) {
    Bucket B = malloc(sizeof(bucket));
    if(B == NULL) {
        fprintf(stderr, "Error: Memory allocation failure | While allocating memory for: bucket.\n");
        exit(EXIT_FAILURE);
    }

    B->b_size = bucket_size;
    B->voters_array = malloc(sizeof(Voter)*bucket_size);
    if(B->voters_array == NULL) {
        fprintf(stderr, "Error: Memory allocation failure | While allocating memory for: bucket.\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < bucket_size; i++) {
        B->voters_array[i] = NULL;
    }
    B->state = empty;           // initially the bucket's state is empty
    B->next = NULL;             // and there are no overflown buckets

    return B;
}


// This function destroys the bucket and frees allocated space
// if count_bytes flag is false it doesn't count the bytes freed,
// otherwise it adds them to the bytes_freed global variable
static void bucket_destroy(Bucket B, bool count_bytes) {
    if(B == NULL)
        return;

    if(!count_bytes) {
        free(B->voters_array);
        free(B);
        B = NULL;
        return;
    }

    for(int i = 0; i < B->b_size; i++) {
        if(B->voters_array[i] == NULL)
            break;
        // Only by this point, we are freeing the voters
        voter_destroy(B->voters_array[i]);
    }
    free(B->voters_array);
    bytes_freed += sizeof(Voter)* B->b_size;

    free(B);
    B = NULL;
    bytes_freed += sizeof(bucket);
}

// Inserts a voter within the bucket B
static void bucket_insert(const Bucket B, Voter V) {
    assert(B != NULL);
    assert(V != NULL);

    int counter = 0;
    for(int i=0; i < B->b_size; i++) {
        if(B->voters_array[i] == NULL) {
            B->voters_array[i] = V;
            counter++;
            break;
        }
        counter++;
    }

    // If we reached the end of the bucket's size,
    // update the state of the bucket to full
    if(counter == B->b_size)
        B->state = full;
}

// Searches for a voter with voter_pin = pin. If they
// are not found, returns NULL
static Voter bucket_search(const Bucket B, int pin) {
    for(int i=0; i < B->b_size; i++) {
        if(B->voters_array[i] == NULL)                  // Reached the end
            break;
        if(voter_get_pin(B->voters_array[i]) == pin)
            return B->voters_array[i];
    }

    // If such voter isn't found
    return NULL;   
}

// Returns the voter stored at B->voters_array[index]
static Voter bucket_remove_voter(const Bucket B, int index) {
    assert(B != NULL);
    assert(B->b_size > index);

    return B->voters_array[index];
}

// Empties the bucket_to_empty and sets it's state to empty
static void bucket_empty(Bucket bucket_to_empty) {
    for(int i=0; i < bucket_to_empty->b_size; i++)
        bucket_to_empty->voters_array[i] = NULL;

    // Update the state of the bucket
    bucket_to_empty->state = empty;
}

// Destroys the list of buckets starting from bucket B. If count_bytes
// is true, then we add the bytes freed as well at the bytes_freed variable
static void bucket_list_destroy(const Bucket B, bool count_bytes) {
    if(B == NULL)
        return;

    Bucket temp = B;
    Bucket temp_before = B;
    while(temp != NULL) {
        temp = temp->next;
        bucket_destroy(temp_before, count_bytes);
        temp_before = temp;
    }
    bucket_destroy(temp_before, count_bytes);           // destroy the last bucket
}


// ------------------------------ HASH TABLE ------------------------------ //

// ---------------- HASH TABLE HELPER FUNCTIONS ----------------- //

// Initial insertion of voter V at H->buckets_array[index]
static void hash_table_simple_insert(const HashTable H, const Voter V, size_t index) {
    assert(H != NULL);
    assert(V != NULL);
    assert(index < H->size);

    Bucket temp = H->buckets_array[index];
    while(temp->state == full) {
        if(temp->next != NULL)
            temp = temp->next;
        else {
            temp->next = bucket_create(H->b_size);      // overflow bucket
            temp = temp->next;
            break;
        }
    }
    bucket_insert(temp, V);
}

// Checks if a bucket split is needed
static bool hash_table_split_needed(const HashTable H) {
    assert(H != NULL);

    if(H->lambda > H->l_threshold)
        return true;
    return false;
}

// Performs bucket split
static void hash_table_split(const HashTable H) {
    assert(H != NULL);

    H->size++;                                                                  // Increase number of non-overflown buckets

    H->buckets_array = realloc(H->buckets_array, sizeof(Bucket) * H->size);     // Reallocate memory
    if(H->buckets_array == NULL) {
        fprintf(stderr, "Error: Memory reallocation failure | While reallocating memory for: buckets_array of hash_table.\n");
        exit(EXIT_FAILURE);
    }

    H->buckets_array[H->size - 1] = bucket_create(H->b_size);                   // Create the new bucket at the end of the buckets_array
}

// Resets p index, updates the round and the old size
static void hash_table_reset_p(const HashTable H) {
    assert(H != NULL);

    H->p_index = 0;             // Reset p
    H->round++;                 // Increase the round                               
    H->size_old = H->size;      // Update the old size to be equal to the current size
}

// Returns a list of voters that need to be redistributed
static List get_keys_for_redistribution(const HashTable H) {
    assert(H != NULL);

    Bucket temp = H->buckets_array[H->p_index];
    Voter v;
    List redistribute_keys_list = list_create(NULL, NULL, NULL);
    do {
        for(int i = 0; i < temp->b_size; i++) {
            v = bucket_remove_voter(temp, i);
            if(v == NULL)
                break;
            list_append(redistribute_keys_list, bucket_remove_voter(temp, i));
        }
        temp = temp->next;
    } while(temp != NULL);

    return redistribute_keys_list;
}

// Destroys all overflown buckets of H->buckets_array[H->p_index]
static void hash_table_destroy_overflown_buckets(const HashTable H) {
    assert(H != NULL);

    Bucket buckets_to_delete = H->buckets_array[H->p_index]->next;
    H->buckets_array[H->p_index]->next = NULL;
    bucket_list_destroy(buckets_to_delete, false);
}

// Performs redistribution of the voters inside the list redistribute_keys_list
static void hash_table_keys_redistribute(const HashTable H, List redistribute_keys_list) {
    assert(H != NULL);

    Voter v;
    while(!is_list_empty(redistribute_keys_list)) {
        v = list_remove(redistribute_keys_list);
        int h_i_plus_1 = hash_function(voter_get_pin(v), H->round+1, H->init_size);
        hash_table_simple_insert(H, v, h_i_plus_1);
    }
    
    // Free allocated space from helper list
    list_destroy(redistribute_keys_list, false);
}

// Redistribution stage of insertion
static void hash_table_redistribution(const HashTable H) {
    assert(H != NULL);

    // Get the voters we are going to redistribute
    List redistribute_keys_list = get_keys_for_redistribution(H);

    // Empty the head bucket
    bucket_empty(H->buckets_array[H->p_index]);

    // Destroy any overflown buckets at that index
    if(H->buckets_array[H->p_index]->next != NULL) {
        hash_table_destroy_overflown_buckets(H);
    }

    // Perform the redistribution
    hash_table_keys_redistribute(H, redistribute_keys_list);

    // Update the p index
    H->p_index++;
}


// -------------------- HASH TABLE FUNCTIONS -------------------- //

// Constructor for the HashTable
HashTable hash_table_create(int m, int bucket_size, float load_threshold) {
    
    HashTable H =  malloc(sizeof(hash_table));
    if(H == NULL) {
        fprintf(stderr, "Error: Memory allocation failure | While allocating memory for: hash_table.\n");
        exit(EXIT_FAILURE);
    }

    H->init_size    = m;
    H->size         = m;
    H->size_old     = m;
    H->b_size       = bucket_size;
    H->n_voters     = 0;
    H->lambda       = 0.0;
    H->p_index      = 0;
    H->round        = 0;
    H->l_threshold    = load_threshold;
    H->n_voters_voted = 0;

    H->buckets_array = malloc(sizeof(Bucket) * m);
    if(H->buckets_array == NULL) {
        fprintf(stderr, "Error: Memory allocation failure | While allocating memory for: buckets_array of hash_table.\n");
        exit(EXIT_FAILURE);
    }


    for(int i = 0; i < m; i++) {
        H->buckets_array[i] = bucket_create(H->b_size);
    }

    return H;
}

void hash_table_insert(const HashTable H, const Voter V) {
    assert(H != NULL);
    assert(V != NULL);

    int pin = voter_get_pin(V);

    // Calculate hash value of key
    size_t h_i = hash_function(pin, H->round, H->init_size);
    if(h_i < H->p_index)
        h_i = hash_function(pin, H->round + 1, H->init_size);

    // Initial insertion of key
    hash_table_simple_insert(H, V, h_i);

    // Increase number of keys of hash table
    H->n_voters++;

    // Increase number of voters who have voted, if V has voted
    if(voter_has_voted(V))
        H->n_voters_voted++;

    // Update load factor right away
    H->lambda = calc_lambda(H);
    
    // Check if splitting is needed
    if (hash_table_split_needed(H)) {

        // Splitting
        hash_table_split(H);

        // Redistribution
        hash_table_redistribution(H);

        // Check if the size has become double the old size. If
        // so, reset the p index and update the old size
        if (H->size == 2*H->size_old)
            hash_table_reset_p(H);
    }
}

Voter hash_table_search(const HashTable H, int pin) {
    assert(H != NULL);
    assert(pin >= 0);

    size_t h_i = hash_function(pin, H->round, H->init_size);
    if (h_i < H->p_index)
        h_i = hash_function(pin, H->round + 1, H->init_size);

    Bucket temp = H->buckets_array[h_i];
    while(temp != NULL) {
        Voter v = bucket_search(temp, pin);
        if(v != NULL)
            return v;
        temp = temp->next;
    }

    return NULL;
}

bool hash_table_exists(const HashTable H, int pin) {
    assert(H != NULL);
    assert(pin >= 0);

    if(hash_table_search(H, pin) != NULL)
        return true;
    return false;
}

int hash_table_n_voters(const HashTable H) {
    assert(H != NULL);

    return H->n_voters;
}

bool is_hash_table_empty(const HashTable H) {
    assert(H != NULL);

    return (H->n_voters == 0);
}

int hash_table_n_voters_voted(const HashTable H) {
    assert(H != NULL);

    return H->n_voters_voted;
}

Voter hash_table_mark_voter_voted(const HashTable H, int pin) {
    assert(H != NULL);
    assert(pin >= 0);

    Voter v = hash_table_search(H, pin);
    if(v == NULL) {
        printf("%d does not exist\n", pin);
        return NULL;
    }
    if(!voter_has_voted(v)) {
        voter_vote(v);
        H->n_voters_voted++;
        return v;
    }
    printf("%d Marked Voted\n", pin);
    return NULL;
}

float hash_table_perc(const HashTable H) {
    assert(H != NULL);

    return (((float)H->n_voters_voted)/ ((float)H->n_voters) )*100;
}

void hash_table_destroy(HashTable HT) {
    if(HT == NULL)
        return;

    for(size_t i = 0; i< HT->size; i++)
        bucket_list_destroy(HT->buckets_array[i], true);
    
    free(HT->buckets_array);
    bytes_freed += sizeof(Bucket) * HT->size;

    free(HT);
    HT = NULL;
    bytes_freed += sizeof(hash_table);
}

// The function below is for testing purposes only. It prints out all the voters' pins
// in the way they are stored within the HashTable HT. If you want to use it simply 
// uncomment the function's body here, and uncomment the use of this function at the 
// database_insert_file function of DataBase module.
// void hash_table_insert_test_print(HashTable HT) {
//     assert(HT != NULL);

//     Bucket temp;
//     int counter;
//     for(size_t i = 0; i < HT->size; i++) {
//         temp = HT->buckets_array[i];
//         printf("%ld:  ", i);
//         do {
//             printf("{ ");
//             counter = 1;
//             for(int j = 0; j < temp->b_size; j++) {
//                 if(temp->voters_array[j] == NULL)
//                     break;
//                 printf("|%d|", voter_get_pin(temp->voters_array[j]));
//                 if(counter != temp->b_size && temp->voters_array[counter] != NULL)
//                     printf(", ");
//                 counter++;
//             }
//             printf(" }-->");
//             temp = temp->next;
//         } while(temp != NULL);
//         printf("\n");
//     }
//     printf("Round: %d\n", HT->round);
//     printf("Load Factor: %f\n", HT->lambda);
// }
