#ifndef __file_utils_h__
#define __file_utils_h__

// Path to files to read
#define FILEPATH "./Files"

#define PATHMODE "/" 
//  data to keep in memory before write into the report file
#define CHUNK_SIZE 4096 

typedef struct node *lista;

// define the list's node 
typedef struct node {
    char *fname;
    long size;
    long seek_line;
    struct node *next;
}node;


// create a list of file from DIR
node *create_list_of_files_from_dir();

node *getHead();

long get_total_files_sizes();

long get_file_size(const char* filename);

void printlist();

void build_report_file(float time, int rank);

void free_list_of_files();

#endif