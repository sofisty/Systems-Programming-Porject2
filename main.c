#include "client.h"

int main ( int argc, char **argv ) {
	char* common=NULL, *input=NULL, *mirror=NULL ,*log=NULL, *buff=NULL;
	int id;
	FILE* logptr;

	store_arguments( argc, argv,  &id, &common, &input , &mirror,  &buff,  &log );
	if(strtol(buff,NULL,10)<=32){fprintf(stderr, "%d Invalid buffer size\n",getpid() ); return-1;}
	printf("%d %s %s %s %s %s\n",id, common,input,mirror,buff,log );
	
	if(vald_dirs(input, mirror, common, log)==-1){return 1;}
	
	if( update_common( common,id) ==-1){return 1;}
	
	if( (logptr = fopen (log, "a"))==NULL ){
		fprintf(stderr, "Main: %d Failed openning log file %s\n",getpid(),log );
		exit(0);		
	}
	fprintf(logptr, "!id %d\n",id );
	fclose(logptr);

	monitor_folder( common, id, input, buff, mirror,log);
	
	if(buff!=NULL){free(buff);}
	if(common!=NULL){free(common);}
	if(mirror!=NULL){free(mirror);}
	if(log!=NULL){free(log);}
	if(input!=NULL){free(input);}
	

	return 0;
}