#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "./schedule.h"
#include "./file_utils.h"
#include "./frequency_list.h"
#include "./mpi_utils.h"


node *get_starting_point(long start, long part) {
    node *head = getHead(); 
    node *curr = head;

    long sectionStart = start;
    long sectionSize = head->size;

    //  if i can analyze the first file set the "seek" point
    if(sectionSize > start) {
        head->seek_line = start;
    }
    else {  //  otherwise scroll the list till you doun't reach your starting point
        sectionSize = 0;    
        while (curr != NULL && sectionSize <= sectionStart) {  

            sectionSize += curr->size;  
            // if another process analyzed this, remove it from the section and update the head = starting poing
            if(sectionSize <= sectionStart) {   
                start -= curr->size;       
                head = curr->next;     
                head->seek_line = start;
            }
            curr = curr->next;     
        }
    }
    return head; // process starting point 
}


long count_words(FILE *F, long words_counted, long part) {
    int ch = 0, len = 0;  //  'len' is for keep track of the word length 
    char word[WORDLENGHT]; 
    // clean the word buffer 
    memset(word, '\0', WORDLENGHT);

    // do this unltil you read all the chars in the file && you didn't finish your part of work || u're reading a word
    while (( (ch = fgetc(F)) != EOF) && ((part >= words_counted) || (len > 0) )) {

        words_counted++;

        ch = tolower(ch);  
        // check the ascii for the letters a-z
        if (ch > 96 && ch < 123 ) {
            word[len] = ch;
            len++;
        }
        else { 
            // min size for count a word 
            if (len > 3) 
                insert_update(word, 1);
             
            // reset the string
            memset(word, '\0', WORDLENGHT);
            len = 0;
        }
        
    }
    // for the last word of a file
    if ((ch == EOF) && (len > 3)) {
        insert_update(word, 1);
        words_counted++;    
    }
    return words_counted;
    
}


long prepare_to_count(FILE *F, long word_counted, long start, long part) {
    int ch = 0; 
    //  if another process opened this file, go to your starting line
     if (start != 0) {
        while(1) {
            ch = fgetc(F);
            word_counted++;
            if (ch == EOF) 
                return 0;
            else if (ch == ' ' || ch == '\n') {
                break;
            }
        }
    }
    return count_words(F, word_counted, part);
}

void compute_word_frequency_slave(buffer_split_work *work) {
    FILE *F;
    node *head = getHead();     

    // scroll the list till u don't reach your starting point 
    while(strcmp(head->fname, work->f_name)) {
        head = head->next;
    }

    long word_counted = 0;
    int firstFile = 1;
    //  count the words untill u don't finish your part of work
    while (head != NULL && word_counted < work->part_size) {

        char address[100] = FILEPATH PATHMODE;
        strcat(address, head->fname);  

        F = fopen(address, "r");
        // set the seek to the starting point, for the first tile to read
        if (firstFile) {
            fseek(F, work->start, SEEK_SET);
            firstFile = 0;
        }
        else // otherwise start from the beginning
            work->start = 0;
        word_counted = prepare_to_count(F, word_counted, work->start, work->part_size);

        fclose(F);
        head = head->next;
    }
}   

void compute_word_frequency_master(long start, long part) {
    FILE *F;
    long word_counted = 0;

    struct node *section = get_starting_point(start, part);

    //int firstFile = 1;
    //  count the words untill u don't finish your part of work
    while (section != NULL && word_counted < part) {
        char address[100] = FILEPATH PATHMODE;
        strcat(address, section->fname);  
        F = fopen(address, "r");

        //printf("Master Fname: %s\n",section->fname );
    
        fseek(F, start, SEEK_SET);

        
        word_counted = prepare_to_count(F, word_counted, section->seek_line, part);
        fclose(F);
        section = section->next;
    }

}   

// // =========  DEBUG ONLY ==========  
// int main() {
//         lista x = create_list_of_files_from_dir();
//         printlist(x);
//         // long l = get_total_files_size();
//         //lista n = get_section(16, 3);

//         compute_word_frequency(0, 44);
// }  