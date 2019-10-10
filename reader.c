#include "reader.h"
#define PERMS   0666
#define MAXBUFF 5
volatile sig_atomic_t termination = 0;
volatile sig_atomic_t timeout = 0;

int store_Rarguments(int argc, char** argv, char** common, char** newId, char** myId, int* buff_size,
char** mirror,char** log, char** fifo0){
	
	char* wrong_args="Incorrect arguments supplied";

	if(argc!=7){
		fprintf(stderr, "%s\n",wrong_args);
		return -1;
	}
	else{
		*common=argv[1];
		*newId=argv[2];
		*myId=argv[3];
		*buff_size=strtol(argv[4],NULL,10);
		*mirror=argv[5];
		*log=argv[6];
		*fifo0=malloc( (strlen(argv[2]) + strlen(argv[3]) + strlen(argv[1]) +12 ) *sizeof(char));
		memset(*fifo0, '\0',(strlen(argv[2]) + strlen(argv[3]) + strlen(argv[1]) +12 ) *sizeof(char));
		strcat(*fifo0,*common);
		strcat(*fifo0,"/");
		strcat(*fifo0, argv[2]);
		strcat(*fifo0,"_to_");
		strcat(*fifo0,argv[3]);
		strcat(*fifo0,".fifo");
	}
		
	return 0;
		
}

//handler gia thn apotuxia tou reader
void SIGINT_handler(int sig){
	
	 	if(sig==SIGUSR1){
	 		termination=1;
	 		printf("Reader received SIGUSR1\n");
	 		termination=1;
	 	}
	 	else if(sig==SIGUSR2){
	 		printf("Reader received SIGUR2\n");
	 		
	 		termination=1;
	 	}
		else if(sig== SIGALRM){
			printf("Timeout\n");
			termination=1;
			timeout=1;
		}
   
}


