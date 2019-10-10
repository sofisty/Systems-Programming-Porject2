#include "client.h"
//svhnei ton fakelo me to onoma id apo to mirror
int main(int argc, char **argv ){
	char* mirror;
	int id ;
	char dir_name[1024];
	memset(dir_name,0,1024);
	char command[1024];
	memset(command,0,1024);

	mirror=argv[1];
	id=strtol(argv[2],NULL,10);

	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
	signal(SIGUSR1,SIG_IGN);

	printf("Deleting mirror for id %d \n",id);
	sprintf(command,"rm -rf %s/%d",mirror,id);
	if(system(command)<0){fprintf(stderr, "%d System rm failed\n",getpid()); exit(0);}

	return 0;
}