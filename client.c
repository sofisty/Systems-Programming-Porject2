#include "client.h"
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN ( 1024 * ( EVENT_SIZE + 16 ) )
#define PERMS   0666
#define T 5
#define NFDS 1

volatile sig_atomic_t flag_sigusr1 = 0;
volatile sig_atomic_t flag_sigusr2 = 0;
volatile sig_atomic_t flag_sigint = 0;

//apothhkeuei ta orismata apo to stdin
int store_arguments(int argc, char** argv, int* id ,char** common, char** input , char** mirror, char** buff, char** log ){
	int i, size ;
	int fl_n=0, fl_c=0, fl_i=0, fl_m=0, fl_b=0, fl_l=0;
	char* wrong_args="Incorrect arguments supplied";

	if(argc!=13){
		fprintf(stderr, "%s\n",wrong_args);
		return -1;
	}
	else{
		i=0;
		while(i<12){
			if( strcmp(argv[i], "-n") == 0){
				fl_n++;
				if(fl_n>1){fprintf(stderr, "%s\n",wrong_args); return -1;} //an dothei 2 fores to idio argument
				
				*id=strtol(argv[i+1], NULL, 10);

				
			}
			else if(strcmp(argv[i], "-c") == 0){
				fl_c++;
				if(fl_c>1){fprintf(stderr, "%s\n",wrong_args); return -1;}
				
				size=strlen(argv[i+1])+1;
				*common=malloc( size* sizeof(char));
				memset( *common, '\0', size*sizeof(char));			
				strcpy(*common, argv[i+1]);
				
			}
			else if(strcmp(argv[i], "-i") == 0){
				fl_i++;
				if(fl_i>1){fprintf(stderr, "%s\n",wrong_args); return -1;}
				
				size=strlen(argv[i+1])+1;
				*input=malloc( size* sizeof(char));
				memset( *input, '\0', size*sizeof(char));			
				strcpy(*input, argv[i+1]);
			

			}
			else if(strcmp(argv[i], "-m") == 0){
				fl_m++;
				if(fl_m>1){fprintf(stderr, "%s\n",wrong_args); return -1;}
				
				size=strlen(argv[i+1])+1;
				*mirror=malloc( size* sizeof(char));
				memset( *mirror, '\0', size*sizeof(char));			
				strcpy(*mirror, argv[i+1]);
			
			}
			else if(strcmp(argv[i], "-b") == 0){
				fl_b++;
				if(fl_b>1){fprintf(stderr, "%s\n",wrong_args); return -1;}

				size=strlen(argv[i+1])+1;
				*buff=malloc( size* sizeof(char));
				memset( *buff, '\0', size*sizeof(char));			
				strcpy(*buff, argv[i+1]);
				
			}
			else if(strcmp(argv[i], "-l") == 0){
				fl_l++;
				if(fl_l>1){fprintf(stderr, "%s\n",wrong_args); return -1;}
				
				size=strlen(argv[i+1])+1;
				*log=malloc( size* sizeof(char));
				memset( *log, '\0', size*sizeof(char));			
				strcpy(*log, argv[i+1]);
			
			}
			i++;
		}

		if( (fl_n+fl_c+fl_i+fl_m+fl_b+fl_l)!=6 ){ //exun dothei ta swsta arguments
			fprintf(stderr, "%s\n",wrong_args );
			return -1;
		}

		return 0;
	}  

}