//sunarthsh ulopoihshs prwtokollou epikoinwnias gia ton reader
int createMirror(char* mirror, int buff_size, char* fifo0, char* log, int* fail_flag){
	int readfd;
 	char* buff=malloc(buff_size* sizeof(char));
	if(buff==NULL){fprintf(stderr, "%d Mallon failed\n",getpid() ); return -1;}
	memset(buff,0,buff_size);
	int n,  total_read, read_size;
	unsigned short int name_length=0;
	unsigned int file_length=0;
	char* file_name=NULL;
	char temp_name[3000];
	FILE *fptr=NULL, *logptr=NULL;
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	

	if(	(readfd = open(fifo0, O_RDONLY| O_NONBLOCK)) <0){ //epilogh nonblock wste na mhn kollaei o reader kai na mporei na elegxei to shma aporuxias
	   	 	fprintf(stderr, "%d Cannot open read fifo\n",getpid() );
	   	 	free(buff);
	   	 	return -1;
	}

 	read_size=10;
   if(termination==1){ free(buff);return 0;} //volatile flag gia ton signal handler tou shmatos
   											//an termination==1 exei dextei SIGUSR1 apo ton patera

	
   if(termination==1){ close(readfd) ; free(buff); return 0;}
    while(1){ //loupa mexri na diavasei epituxia h apotuxia

	
    	if(termination==1){printf("--1\n");  break;}
		memset(buff,0,buff_size);
		read_size=5; //mexri 5 pshfia o short unsigned int 
		
		if(termination==1){ printf("--2\n");break;}
		signal(SIGALRM, SIGINT_handler); //timeout an den exei diavasei kati mesa se 30 deuterolepta
   		alarm(30);
    	while((n = read(readfd, buff, read_size)) <= 0){if(termination ==1)break;}
    	signal(SIGALRM, SIG_IGN);
    	
    	if(strcmp(buff,"00")==0){ break;} //success
    	if(strcmp(buff,"-f")==0){ termination=1; *fail_flag=1;break;} //failed writer, return -1, signal ppid   
    	name_length=(unsigned short int)strtol(buff,NULL,10); //to megethos tou onomatos tou arxeiou
    	//printf("> name length: %d\n",name_length );
    	
    	file_name=malloc((name_length+1)*sizeof(char)); //onoma arxeiou
    	memset(file_name,0,(name_length+1));
    	total_read=0;

    	if(name_length>buff_size){ 
    		read_size=buff_size-1;
	    }
	    else{ read_size=name_length;}
    	memset(buff,0,buff_size);

	    if(termination==1){ break;}
    	while(total_read<name_length){ //diavase to bytes oso to name_length
    		
    		if(name_length-total_read<read_size){
    			read_size=name_length-total_read;
    		}
    		

    		if(termination==1){ break;}
    		signal(SIGALRM, SIGINT_handler);
   			alarm(30);
    		while((n = read(readfd, buff, read_size)) <= 0){if(termination ==1)break;}
    		signal(SIGALRM, SIG_IGN);
  			
  			if(strcmp(buff,"-f")==0){ termination=1; *fail_flag=1;break;}     
			
			strcat(file_name,buff);
			total_read+=n;
			memset(buff,0,buff_size);

    	}
	    	
    	//printf("> file_name: %s\n",file_name );
    	if(termination==1){ break;}
    	
    	memset(buff,0,buff_size);
    	read_size=10;
   		
   		signal(SIGALRM, SIGINT_handler);
   		alarm(30);
    	while((n = read(readfd, buff, read_size)) <= 0){if(termination ==1){break;}}
    	signal(SIGALRM, SIG_IGN);
    	if(strcmp(buff,"-f")==0){ termination=1; *fail_flag=1;break;}     	
    	file_length=(unsigned int)strtol(buff,NULL,10); //to megethos tou arxeiou
 			
    	memset(buff,0,buff_size);
    	if(file_length>buff_size){read_size=buff_size-1;}
		else{read_size=file_length;}

    	if(file_length==0){
    		if(termination==1){ break;}
    		signal(SIGALRM, SIGINT_handler);
   			alarm(30);
    		while((n = read(readfd, buff,strlen("-d"))) <= 0){if(termination ==1)break;} // an diavasei -d shmainei oti einai directory
    		signal(SIGALRM, SIG_IGN);
	    	if(strcmp(buff,"-f")==0){ termination=1; *fail_flag=1;break;}     	
	    	
    		if(strcmp(buff,"-d")==0){//directory
    			//printf("\t directory\n");
    			sprintf(temp_name,"%s/%s",mirror,file_name);
    			if(termination==1){ break;}
    			if( mkdir(temp_name,0777)<0 ){
    				fprintf(stderr, "%d Failed creating dir %s\n",getpid(),temp_name );
    				free(buff);
    				free(file_name);
    				close(readfd);
    				fclose(logptr);
    				return -1;
    			}

			}
			else if(strcmp(buff,"-e")==0){//empty file
				if(termination==1){break;}
				if( (logptr = fopen (log, "a"))==NULL ){
					fprintf(stderr, "%d Failed openning or creating file %s\n",getpid(),log );
    				free(buff);
    				free(file_name);
    				close(readfd);
    				
    				return -1;
				}
				//printf("\t empty file\n");
				memset(temp_name,0,3000);
				sprintf(temp_name,"%s/%s",mirror,file_name);
				if(termination==1){ break;}
				if( (fptr = fopen (temp_name, "w+"))==NULL ){
					fprintf(stderr, "%d Failed creating file %s\n",getpid(),temp_name );
    				free(buff);
    				free(file_name);
    				close(readfd);
    				fclose(logptr);
    				return -1;
				}
				fprintf(logptr, "!r %d\n",0);
				fclose(fptr);
				fclose(logptr);
			}
			else{
				free(buff);
    			free(file_name);
    			close(readfd);
				fprintf(stderr, "%d Something went wrong\n",getpid());
				return -1;
			}
			
    	}
		else{//file
			//printf("einai file \n");
			memset(temp_name,0,3000);
			sprintf(temp_name,"%s/%s",mirror,file_name);
			if(termination==1){ break;}
			if( (fptr = fopen (temp_name, "w+"))==NULL ){
				fprintf(stderr, "%d Failed creating file %s\n",getpid(),temp_name );
				free(buff);
    			free(file_name);
    			close(readfd);
				return -1;
			}
			if( (logptr = fopen (log, "a"))==NULL ){
				fprintf(stderr, "%d Failed openning or creating file %s\n",getpid(),log );
				free(buff);
    			free(file_name);
    			close(readfd);
				return -1;
			}
				
			total_read=0;
			memset(buff,0,buff_size);
			//printf("> data\n");
			if(termination==1){ break;}
	    	while( total_read<file_length){ //diavase akrivws tosa bytes osa to megethos tou arxeiou 

	    		if(file_length-total_read<read_size){
	    			read_size=file_length-total_read;
	    			//printf("~~~~~~~~NEW %d / %d - %d < %d\n",read_size,file_length,total_read,temps );
	    		}
	    		if(termination==1){ break;}
	    		signal(SIGALRM, SIGINT_handler);
   				alarm(30);
				while((n = read(readfd, buff, read_size)) <= 0 ){if(termination ==1)break;}
				signal(SIGALRM, SIG_IGN);
				//printf("WRITE %s\n",buff );	   
	    		
				if(strcmp(buff,"-f")==0){ termination=1; *fail_flag=1;break;}     
				
				fprintf(fptr, "%s",buff );
	    		total_read+=n;
	    		//printf(" %d: %s\n",total_read,buff );
	    		memset(buff,0,buff_size);
								

	    	}
	    	
	    	//printf("total read %d file_length %d\n",total_read,file_length );
	    	fprintf(logptr, "!r %d\n",total_read );
	    	fclose(fptr);
	    	fclose(logptr);
	    	if(termination==1){ break;}
	    
		}
		//printf("---------------------------------------------\n");
    	
	
    	free(file_name);
    	file_name=NULL;
    
	
	}	
	

    close(readfd); 
    free(buff);
    if(file_name!=NULL)free(file_name);
    return 0;
       
}


