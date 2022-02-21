#ifndef __mpi_utils_h__
#define __mpi_utils_h__


typedef struct buffer_split_work{
    long part_size;
    long start;
    char f_name[60];
} buffer_split_work;

void master_job(int np);

void master_split_work(buffer_split_work *work, int np);

void slave_receive_work();

void slave_job();


#endif