//elegxei an ta arxeia pou dothhkan apo ta orismata einai apodekta
int vald_dirs(char* input, char* mirror, char* common, char* log){
	struct stat st;
	memset( &st, 0, sizeof(struct stat) );
	FILE* fptr;

	if (stat(input, &st) == -1) {
    	
    	fprintf(stderr, "Error process %d : Input dir does not exist\n",getpid());
    	return -1;
	}
	memset( &st, 0, sizeof(struct stat) );
	if (stat(input, &st) == -1) {
    	
    	fprintf(stderr, "Error process %d : Input dir does not exist\n",getpid());
    	return -1;
	}
	memset( &st, 0, sizeof(struct stat) );
	if(stat(mirror, &st) != -1){
		if(S_ISDIR(st.st_mode)){
			fprintf(stderr, "Error process %d : Mirror dir already exists\n",getpid());
    		return -1;
		}
		else{
			mkdir(mirror, 0777);
		}
	}
	else{
		mkdir(mirror, 0777);
	}
	memset( &st, 0, sizeof(struct stat) );

	if(stat(common,&st)==-1 ){
		mkdir(common,0777);
	}

	memset( &st, 0, sizeof(struct stat) );
	if(stat(log, &st) != -1){
		if(S_ISREG(st.st_mode)){
			fprintf(stderr, "Error process %d : Log file already exists\n",getpid());
    		return -1;
		}
		else{
			if( (fptr=fopen(log,"w+"))==NULL ){
			fprintf(stderr, "Error process %d : Cannot create %s\n",getpid(),log );
			return -1;
			}
		}
	}
	else{
		if( (fptr=fopen(log,"w+"))==NULL ){
			fprintf(stderr, "Error process %d : Cannot create %s\n",getpid(),log );
			return -1;
		}
		fclose(fptr);
	}
	
	
	return 0;

}

//enhmerwnei to common directory me to .id arxeio gia ton client
int update_common(char* common, int id){
	FILE* fp;
	char file_name[64];
	memset( file_name, '\0' , sizeof(file_name));
	sprintf(file_name,"%s/%d.id",common,id);
	if( access( file_name, F_OK ) != -1 ) {
		fprintf(stderr, "Error process : %d .File %s already exists\n",getpid(),file_name );
		return -1;
	}
	fp=fopen(file_name, "w+");
	fprintf(fp, "%d\n", getpid());

	fclose(fp);
	//printf("updated common\n");

	return 0;


}

//prosthetei kainourgio komvo (client) sthn lista me tis plhrofories gia tous clients pou exoun antallaksei arxeia 
checkedId* add_checkedId(checkedId* checked, char* filename, checkedId** new_curr, pid_t pid){
	checkedId* curr=checked;
	
	
	if(checked==NULL){
		checked=malloc(sizeof(checkedId));
		checked->filename=malloc((strlen(filename)+1)*sizeof(char));
		memset(checked->filename, '\0', (strlen(filename)+1)*sizeof(char));
		strcpy(checked->filename,filename); //to onoma tou arxeiou .id pou antistoixei ston kainoutgio client
		checked->tries=0; //prospatheies antallaghs arxeiwn
		checked->reader=0; //pid gia to process pou diaxeirizetai to read tou client
		checked->writer=0; //pid gia to process pou diaxeirizetai to write tou client
		checked->pid=pid; //to process pid tou allou client (to exei diavasei apo to .id sto commmon)
		checked->next=NULL;
		(*new_curr)=checked;
		return checked;
	}
	while(curr->next!=NULL){
		curr=curr->next;
	}
	curr->next=malloc(sizeof(checkedId));
	curr->next->filename=malloc((strlen(filename)+1)*sizeof(char));
	memset(curr->next->filename, '\0', (strlen(filename)+1)*sizeof(char));
	strcpy(curr->next->filename,filename);
	curr->next->tries=0;
	curr->next->reader=0;
	curr->next->writer=0;
	curr->next->pid=pid;
	curr->next->next=NULL;
	(*new_curr)=curr->next;
	return checked;
}

int search_checked(checkedId* checked, char* filename){ //dinetai ena filename kai eksetazei an vrisketai sthn lista
	checkedId* curr=checked;
	if(checked==NULL){return 0;}
	while(curr!=NULL){
		if(strcmp(curr->filename,filename)==0){return 1;}
		curr=curr->next;
	}
	return 0;

}



void print_checked(checkedId* checked){
	checkedId* curr=checked;
	printf("---Print checked --\n");
	while(curr!=NULL){
		printf("\t%s id:%d reader:%d writer%d \n",curr->filename,curr->id,curr->reader,curr->writer );
		curr=curr->next;
	}
}


void free_checked(checkedId* checked){
	checkedId* curr=checked, *temp;
	if(checked==NULL)return;
	while(curr!=NULL){
		temp=curr;
		curr=curr->next;
		free(temp->filename);
		free(temp);
	}
}

