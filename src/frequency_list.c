#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "./frequency_list.h"

//  from K&R book, HS func version 2
unsigned hash(char *word) {
    unsigned hashval;
    for (hashval = 0; *word != '\0'; word++) {    //  for every char of the string 
        hashval = *word + 31 * hashval;
    }
    return hashval % HASHSIZE;   // index 
}



struct hashNode *search(char *wrd) {
    //  get the hash 
    int hashIndex = hash(wrd);  
    struct hashNode *node = hashTab[hashIndex];

    //  scroll nodes with the same HashIndex
    while (node != NULL ) {
        if (strcmp(node->word , wrd) == 0) 
            return node; 
        node = node->next; 
    }
    return NULL; 
}

void insert_update(char *wrd, int val) {

    struct hashNode *node;

    // search for the item
    if ((node = search(wrd)) == NULL) {

        node = calloc(1, sizeof(struct hashNode));
     //   (*hash_tab_size)++;
        hash_tab_size++;

        int hashIndex = hash(wrd);
   
        strcpy(node->word, wrd); 
        node->value = val;

        //  link the node to the last one added
        node->next = hashTab[hashIndex];

        hashTab[hashIndex] = node;

        //  debug 
        //printf("%ld add = %s - %d \n", (long)getpid(),node->word, node->value);
    }
    //  just increment the item's value
    else {
        node->value += val;
        //  debug 
        //printf("%ld update = %s - %d \n", (long)getpid() ,node->word, node->value);
    }
}

void free_hash() {
    struct hashNode *curr;
    struct hashNode *succ;

    for (int i = 0; i < HASHSIZE; i++) {
        curr = hashTab[i];
        while (curr != NULL) {
            succ = curr->next;
            free(curr);
            curr = succ;
        }
    }
}

int get_hash_size() {
    return hash_tab_size;
}

void inizialize_hs_tab_size(){
   hash_tab_size = 0;
}


/*  // =========  DEBUG ONLY ==========  
int main() {
    insert_update("ciao", 1);
    insert_update("ciao", 1);
    insert_update("va", 1);
    struct hashNode* test = malloc(sizeof(struct hashNode));
    test = search("ciao");
    printf(" %s, %d \n", test->word, test->value);
}   */

