#include "writer.h"
#define PERMS   0666

volatile sig_atomic_t termination = 0;
volatile sig_atomic_t broken_pipe = 0;

//lista pou apothhkeuei osa directories vriskontai sto input
dir_list* add_dir(dir_list* dirL, char* dirname){
	dir_list* curr=dirL;
	if(dirL==NULL){
		dirL=malloc(sizeof(dir_list));
		dirL->dirname=malloc((strlen(dirname)+1)*sizeof(char));
		memset(dirL->dirname, '\0', (strlen(dirname)+1)*sizeof(char));
		strcpy(dirL->dirname,dirname);
		dirL->next=NULL;
		return dirL;
	}
	while(curr->next!=NULL){
		curr=curr->next;
	}
	curr->next=malloc(sizeof(dir_list));
	curr->next->dirname=malloc((strlen(dirname)+1)*sizeof(char));
	memset(curr->next->dirname, '\0', (strlen(dirname)+1)*sizeof(char));
	strcpy(curr->next->dirname,dirname);
	curr->next->next=NULL;
	return dirL;
}

void print_dirL(dir_list* dirL){
	dir_list* curr=dirL;
	if(dirL==NULL){printf("DirL is empty\n");}
	while(curr!=NULL){
		printf(">%s\n",curr->dirname );
		curr=curr->next;
	}
}

void free_dirList(dir_list* dirL){
	dir_list* curr=dirL, *temp;
	if(dirL==NULL)return;
	while(curr!=NULL){
		temp=curr;
		curr=curr->next;
		free(temp->dirname);
		free(temp);
	}
}

int store_Warguments(int argc, char** argv, char** common, int* newId, int* myId, char** input,int* buff_size, char** log,char** fifo1){
	char* wrong_args="Incorrect arguments supplied";

	if(argc!=7){
		fprintf(stderr, "%s\n",wrong_args);
		return -1;
	}
	else{

		*common=argv[1];
		*newId=strtol(argv[2], NULL,10);
		*myId=strtol(argv[3],NULL,10);
		*input=argv[4];
		*buff_size=strtol(argv[5],NULL,10);
		*log=argv[6];

		*fifo1=malloc( (strlen(argv[2]) + strlen(argv[3]) + strlen(argv[1]) +12 ) *sizeof(char));
		memset(*fifo1, '\0',(strlen(argv[2]) + strlen(argv[3]) + strlen(argv[1]) +12 ) *sizeof(char));
		//problem with sprintf
		strcat(*fifo1,*common);
		strcat(*fifo1,"/");
		strcat(*fifo1, argv[3]);
		strcat(*fifo1,"_to_");
		strcat(*fifo1,argv[2]);
		strcat(*fifo1,".fifo");
		
	}
		
	return 0;
}