//elegxei to common directory gia kainourgio arxeio 
checkedId* check_common(char* common, int id, checkedId* checked, char* input, char* buff, char* mirror, char* log){
	struct dirent **namelist=NULL;
    int n;
    char file_name[64];
    char* token;
	memset( file_name, '\0' , sizeof(file_name));
	sprintf(file_name,"%d.id",id);
	checkedId* curr;
	FILE* fptr;
	char path[1024];
	memset(path,0,1024);
	char line[512];
	memset(line,0,512);
	pid_t pid=0;
    
    n = scandir(common, &namelist, NULL, alphasort);
    if (n < 0)
        perror("scandir");
    else {
    	while (n--) { //gia ola ta arxeia pou vriskontai sto common
    			if(strcmp(namelist[n]->d_name,".")!=0 && 
    				strcmp(namelist[n]->d_name,"..")!=0
    				 && strcmp(namelist[n]->d_name,file_name)!=0 && strstr(namelist[n]->d_name, ".fifo")==NULL
    				 && search_checked(checked, namelist[n]->d_name)==0 ){ //kai einai .id 

    				memset(path,0,1024);
    				sprintf(path,"%s/%s",common,namelist[n]->d_name);

    				//printf("%s path\n",path );
   					if( (fptr = fopen (path, "r+"))==NULL){fprintf(stderr, "cannot open id file %s\n", namelist[n]->d_name );}
   					if( (fgets(line, 512,(FILE*) fptr)) !=NULL){
   						pid=(pid_t)strtol(line,NULL,10);
	   					memset(line,0,512);
	   					fclose(fptr);
   					}

    				if(pid!=0){ //an exei apothhkeutei kai to process pid ksekianei h epikoinwnia
    					printf("! new file %s detected !\n", namelist[n]->d_name );
    					checked=add_checkedId( checked, namelist[n]->d_name, &curr, pid);
    					//print_checked( checked);
    					token=strtok(namelist[n]->d_name, ".id");

    					start_communication( common, id, token, input, buff, mirror,log, &curr);
    				}
    				
    			}
              
               free(namelist[n]);
        }
        if(namelist!=NULL)free(namelist);
    }
    return checked;
}

//sunarthsh pou energopoieitai an exei lavei SIGINT h SIGQUIT 
int exit_program(char* common, char* mirror, char* log,checkedId* checked, int id){
	struct dirent **namelist=NULL;
	 int n;
    char* token;
	char char_id[1024];
	memset(char_id,0,1024);
	sprintf(char_id,"%d",id);
	char temp[1024], command[1024];
	memset(temp,0,1024);
	memset(command,0,1024);
	FILE* logptr;

	printf("Waiting for children to exit\n");
	
	while( wait(NULL) != -1 || errno != ECHILD) { //perimene ola ta paidia na teleiwsoun
   
	}

	char file_name[1024];
	memset( file_name, 0 , sizeof(file_name));
	sprintf(file_name,"%s/%d.id",common,id);

	printf("Deleting .id, common and fifos\n");
	int ret = remove(file_name);

	sprintf(command,"rm -rf %s",mirror);
  	if(system(command)<0){fprintf(stderr, "%d System rm failed\n",getpid()); exit(0);}

   if(ret == 0) {
     // printf("%s deleted successfully\n",file_name);
   } else {
      printf("Error: unable to delete %s\n",file_name);
   }

	n = scandir(common, &namelist, NULL, alphasort); 
    if (n < 0)
        perror("scandir");
    else {
    	while (n--) {
    			if(strcmp(namelist[n]->d_name,".")!=0 && 
    				strcmp(namelist[n]->d_name,"..")!=0
    				 && strstr(namelist[n]->d_name, ".fifo")!=NULL){ //diagrafei ola ta fifo pou exoun to id tou client

    				strcpy(temp,namelist[n]->d_name);
    				token=strtok(temp, "_.");
				    while( token != NULL ) {
					   
				    	
					    if(strcmp(token,char_id)==0){
					       	
					       	memset( file_name, 0 , sizeof(file_name));
					      	//sprintf(file_name,"%s/%s",common,namelist[n]->d_name);
					      	strcat(file_name,common);
					      	strcat(file_name,"/");
					      	strcat(file_name,namelist[n]->d_name);
					      	
					      	ret=unlink(file_name);
					      	if(ret == 0) {
						     //printf("--%s deleted successfully\n",file_name);
							} else {
						      //printf("Error: unable to delete %s\n",file_name);
							}
							break;
					      	
					  	}

				      	token = strtok(NULL,"_.");
				   }			
    				
    			}
              
               free(namelist[n]);
        }
        if(namelist!=NULL)free(namelist);
    }

    //enhmerwnei to log file gia thn eksodo tou client
    if( (logptr = fopen (log, "a"))==NULL ){
		fprintf(stderr, "Exit program: Could not open log file \n" );
		exit(0);
		
	}
	fprintf(logptr, "!e %d\n",id);
	fclose(logptr);

    printf("Notify clients that I'm exiting\n");
    checkedId* curr=checked;
	while(curr!=NULL){
		kill(curr->pid, SIGUSR2);
		curr=curr->next;
	}

	free_checked( checked);
	return 0;

}

