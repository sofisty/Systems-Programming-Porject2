#ifndef WRITER_H
#define WRITER_H

#include "client.h"
#include <time.h>
 #include <stdint.h> 


typedef struct dir_list{
	char* dirname;
	struct dir_list* next;
	
}dir_list;

static void handler(int sig);
int is_emptyDir(char* dirname);
char* remove_substr(char *str, char *sub);

int store_Warguments(int argc, char** argv, char** common, int* newId, int* myId, char** input,int* buff_size, char** log,char** fifo1);
dir_list* add_dir(dir_list* dirL, char* dirname);
void print_dirL(dir_list* dirL);
void free_dirList(dir_list* dirL);
int scan_inputD(char* input,char* log, char* fifo1, int buff_size);

#endif