int main ( int argc, char **argv ) {
	printf("READER.C running. %d\n",getpid());
	
	char* common, *mirror, *log;
	char* myId, *newId;
	
	char* fifo0=NULL;
	int buff_size;
	char new_path[1024];
	memset(new_path,0,1024);
	char command[1024];
	memset(new_path,0,1024);
	int ret=0, fail_flag=0;

	struct sigaction sh;

	sh.sa_handler = SIGINT_handler;
	sigemptyset (&sh.sa_mask);
	sh.sa_flags = 0;
	sigaction (SIGUSR1, &sh, NULL);
	sigaction (SIGPIPE, &sh, NULL);
	sigaction (SIGUSR2, &sh, NULL);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	if(store_Rarguments(argc,  argv, &common, &newId, &myId, &buff_size, &mirror,&log, &fifo0)==-1){		
		kill(getppid(),SIGUSR1);
		if(fifo0!=NULL){free(fifo0);}
		return 1;
	}


	if ( (mkfifo(fifo0, PERMS) < 0) ) {
		if(errno==EEXIST){
			//printf("%d open %s\n",getpid(),fifo0 );
		}
		else {perror("can't create fifo"); kill(getppid(),SIGUSR1); if(fifo0!=NULL){free(fifo0);} return -1;}
	}
	//else{//printf("%d created %s\n",getpid(),fifo0 );}
											
	strcat(new_path,mirror);
	strcat(new_path,"/");
	strcat(new_path,newId);

	if( mkdir(new_path,0777)){
    	if(errno==EEXIST){fprintf(stderr, "%d Mirror for %s id already exists\n",getpid(),newId );}
    	else {printf("error while trying to create '%s'\n",new_path );}
    	kill(getppid(),SIGUSR1);
		if(fifo0!=NULL){free(fifo0);}
    	
    	return 1;
	}
	
	if(termination!=1){
		
		 ret=createMirror( new_path, buff_size,  fifo0,  log , &fail_flag);  
	}
  	if(ret==-1){ //an apetuxe o idios stelnei shma ston patera
  		fprintf(stderr, "%d failed reading pipe\n",getpid());
  		sprintf(command,"rm -rf %s",new_path);
  		if(system(command)<0){fprintf(stderr, "%d System rm failed\n",getpid()); exit(0); }
  		fprintf(stderr, "%d Reader did not succeed. Sending SIGUSR1 to father.\n",getpid() );
  		kill(getppid(),SIGUSR1);
  		
  	}
  	if(termination==1){
  		sprintf(command,"rm -rf %s",new_path);
  		if(system(command)<0){fprintf(stderr, "%d System rm failed\n",getpid()); exit(0);}
  		unlink(fifo0);
  		fprintf(stderr, "\tReader exits due to signal\n" );
  		if(timeout==1 )kill(getppid(),SIGUSR1); //an exei sumvei timeout enhmerwnei ton patera
  		if(fail_flag==1){kill(getppid(),SIGUSR1); fprintf(stderr, "\tGot -f from pipe\n" );} //an exei lavei apo ton writer -f enhmerwnei ton patera
  	}
    
    printf(">>> Reader returns now\n");
  	if(fifo0!=NULL){free(fifo0);}
  
	return 0;
}