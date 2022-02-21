#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mpi.h"
#include "./frequency_list.h"
#include "./file_utils.h"
#include "./schedule.h"

//#define WORDLENGHT 40

typedef struct buffer_frequency_list{
    char word[WORDLENGHT];
    int value;
} buffer_frequency_list;

MPI_Datatype define_MPI_Type_split() {

    // mpi structure name
    MPI_Datatype mpi_elemnt;
    // number of structure members
    const int nitems = 3;

    // array of structure member sizes
    int blocklengths[3];
    blocklengths[0] = 1;
    blocklengths[1] = 1;
    blocklengths[2] = 60;    

    // structure member types
    MPI_Datatype types[3] = { MPI_LONG, MPI_LONG, MPI_CHAR };

    // offset of structure members
    MPI_Aint offsets[3];
    offsets[0] = offsetof( buffer_split_work, part_size);
    offsets[1] = offsetof( buffer_split_work, start);
    offsets[2] = offsetof( buffer_split_work, f_name);

    // create mpi struct
    MPI_Type_create_struct( nitems, blocklengths, offsets, types, &mpi_elemnt);
    MPI_Type_commit( &mpi_elemnt);           
    return mpi_elemnt;
}


MPI_Datatype define_MPI_Type_frequency() {
    // mpi structure name
    MPI_Datatype mpi_elemnt;
    // number of structure members
    const int nitems = 2;

    // array of structure member sizes
    int blocklengths[2];
    blocklengths[0] = WORDLENGHT;
    blocklengths[1] = 1;

    // structure member types
    MPI_Datatype types[2] = { MPI_CHAR, MPI_INT };

    // offset of structure members
    MPI_Aint offsets[2];
    offsets[0] = offsetof( buffer_frequency_list, word);
    offsets[1] = offsetof( buffer_frequency_list, value);

    // create mpi struct
    MPI_Type_create_struct( nitems, blocklengths, offsets, types, &mpi_elemnt);
    MPI_Type_commit( &mpi_elemnt);           
    return mpi_elemnt;
}


void reduce(buffer_frequency_list *send, int size) {
    //  scroll the array and add the items into the MASTER hashTab
    for (int i = 0; i < size ; i++) {
        insert_update(send[i].word, send[i].value);
    }
}

void master_split_work(buffer_split_work *send_work, int rank){
    MPI_Request job_to_do;
    MPI_Datatype mpi_elemnt;
    mpi_elemnt = define_MPI_Type_split();
    MPI_Isend(send_work, 1, mpi_elemnt, rank, 1, MPI_COMM_WORLD, &job_to_do);

    //MPI_Send(send_work, 1, mpi_elemnt, rank, 1, MPI_COMM_WORLD);
}

void slave_receive_work(){
    MPI_Datatype mpi_elemnt;
    MPI_Status status;
    mpi_elemnt = define_MPI_Type_split();

    buffer_split_work *send_work = calloc(1, sizeof(buffer_split_work));
  //  printf("START RECEIVING FROM MASTER... \n");
    MPI_Recv(send_work, 1, mpi_elemnt, 0, 1, MPI_COMM_WORLD, &status);   
   // printf("I Received %s, start from: %ld \n", send_work->f_name, send_work->start);
    compute_word_frequency_slave(send_work);
    
}


//  reveive the array for the slaves and copy it into the hashTab
void master_job(int np) {
    MPI_Status status;
    int tag = 0, sizebuf = 0;

    // Define MPI structure 
    MPI_Datatype mpi_elemnt;
    mpi_elemnt = define_MPI_Type_frequency();


    for(int j = 1; j < np; j++) {
        MPI_Recv(&sizebuf, 1, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
    
        buffer_frequency_list *send = (buffer_frequency_list*) calloc(sizebuf, sizeof(buffer_frequency_list));
        MPI_Recv(send, sizebuf, mpi_elemnt, status.MPI_SOURCE, tag, MPI_COMM_WORLD, &status);

        //printf("hHERE2  %d \n ", status.MPI_ERROR);
        reduce(send, sizebuf);
/*         for (int i = 0; i < sizebuf ; i++) {
            printf("slave: %d = %d - %s - %d \n", status.MPI_SOURCE , i , send[i].word, send[i].value);
        }  */
        free(send);
        sizebuf = 0;
    }
}
//  put the hashtab into an array of struct and send to the master
void slave_job() {
    int tag = 0;
    MPI_Datatype mpi_elemnt;
    mpi_elemnt = define_MPI_Type_frequency();
    MPI_Request buffer_size_request;

    struct hashNode *link;
    int size_hstab = get_hash_size();
    
    MPI_Isend(&size_hstab, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &buffer_size_request);
    buffer_frequency_list *send = (buffer_frequency_list*) calloc(size_hstab, sizeof(buffer_frequency_list));


    int j=0;
    for (int i = 0;  i < HASHSIZE ; i++) {
        link = hashTab[i];
        while (link != NULL) {
            send[j].value = link->value;
            strcpy(send[j].word, link->word);
            //printf("Slave 1 = %d - %s - %d \n" , j , send[j].word , send[j].value);
            j++;
            link = link->next;
        }
    }
    //printf( "j= %d - size_hstab= %d\n", j, size_hstab);
    MPI_Wait(&buffer_size_request, MPI_STATUS_IGNORE);

    MPI_Send(send, size_hstab, mpi_elemnt, 0, tag, MPI_COMM_WORLD);

    free(send);    
}
