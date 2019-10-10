#ifndef READER_H
#define READER_H
#include "client.h"
#include <time.h>
#include <poll.h>
#include  <signal.h>

int store_Rarguments(int argc, char** argv,char** common, char** newId, char** myId, int* buff_size,char** mirror,char** log,char** fifo0);
int createMirror(char* mirror, int buff_size, char* fifo0, char* log, int* fail_flag);
void SIGINT_handler(int sig);
void open_blocked(int s);
#endif