char* remove_substr(char *str, char *sub){
	
    size_t len = strlen(sub);
    if(len > 0) {
        char *p = str;
        while((p = strstr(p, sub)) != NULL) {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
      
    return str;
}

//elegxei an ena directory einai adeio
int is_emptyDir(char* dirname){
        int n=0;
        struct dirent* d=NULL;
        DIR* dir = opendir(dirname);
        if (dir == NULL) return 0;
       	while((d = readdir(dir))!=NULL) {
       		if(n>2)break;
       		n++;
       	}

       	if(n>2){ //not empty
       		closedir(dir);
        	return 0;	
       	}
        else{ //empty
        	closedir(dir);
        	return 1;	
        }
        
        
}



//sunarthsh ulopoihsh prwtokollou epikoinwnias gia ton writer
int scan_inputD(char* input, char* log,char* fifo1, int buff_size){
	int writefd;
	char* buff=malloc(buff_size*sizeof(char));
	if(buff==NULL){
		fprintf(stderr, "%d: Malloc buff failed\n",getpid() );
		 return -1;
	}

	
    struct sigaction sa;
    //ta xeirizetai o pateras 
   signal(SIGINT, SIG_IGN);
   signal(SIGQUIT, SIG_IGN);

	dir_list* dirL=NULL,*curr;
	struct dirent *de;
	DIR *inp_dir ;
	FILE *fp, *logptr;
	char path[3000], nest_path[3000], file[3000];
	int count_dirs=0, count_files=0;
	unsigned short int name_length;
	unsigned int file_length;
	int i, j, write_size; 
	char c;
	char* subs;
	subs=malloc((strlen(input)+2)*sizeof(char));
	memset(subs,0,strlen(input)+2);
	strcat(subs,input);
	strcat(subs,"/");
   	
   	writefd=open(fifo1, O_WRONLY| O_NONBLOCK ); //wste na lavei to signal an den exei anoiksei akoma kapoios reader
 	while(errno==ENXIO){
 		close(writefd);
 		if(termination==1){free(buff); return 0;}
 		writefd=open(fifo1, O_WRONLY | O_NONBLOCK);
 		
   
 	}
 	close(writefd);

    if(termination!=1){
	    if(	(writefd = open(fifo1, O_WRONLY)) <0){
	   	 	fprintf(stderr, "%d Cannot open write fifo\n",getpid() );
	   	 	if(write(writefd, "-f",  strlen("-f")) <0 ){
		   		fprintf(stderr, "%d Write error\n",getpid() );
		    }
	   	 	return -1;
	   }
	   
	}else{free(subs); free(buff); return 0;}

 	
   dirL=add_dir( dirL, input);//lista gia ta directories pou uparxoun sto input
   							//molis vrethei kapoio prostithetai sthn oura kai sthn epomenh loupa elegxetai 
   							//arxikopoieitai me to input
  

   if(is_emptyDir(input)==1){
    		printf("Empty input directory\n"); //den exei kati na grapsei to input einai adeio
		   	memset(buff,0,buff_size);
		   	strcpy(buff,"00");
		    if(write(writefd, buff,  5) <0 ){
		   		fprintf(stderr, "%d 1Write error\n",getpid() );
		    }

		   	free(subs);
		   	free(buff);
		   	free_dirList(dirL);
			close(writefd);   
		   	return 0;
    }



   curr=dirL;
   memset(buff,0,buff_size);

   while(curr!=NULL){
   		if(termination==1){ break;}

   		memset(path,0,3000);
   		strcpy(path,curr->dirname);
   		inp_dir =opendir(curr->dirname); //elegxetai to directory ths listas
   		count_dirs+=1;
   		if (inp_dir == NULL){ 
        	printf("%d: Could not open %s directory",getpid(),input ); 
        	
		   	free(subs);
		   	free(buff);
		   	free_dirList(dirL);
			
			if(write(writefd, "-f",  strlen("-f")) <0 ){ //stelnei shma apotuxias ston reader
		   		fprintf(stderr, "%d Write error\n",getpid() );
		    }
		    close(writefd);   

        	return -1;
    	} 

    	
    	
   		while ((de = readdir(inp_dir)) != NULL){ //oso uparxoun arxeia h directories sto directory pou eksetazetai apo thn lista
            memset(nest_path,0,3000);
            if(de->d_type==DT_DIR && strcmp(de->d_name,".")!=0 && strcmp(de->d_name,"..")!=0){ // an enai directory
            	//printf("%s is dir\n",de->d_name );
            	strcat(nest_path,path);
            	strcat(nest_path,"/");
            	strcat(nest_path,de->d_name);
            	dirL=add_dir( dirL, nest_path); //prosthetei to directory sthn lista wste na elegxthei meta

            	name_length=strlen(nest_path)-strlen(subs); //megethos onomatos dir
            	memset(buff,0,buff_size);
            	sprintf(buff,"%hu",name_length);

            	//printf("!> name length: %s \n",buff);
            	write_size=5;
            	if(termination==1){break;}
            	if(write(writefd, buff,  write_size) <0 ){
            		fprintf(stderr, "%d 1Write error\n",getpid() );
            		free(subs);
				   	free(buff);
				   	free_dirList(dirL);
            		
            		closedir(inp_dir); 
            		if(write(writefd, "-f",  strlen("-f")) <0 ){
		   				fprintf(stderr, "%d Write error\n",getpid() );
		    		} 
		    		close(writefd);
            		return -1;
            	}
            	
            	memset(buff,0,buff_size);
            	i=0;
            	j=strlen(subs) ;
            	//printf("!> file name: \n");
            	while( (j-strlen(subs)) <name_length){ //grafei to onoma tou dir
            		i=0;
            		memset(buff,0,buff_size);
            		while( i<buff_size && (j-strlen(subs)) <name_length){
            			buff[i]=nest_path[j];
            			i++;
            			j++;
            		}
  					
            		
            		buff=remove_substr(buff, subs); //diagrafei apo to full path tou arxeiou to path tou input px: ./input/dir1 -> dir1
            		if(termination==1){ break;}
            		if(write(writefd, buff,strlen(buff) )<0 ){ //grafei to onoma tou dir
            			fprintf(stderr, "%d 2Write error\n",getpid() );
            			free(subs);
					   	free(buff);
					   	free_dirList(dirL);
					   	closedir(inp_dir); 
            			
            			if(write(writefd, "-f",  strlen("-f")) <0 ){
		   				fprintf(stderr, "%d Write error\n",getpid() );
		    			} 
		    			close(writefd);
            			return -1;
            		}
            		
            	}
            	if(termination==1){break;}
            	memset(buff,0,buff_size);
            	file_length=0;
            	sprintf(buff,"%u",file_length); //gia to megethos tou directory grafei 0

            	write_size=10;
            	if(termination==1){ break;}
            	if(write(writefd, buff,  write_size)<0 ){
            		fprintf(stderr, "%d 3Write error\n",getpid() );
            		free(subs);
				   	free(buff);
				   	free_dirList(dirL);
            		
            		closedir(inp_dir);
            		if(write(writefd, "-f",  strlen("-f")) <0 ){
		   				fprintf(stderr, "%d Write error\n",getpid() );
		   			 }
		   			 close(writefd);  
            		return -1;
            	}
            	//printf(">! file size: %s\n",buff );

            	memset(buff,0,buff_size); //gia ta data tou directory grafei -d
            	strcpy(buff,"-d");
            	if(termination==1){  break;}
            	if(write(writefd, buff, strlen(buff))<0 ){
            		fprintf(stderr, "%d 4Write error\n",getpid() );
            		free(subs);
				   	free(buff);
				   	free_dirList(dirL);
            		
            		 closedir(inp_dir); 
            		 if(write(writefd, "-f",  strlen("-f")) <0 ){
		   				fprintf(stderr, "%d Write error\n",getpid() );
		   			 }
		   			 close(writefd);
            		 return -1;
            	}
            	//printf(">! data: %s\n",buff );

    	
            } 
            else if(de->d_type==DT_FIFO){
            	//printf("%s is fifo\n",de->d_name );
            }
            else if(de->d_type==DT_REG){ //an einai arxeio  
            	//printf("%s is reg file\n",de->d_name );
            	count_files+=1;
            	memset(file,0,3000);
            	strcat(file,path);
            	strcat(file,"/");
            	strcat(file,de->d_name);
               	
               	name_length=strlen(file)-strlen(subs);
            	memset(buff,0,buff_size);
            	sprintf(buff,"%hu",name_length);
            
            	write_size=5;
            	if(termination==1){  break;}
            	if(write(writefd, buff,  write_size)<0 ){
            		fprintf(stderr, "%d 5Write error\n",getpid() );
            		free(subs);
				   	free(buff);
				   	free_dirList(dirL);
            		
            		closedir(inp_dir); 
            		if(write(writefd, "-f",  strlen("-f")) <0 ){
		   				fprintf(stderr, "%d Write error\n",getpid() );
		   			 }
		   			 close(writefd);
            		 return -1;
            	}
            	//printf(">! name legth: %s\n",buff );
            	memset(buff,0,buff_size);
            
            	i=0;
            	j=strlen(subs);
            	//printf("!> file name: \n" );
            	while((j-strlen(subs))<name_length){
            		i=0;
            		memset(buff,0,buff_size);
            		while( i<buff_size && (j-strlen(subs)<name_length) ){
            			buff[i]=file[j];
            			i++;
            			j++;
            		}
            		//printf("\t %s\n",buff );
            		if(termination==1){  break;}
            		if(write(writefd, buff, strlen(buff))<0 ){
            			fprintf(stderr, "%d 6Write error\n",getpid() );
            			free(subs);
            			free(buff);
            			free_dirList(dirL);
            			
            			closedir(inp_dir);
            			if(write(writefd, "-f",  strlen("-f")) <0 ){
		   				fprintf(stderr, "%d Write error\n",getpid() );
			   			 }
			   			 close(writefd);  
            			return -1;
            		}
            		
            	}
            	if(termination==1){ break;}
            	fp = fopen(file, "r");
				if( fp == NULL )  {
 				 	fprintf(stderr, "%d Error opening file\n",getpid());
  					free(subs);
            		free(buff);
            		free_dirList(dirL);
  					close(writefd);
  					closedir(inp_dir); 
  					return(-1);
  				}
            	fseek(fp, 0L, SEEK_END); //diatrexei gia na vrei to megethos tou arxeiou
				file_length=(unsigned int)ftell(fp);
				rewind(fp);
				memset(buff,0,buff_size);
				sprintf(buff, "%u",(uint)file_length);
				write_size=10;
				if(termination==1){ break;}
				//data size
				if(write(writefd, buff,  write_size)<0 ){ 
					fprintf(stderr, "%d 7Write error\n",getpid() );
					 free(subs);
            		free(buff);
            		free_dirList(dirL);
					 
					 fclose(fp);
					 closedir(inp_dir);
					 if(write(writefd, "-f",  strlen("-f")) <0 ){
		   				fprintf(stderr, "%d Write error\n",getpid() );
		   			 }
		   			 close(writefd); 
					 return -1;
				}
				
				//printf("!> data: \n");
				if(file_length!=0){
					c=fgetc(fp);
					while(c!= EOF){
	            		i=0;
	            		memset(buff,0,buff_size);
	            		while( i<buff_size-1 && c!=EOF){ //grafei ston buffer mexri na teleiwsei to arxeio 
	            			buff[i]=c;
	            			c=fgetc(fp);
	            			i++;
	       
	            		}
	            		if(termination==1){ break;}
	            		buff[i]='\0';
	            		
	            		if(write(writefd, buff, strlen(buff))<0){ 
	            			if(termination==1){
	            				break;
	            			}
	            			else{
	            				fprintf(stderr, "%d 8Write error\n",getpid() );
	            				free(subs);
			            		free(buff);
			            		free_dirList(dirL);
			            		fclose(fp);
	            				
	            				closedir(inp_dir);
				            	if(write(writefd, "-f",  strlen("-f")) <0 ){
					   				fprintf(stderr, "%d Write error\n",getpid() );
					   			 }
					   			 close(writefd);    
	            				return -1;
	            			}
	            		}
	            			
	            		//printf("\t %s\n",buff );
	            		
	            	}
	            	if(termination==1){ break;}
	            	if( (logptr = fopen (log, "a"))==NULL ){
						fprintf(stderr, "%d Failed openning %s\n",getpid(),log );
						free(subs);
	            		free(buff);
	            		free_dirList(dirL);
	            		fclose(fp);
        				
        				closedir(inp_dir); 
        				if(write(writefd, "-f",  strlen("-f")) <0 ){
			   				fprintf(stderr, "%d Write error\n",getpid() );
			   			 }
			   			 close(writefd);   
						return -1;
					}
					fprintf(logptr, "!w %d\n",file_length );
	    			fclose(logptr);
				}
				else{
					memset(buff,0,buff_size);
					strcpy(buff,"-e"); //an einai adeio to arxeio gia data grafei -e
					if(termination==1){ break;}
					if(write(writefd, buff, strlen(buff))<0 ){
						fprintf(stderr, "%d 9Write error\n",getpid() );
						free(subs);
	            		free(buff);
	            		free_dirList(dirL);
						
						fclose(fp);
						closedir(inp_dir); 
						if(write(writefd, "-f",  strlen("-f")) <0 ){
			   				fprintf(stderr, "%d Write error\n",getpid() );
			   			 }
			   			 close(writefd);  
						return -1;
					}
					if( (logptr = fopen (log, "a"))==NULL ){ //grafei sto log arxeio ton arthmo twn bytes pou egrapse
						fprintf(stderr, "%d Failed openning %s\n",getpid(),log );
						free(subs);
	            		free(buff);
	            		free_dirList(dirL);
	            		fclose(fp);
        				
        				closedir(inp_dir);  
        				if(write(writefd, "-f",  strlen("-f")) >0 ){
			   				fprintf(stderr, "%d Write error\n",getpid() );
			   			 }
			   			 close(writefd);    
						return -1;
					}
					fprintf(logptr, "!w %d\n",0);
	    			fclose(logptr);
					

				}
				if(termination==1){ break;}
				fclose(fp);


            }
            else{
            	//printf("%s Not a file or dir\n",de->d_name );
            }  			
   		   
   		}
   		if(termination==1){ break;}
   		closedir(inp_dir); 
   		curr=curr->next; 
   		
   	}
	if(termination==0){ //grafei 00 gia to telos tou input
		printf("Total dirs %d and %d files\n",count_dirs-1,count_files );
	   	memset(buff,0,buff_size);
	   	strcpy(buff,"00");
	   	write_size=5;
	  	if(termination==1){ return 0;}
	   	if(write(writefd, buff, write_size)<0 ){
	   		free(subs);
		   	free(buff);
		   	free_dirList(dirL);
	   		fprintf(stderr, "%d 10Write error\n\n",getpid() ); 
	   		if(write(writefd, "-f",  strlen("-f")) <0 ){
   				fprintf(stderr, "%d Write error\n",getpid() );
   			 }
   			 close(writefd);  
	   		return -1;
	   	}
	}


   	free(subs);
   	free(buff);
   	free_dirList(dirL);
	close(writefd);   

	return 0;
}


static void handler(int sig){
	
		
	 	if(sig==SIGUSR1){ 
	 		termination=1; 
	 		printf("Writer received SIGIUSR1\n");
	 	}
		

		if(sig==SIGPIPE){
			termination=1; 
			broken_pipe=1;
			printf("Writer received SIGPIPE\n");
			
		}
		
	 
   
}

int main ( int argc, char **argv ) {
	printf("WRITER.C running %d.\n",getpid());
	
	int buff_size;
	char* common, *log;
	char* input;
	int myId, newId;
	
	
	char* fifo1=NULL;
	
	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
    struct sigaction sa;

    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = &handler;
    sa.sa_flags = SA_NODEFER;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 0;
    }
     if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("sigaction");
        return 0;
    }
     if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction");
        return 0;
    }

	

	if(termination==1){  fprintf(stderr, "Writer exits due to signal\n" ); return 0;}
	if(store_Warguments(argc,  argv, &common, &newId, &myId,&input,&buff_size , &log,&fifo1)==-1){
		//mallon tha prepei na enhmerwsw patera
		kill(getppid(),SIGUSR1);
		if(fifo1!=NULL){free(fifo1);}
		printf("Writer failed, sends SIGUSR1 to parent.\n");
		return 1;
	}

	
	//printf("FIFO %s\n",fifo1 );
	if(termination==1){ fprintf(stderr, "Writer exits due to signal\n" ); if(fifo1!=NULL){free(fifo1);}  return 0;}
	if ( (mkfifo(fifo1, PERMS) < 0) ) {
		if(errno==EEXIST){
			  
			//printf("%d open %s\n", getpid(),fifo1);
		}
		else {
			perror("can't create fifo"); 
			kill(getppid(),SIGUSR1);
			if(fifo1!=NULL){free(fifo1);} 
			fprintf(stderr, "Writer failed, sends SIGUSR1 to parent\n" );
			return 1;
		} 
	}
	else{printf("%d created %s\n",getpid(),fifo1 );}

	 
	if(termination==1){ fprintf(stderr, "Writer exits due to signal\n" ); if(fifo1!=NULL){free(fifo1);} return 0;}
   	if(scan_inputD(input, log,fifo1,  buff_size)==-1){
   		fprintf(stderr, "Writer failed, sends SIGUSR1 to parent\n");
   		kill(getppid(),SIGUSR1); //an apetuxe enhmerwnei ton patera
	
   	}

   	if(termination==1){
   		fprintf(stderr, "Writer exits due to signal\n" ); 
   		if(broken_pipe==1)kill(getppid(),SIGUSR1); //an dexthke sigpipe enhmerwnei ton patera
   		if(fifo1!=NULL){free(fifo1);}
   		return 0;
   	}
   	 
    printf(">>>Writer returns now\n"); 
    if(fifo1!=NULL){free(fifo1);}
    termination=0;
   
	return 0;
   	
}