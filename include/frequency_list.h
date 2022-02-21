#ifndef __frequency_list_h__
#define __frequency_list_h__


#define HASHSIZE 50000
#define WORDLENGHT 40

int hash_tab_size;

// define the list's node 
struct hashNode {
    char word[WORDLENGHT];
    int value;
    struct hashNode *next;
};

//  define the array of nodes
struct hashNode *hashTab[HASHSIZE];

void insert_update(char *wrd, int value);

int get_hash_size();

void inizialize_hs_tab_size();

void free_hash();

#endif