#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./file_utils.h"
#include "./frequency_list.h"



// init the list of the files and the head
lista list_files = NULL; 
node *head = NULL;

//  the path address is setted in file_utils.h
lista create_list_of_files_from_dir(){
    node *tmp, *nodo;
    list_files = NULL;
    DIR *p;
    struct dirent *pp;     

    p = opendir (FILEPATH);
    if (p != NULL) {
        while ((pp = readdir (p)) != NULL) {
            /* ignore "." & ".." directories */
            if( !strcmp( ".", pp->d_name ) || !strcmp( "..", pp->d_name ) || !strcmp( ".DS_Store", pp->d_name ) )
                continue;

            // if the list is empty
            if(list_files == NULL) {

                nodo = (struct node*) malloc(sizeof(struct node)); 
                if (nodo == NULL) 
                    return list_files;  // if the malloc fail

                // add the file to the list
                nodo->fname = pp->d_name;
                nodo->size = get_file_size(pp->d_name);
                nodo->next = list_files;
                nodo->seek_line = 0;
                list_files = nodo;
                // set the head
                head = list_files;
            }

            //  add the others files to the list's tail 
            else {
                tmp = list_files; 

                // go to the end of the list
                while (tmp->next != NULL) {
                    tmp = tmp->next;
                }
                // add the file to the list
                nodo = (struct node*) malloc(sizeof(struct node)); 
                if (nodo == NULL) 
                    return list_files;  // if the malloc fail

                nodo->fname = pp->d_name;
                nodo->size = get_file_size(pp->d_name);
                nodo->seek_line = 0;
                nodo->next = NULL;
                tmp->next = nodo;
            }
        }
    }  
    return list_files; 
}

//  scroll each file into the list for sum the sizes
long get_total_files_sizes( ){
    long line = 0;
    while (list_files != NULL) {
        line += list_files->size;
        list_files = list_files->next;
    }
    return line;
}

// scroll the file for get the number of lines
long get_file_size(const char* filename) {
    char address[100] = FILEPATH PATHMODE;

    FILE* f;
    strcat(address , filename);  
    f = fopen(address, "r");

    if (f == NULL) 
        printf("Error open file for scan the lines\n");
     
    fseek(f, 0L, SEEK_END);
    long size = ftell(f);
    fclose(f);
    return size;
}

// for debug
void printlist(lista x){
    while (x != NULL) {
       printf("%s --  %ld \n", x->fname, x->size);
       x = x->next;
    }
    printf("\n");
}

//  return the headNode
node* getHead() {
    return head;
}

void free_list_of_files(){
    node *head = getHead();
    node *curr;
    while (head != NULL) {
        curr = head->next;
        free(head);
        head = curr;
    }
}

//  create a report file for show the results of the job
void build_report_file(float time, int rank) {
    FILE *F;
    char *file_name = calloc(64, sizeof(char));

    sprintf(file_name, "./logs/Report process %d.txt", rank);   
    if(!(F = fopen(file_name, "w")))
        printf("ERROR create report file\n");

    char file_buffer[CHUNK_SIZE + 64];
    int buffer_count = 0, i = 1, total_words = 0, different_words=0;
    struct hashNode *link;

    buffer_count += sprintf( &file_buffer[buffer_count], "      WORD           -       OCCURRENCY\n\n");

    //  scroll all the index of the hash tab
    while (i < HASHSIZE) {
        link = hashTab[i];
        //  check if there are more nodes on the same index
        while( link != NULL) {
            different_words++;
            total_words += link->value;
            buffer_count += sprintf( &file_buffer[buffer_count], "%-20s - %11d\n", link->word, link->value);
            link = link->next;

            //  if the chunk is big enough, write it.
            if( buffer_count >= CHUNK_SIZE )  {
                fwrite( file_buffer, CHUNK_SIZE, 1, F ) ;
                buffer_count -= CHUNK_SIZE ;
                memcpy( file_buffer, &file_buffer[CHUNK_SIZE], buffer_count ) ;
            }
        }
        i++;
    }
    buffer_count += sprintf( &file_buffer[buffer_count], "\n             WORDS: %d\n             OCCURRENCES: %d\n\n             TIME: %f MS\n", total_words, different_words,time);
    //if (rank == 0)
        //printf("TOT= %d differets %d\n", total_words, different_words);
    //  wirte remainder
    if( buffer_count > 0 ) 
        fwrite( file_buffer, 1, buffer_count, F );  
}
