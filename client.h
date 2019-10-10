#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <errno.h>
#include<sys/wait.h>
#include <dirent.h>
#include <signal.h>
#include <limits.h>
#include <sys/signalfd.h>
#include <assert.h>
#include <poll.h>

#include<signal.h> 

typedef struct checkedId{
	char* filename;
	int id;
	pid_t pid;
	pid_t reader;
	pid_t writer;
	int tries;
	struct checkedId* next;
	
}checkedId;

void sigusr(int signo, siginfo_t *si, void *data);

int exit_program(char* common, char* mirror,char* log, checkedId* checked, int id);

checkedId* handle_SIGUSR2(pid_t flag_sigusr2, char* mirror,checkedId* checked);
checkedId* handle_SIGUSR1(checkedId* checked, pid_t sender, char* common,int  id, char* input, char* buff, char* mirror, char* log);

checkedId* add_checkedId(checkedId* checked, char* filename, checkedId** curr, pid_t pid);
int search_checked(checkedId* checked, char* filename);
void print_checked(checkedId* checked);
void free_checked(checkedId* checked);
checkedId* find_SignalId(checkedId* checked, pid_t childId, pid_t* signalId, int* read_write, pid_t* client, checkedId** found);
int find_children(checkedId* checked, pid_t sender, checkedId** found);

int store_arguments(int argc, char** argv, int* id ,char** common, char** input , char** mirror, char** buff, char** log );
int vald_dirs(char* input, char* mirror, char* common, char* log);
int update_common(char* common, int id);
int monitor_folder(char* common, int id, char* input, char* buff, char* mirror, char* log);
int start_communication(char* common, int myId, char* newId, char* input, char* buff, char* mirror,char* log, checkedId** curr);
checkedId* check_common(char* common, int id,checkedId* checked, char* input, char* buff, char* mirror, char* log);
int filter_file(const struct dirent *file);

#endif 