//sunarthsh pou kaleitai otan exei lavei SIGUSR1, entopizei apo poio paidi exei erthei to signal kai epistrefei ton komvo ths listas
checkedId* find_SignalId(checkedId* checked, pid_t childId, pid_t* signalId, int* read_write, pid_t* client, checkedId** found){
	checkedId* curr=checked;
	while(curr!=NULL){
		if(curr->reader==childId){
			if(curr->tries>=3){

				printf("This id %d has already failed at least 3 times\n",curr->id);
				(*signalId)= 0;
				(*found)=NULL;
				return checked;
			}
			curr->tries+=1;
			*signalId=curr->writer;
			*read_write=1;
			*client=curr->pid;
			*found=curr;
			return checked;
		}
		else if (curr->writer==childId){
			if(curr->tries>=3){

				printf("This id %d has already failed at least 3 times\n",curr->id);
				(*signalId)= 0;
				(*found)=NULL;
				return checked;
			}
			curr->tries+=1;
			*signalId=curr->reader;
			*client=curr->pid;
			*read_write=0;
			*found=curr;
			return checked;
		}
		else if(curr->pid==childId){
			if(curr->tries>=3){

				printf("This id %d has already failed at least 3 times\n",curr->id);
				(*signalId)= 0;
				(*found)=NULL;
				return checked;
			}
			curr->tries+=1;
			*signalId=0;
			*read_write=2;
			*found=curr;
			return checked;
		}
		curr=curr->next;
	}
	*signalId=0;
	(*found)=NULL;
	return checked;
}

//vriskei ton komvo ths checked listas pou antistoixei sto pid tou patera pou dinetai san orisma 
int find_children(checkedId* checked, pid_t sender, checkedId** found){
	checkedId* curr=checked;
	while(curr!=NULL){
		if(curr->pid==sender){
			
			*found=curr;
			
			return 0;
		}
		curr=curr->next;
	}

	return -1;
}




//sunarthsh pou kaleitai otan exei lavei SIGUSR1: kapoio paidi apetuxe
checkedId* handle_SIGUSR1(checkedId* checked, pid_t sender, char* common,int  id, char* input, char* buff, char* mirror, char* log){
	pid_t signalId=0;
	int status;
	pid_t client;
	int read_write=-1;
	int sig_fail=0, send_fail=0;
	checkedId* found=NULL;
	//printf("signal handler 1\n");

	checked=find_SignalId( checked, sender, &signalId, &read_write, &client, &found );//vres ton apostolea
	if(found!=NULL && sender!=getpid()){
		

		if(kill(signalId, SIGUSR1)<0){ //enhmerwse to allo paidi gia na termatisei
			fprintf(stderr, "Could not send signal to %d\n",signalId );
			sig_fail=1;
		}
		else{
			printf("Esteila signal sto allo paidi %d\n",signalId );
		}
		

		if(kill(sender,0)<0){ //elegxei an uparxei akoma o sender
			send_fail=1;
		}

		//printf("anamenw gia to allo paidi\n");
		if(sig_fail!=1){
			waitpid(signalId,&status, 0);
	    	if(WIFEXITED(status)>0){printf(" %d exited normally\n",signalId);}
	    	else if(WCOREDUMP(status)) {printf("core dump %d\n",signalId  );}
	    	else if(WIFSIGNALED(status)){ printf("%d terminated by signal %d\n",signalId ,WTERMSIG(status) ); }
		    
		}
		
		//printf("anamenw gia ton sender\n");
		if(send_fail!=1){
			waitpid(sender, &status, 0);
	    	if(WIFEXITED(status)>0){printf("%d exited normally\n",sender);}
	    	else if(WCOREDUMP(status)) {printf("core dump %d\n",sender  );}
	    	else if(WIFSIGNALED(status)){ printf("%d terminated by signal %d\n",sender, WTERMSIG(status)); }
			
		}

		char newId[512];
		memset(newId, 0, 512);
		sprintf(newId,"%d",found->id);

		flag_sigusr1=0;
		printf("_______________Retry_______________ \n");
		//ksekinaei ksana thn epikoinwnia
		start_communication( common, id,  newId, input,  buff,  mirror,  log, &found);
	
		
    }
    flag_sigusr1=0;
    //printf("exiting from handler\n");
    return checked;
}


