
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
// files support library
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include "./file_utils.h"
#include "./schedule.h"
#include "./mpi_utils.h" 
#include "./frequency_list.h" 



int main(int argc, char *argv[]) {
    int rank, np; 
    double start, end;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    start = MPI_Wtime();
    lista x = create_list_of_files_from_dir();
    long total_size = get_total_files_sizes();
    int start_index = 0;
     
    //  EXECUTE ONLY WITH MASTER 
    if (np == 1) {
        compute_word_frequency_master(0, total_size);
    }
    //  EXECUTE ONLY MASTER AND ONE SLAVE  
    else if (np == 2) {
        int part_size = total_size / (np);
        int rest = 0;      
        inizialize_hs_tab_size(); 

        // split the work in half, for master and slave
        if(rank == 0) {

            rest = total_size % (np);

            // the slave will start from the second half till the end
            struct node *section = get_starting_point(part_size, part_size); 
            buffer_split_work *send_work = calloc(1, sizeof(buffer_split_work));

            send_work->part_size = part_size+rest;
            send_work->start = section->seek_line;
            strcpy(send_work->f_name , section->fname);

            master_split_work(send_work, 1);

        }
        if(rank == 0) {
            //  compute the first half of work
            compute_word_frequency_master(0, part_size);
            //  receive the work done by the slave
            master_job(np);

        }
        if(rank != 0) {
            // compute the second half of work
            slave_receive_work();
            // send the frequency list to the master
            slave_job();
        }

    } 
    //  EXECUTE WITH MORE SLAVES
    else {
        int part_size = total_size / (np - 1);
        int rest = 0;      
        long master_start = 0;
        inizialize_hs_tab_size();

        // split the work for the slaves 
        if(rank == 0) {

            rest = total_size % (np -1);

            for (int i = 1; i < np; i++) {     
                
                struct node *section = get_starting_point(start_index + master_start, part_size); 
                buffer_split_work *send_work = calloc(1, sizeof(buffer_split_work));

                send_work->part_size = part_size;
                send_work->start = section->seek_line;
                strcpy(send_work->f_name , section->fname);

                master_split_work(send_work, i);
                master_start += part_size;
            }
        }

        if(rank == 0) {

            //  compute the rest if exist
            if (rest != 0) 
                compute_word_frequency_master(master_start, rest);
            
            //  receive the work done by the slaves
            master_job(np);

        }
        if(rank != 0) {

            // receive the starting point from the master
            slave_receive_work();

            // send the frequency list to the master
            slave_job();
        }
    }
    end = MPI_Wtime();

    build_report_file(end-start, rank);

    free_hash();

    free_list_of_files();

    MPI_Finalize();

    if (rank == 0) { 
        printf("\n\n\n\nFiles analyzed in %f MS check the /logs folder for the reports \n\n\n\n", end-start);
    }
    exit(0);
}

