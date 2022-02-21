#ifndef __schedule_h__
#define __schedule_h__

#include "./file_utils.h"
#include "./mpi_utils.h"

void compute_word_frequency_master(long start, long part);

node *get_starting_point(long start, long part);

void compute_word_frequency_slave(buffer_split_work *work);


#endif