//kaleitai otan dexetai SIGUSR2 : kapoios client exei fugei apo to susthma
checkedId* handle_SIGUSR2(pid_t sender, char* mirror,checkedId* checked){
	
	pid_t reader=0, writer=0;
	
	checkedId* found;
	pid_t childpid;
	char temp_reader[1024];
	memset(temp_reader,0,1024);
	char temp_writer[1024];
	memset(temp_writer,0,1024);
	char temp_id[1024];
	memset(temp_id,0,1024);
	int status;
	int fail_w=0, fail_r=0;

	if(sender!=0){
		//vres ton komvo tou apostolea kai perimene ta paidia pou epikoinwnoun me auton na teleiwsoun
		if(find_children( checked,  sender, &found )!=-1){
			printf("----HANDLE SIGUR2----\n");
			
			reader=found->reader;
			writer=found->writer;
					
			//printf("%d reader\n",reader );
				if(kill(reader, 0)<0){
					//printf("failed sending signal to reader\n");
					fail_r=1;

				}
			
				//printf("%d writer\n", writer);
				if(kill(writer, 0)<0){
					//printf("failed sending signal to writer\n");
					fail_w=1;
				}
				
				if(fail_r!=1){
					waitpid(reader, &status, 0);
			    	if(WIFEXITED(status)>0){printf("%d exited normally\n",reader);}
			    	else if(WCOREDUMP(status)) {printf("core dump %d\n",reader );}
			    	else if(WIFSIGNALED(status)){ printf("%d terminated by signal %d\n",reader, WTERMSIG(status)); }

				}
		    	
		    	if(fail_w!=1){
		    		waitpid(writer, &status,0);
			    	if(WIFEXITED(status)>0){printf("%d exited normally\n",writer);}
			    	else if(WCOREDUMP(status)) {printf("core dump %d\n",writer );}
			    	else if(WIFSIGNALED(status)){ printf("%d terminated by signal %d\n",writer, WTERMSIG(status)); }

			    	
		    	}
		    

	    	sprintf(temp_id,"%d",found->id);

	    	//fork paidi, pou einai upeuthuno gia thn diagrafh tou fakelou sto mirror, pou sxetizetai me ton client sender
	    	childpid=fork();
			if(childpid==0){
				signal(SIGINT,SIG_IGN);
				signal(SIGQUIT,SIG_IGN);
				signal(SIGUSR1,SIG_IGN);			

				if(execl( "./exit.o", "./exit",(char*)mirror,(char*)temp_id ,(char*) NULL)==-1){
					printf("%d\n",errno );
				}
				exit(0);
				

			}
			else{
				found->writer=0;
			    found->reader=0;
			}			
   
		}
		else{
	
    		printf("Could not find checked_list's node for sender\n");
    	}
	}
	else{
		printf("Sender is 0 \n");
	}
	
    flag_sigusr2=0;
    return checked;
}

//signal handler
void sigusr(int signo, siginfo_t *si, void *data) {
  (void)signo;
  (void)data;
 // printf("Signal %d from pid %lu\n", (int)si->si_signo,
 //(unsigned long)si->si_pid);
  if(signo==SIGUSR1){printf("Parent process received SIGUSR1 %d \n",si->si_pid);	flag_sigusr1=si->si_pid;}
  else if (signo==SIGUSR2){printf("Sigusr2 from %d\n",si->si_pid ); flag_sigusr2=si->si_pid;}
  else if(signo==SIGINT || signo==SIGQUIT ){printf("Received SIGINT/SIGQUIT\n" ); flag_sigint=1;}
  
  
}

//periodikos elegxos tou common kai twn shmatwn kata T
int monitor_folder(char* common, int id, char* input, char* buff, char* mirror, char* log){

  checkedId* checked=NULL;
  int i; 
  i=0;

  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = sigusr;
  if (sigaction(SIGUSR1, &sa, 0) == -1) {
    fprintf(stderr, "%s: %s\n", "sigaction", strerror(errno));
  }
     if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("sigaction");
        return 0;
    }
     if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 0;
    }
     if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        perror("sigaction");
        return 0;
    }




 	 while(1){
  		if(i==INT_MAX){i=0;}
  		if((i%T)==0 || i==0){
  		
 			checked=check_common( common, id, checked, input, buff, mirror,log);
 		
		}
		if(flag_sigusr1!=0 ){
 			sigprocmask(SIG_SETMASK, &(sa.sa_mask), NULL);
 			checked= handle_SIGUSR1( checked, flag_sigusr1, common ,id,  input,  buff,  mirror, log);
 			sigprocmask(SIG_UNBLOCK, &(sa.sa_mask), NULL); //Unblock sigurs
 			

 		}
 		else if(flag_sigint==1){
 			exit_program(common, mirror,log, checked, id);
 			return 0;
 		}
 		else if(flag_sigusr2!=0){
 			checked=handle_SIGUSR2(flag_sigusr2, mirror, checked);
 			
 		}
	 	i++;
 	}
  return 0;


}


//sunarthsh dhmiourgias paidiwn gia thn epikoinwnia
int start_communication(char* common, int myId, char* newId, char* input, char* buff, char* mirror, char* log, checkedId** curr){
	pid_t childpid[2];

	struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = sigusr;
  if (sigaction(SIGUSR1, &sa, 0) == -1) {
    fprintf(stderr, "%s: %s\n", "sigaction", strerror(errno));
  }


	if(*curr==NULL){fprintf(stderr, "Checked list node Null inside start_communication\n" ); exit(0);}
	(*curr)->id= strtol(newId,NULL,10);
	
	//sigprocmask(SIG_SETMASK, &(sa.sa_mask), NULL); //Unblock sigurs

	childpid[1]=fork();
	if(childpid[1]==0){
		//sigprocmask(SIG_UNBLOCK, &(sa.sa_mask), NULL); //Unblock sigurs*/
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		char temp[16];
		memset(temp,'\0',sizeof(temp));
		sprintf(temp,"%d",myId);
		//printf("writer %d from %d\n",getpid(),getppid() );
		if( flag_sigusr1==getppid() ){ printf("Writer exits before it starts\n");  return 0;} //periptwsh pou exei stalei signal apotuxias paidiou apo ton pater
																							// prin thn klhsh ths exec
		if(execl("./writer.o", "./writer",(char*)common,(char*)newId,(char*)temp,(char*)input,(char*)buff,(char*)log,(char*) NULL)==-1){
				printf("%d\n",errno );
		}
		exit(0);
	}
	else{
		childpid[0]=fork();
		if(childpid[0]==0){
			//sigprocmask(SIG_UNBLOCK, &(sa.sa_mask), NULL); //Unblock sigurs*/
			//printf("reader %d from %d\n",getpid(),getppid() );
			signal(SIGINT, SIG_IGN);
			signal(SIGQUIT, SIG_IGN);
			char temp[16];
			memset(temp,'\0',sizeof(temp));
			sprintf(temp,"%d",myId);
			if( flag_sigusr1==getppid() ){ printf("Writer exits before it starts\n");  return 0;}
			if(execl( "./reader.o", "./reader",(char*)common,(char*)newId,(char*)temp ,(char*)buff,(char*)mirror,(char*)log ,(char*) NULL)==-1){
				printf("%d\n",errno );
			}
			exit(0);
		}
		else{	
			(*curr)->writer=childpid[1];
			(*curr)->reader=childpid[0]; 
			return 0;
			//sigprocmask(SIG_UNBLOCK, &(sa.sa_mask), NULL); //Unblock sigurs*/
		}
	}

	
    return 0;
}