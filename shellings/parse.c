#include "parse.h" 

/* parse the input */  
job* parseCommand(char* cmdLine){
	//fprintf(stderr, "%s\n","PARSECOMMAND");
	job* newjob=makeJob();
	newjob->firstProcess=makeProcess();
  char *word=(char *)malloc(20*sizeof(char));
  if(word==NULL){
    fprintf(stderr, "%s\n","Error line 43");
  }
  int njobs=20;
  int nargs=20;
  int wlen=20;
	int i=0,j=0;
  int endofarg=0;
  int endofline=0;
  int notaproc=0;
  while(cmdLine[i]!='\n'){
    //errorInt(i);
    //errorInt(endofline);
    //errorString("BEGIN");
    //errorString(&cmdLine[i]);
    switch(cmdLine[i]){
      case '\n':
        endofline=1;
        break;
      case ' ': case '\t':
        i++;
        continue;      
      case '|':
          if(newjob->numofCommand==njobs){
            newjob->command=(char**)realloc(newjob->command,(njobs*=2) * sizeof(char *));
          }
          newjob->command[newjob->numofCommand]=(char*)malloc((2)*sizeof(char));
          strcpy(newjob->command[newjob->numofCommand],"|");
          newjob->numofCommand++;
          newjob->numPipe++;
          newjob->numProcJob++;
          i++;
        break;
      case '<':
          if(newjob->numofCommand==njobs){
            newjob->command=(char**)realloc(newjob->command,(njobs*=2) * sizeof(char *));
          }
          newjob->command[newjob->numofCommand]=(char*)malloc((2)*sizeof(char));
          strcpy(newjob->command[newjob->numofCommand],"<");
          newjob->numofCommand++;
          i++;
          notaproc=1;
        break;
      case '>':
          if(newjob->numofCommand==njobs){
            newjob->command=(char**)realloc(newjob->command,(njobs*=2) * sizeof(char *));
          }
          if(cmdLine[i+1]!='>'){
            newjob->command[newjob->numofCommand]=(char*)malloc(2*sizeof(char));
            strcpy(newjob->command[newjob->numofCommand],">");
            newjob->numofCommand++;
            i++;
          }else{
            newjob->command[newjob->numofCommand]=(char*)malloc(3*sizeof(char));
            strcpy(newjob->command[newjob->numofCommand],">>");
            newjob->numofCommand++;
            i+=2;
          }
          notaproc=1;
        break;
      case '&':
        if(newjob->numofCommand==njobs){
          newjob->command=(char**)realloc(newjob->command,(njobs*=2) * sizeof(char *));
        }
        newjob->command[newjob->numofCommand]=(char*)malloc((2)*sizeof(char));
        strcpy(newjob->command[newjob->numofCommand],"&");
        newjob->fgbgState=1;
        endofline=1;
        newjob->numofCommand++;
        i++;
        break;
      default:
          endofarg=0;
          while(endofarg==0){
            //errorInt(i);
            //errorInt(endofarg);
            //errorString("BEGIN2");
            switch(cmdLine[i]){
              case '\n': 
                endofline=1;
                endofarg=1;
                break;
              case '|':
                newjob->numofCommand++;
                endofarg=1;
                break;
              case ' ': case '\t':
                i++;
                continue;
              case '.':
                if(newjob->firstProcess->argc==nargs){
                  newjob->command=(char**)realloc(newjob->command,(njobs*=2) * sizeof(char *));
                  newjob->firstProcess->argv=(char**)realloc(newjob->firstProcess->argv,(nargs*=2) * sizeof(char *));
                }
                for(j=0;cmdLine[i]!=' ' && cmdLine[i]!='\n';j++,i++){
                   if(wlen==strlen(word)){
                    wlen*=2;
                    word=(char *)realloc(word,wlen*sizeof(char));
                  }
                  word[j]=cmdLine[i];
                }
                word[j]='\0';
                newjob->command[newjob->numofCommand]=(char*)malloc((strlen(word))*sizeof(char));
                newjob->firstProcess->argv[newjob->firstProcess->argc]=(char *)malloc((strlen(word))*sizeof(char));
                strcat(newjob->command[newjob->numofCommand],word);
                strcpy(newjob->firstProcess->argv[newjob->firstProcess->argc],word);
                word=(char *)realloc(word,1*sizeof(char));
                newjob->firstProcess->argc++;
                break;
              case '$':
                if(newjob->firstProcess->argc==nargs){
                  newjob->command=(char**)realloc(newjob->command,(njobs*=2) * sizeof(char *));
                  newjob->firstProcess->argv=(char**)realloc(newjob->firstProcess->argv,(nargs*=2) * sizeof(char *));
                }
                i++;
                for(j=0;isdigit(cmdLine[i]);j++,i++){
                   if(wlen==strlen(word)){
                    wlen*=2;
                    word=(char *)realloc(word,wlen*sizeof(char));
                  }
                  word[j]=cmdLine[i];
                }
                word[j]='\0';
                newjob->command[newjob->numofCommand]=(char*)malloc((strlen(word))*sizeof(char));
                newjob->firstProcess->argv[newjob->firstProcess->argc]=(char *)malloc((strlen(word))*sizeof(char));
                strcat(newjob->command[newjob->numofCommand],word);
                strcpy(newjob->firstProcess->argv[newjob->firstProcess->argc],word);
                word=(char *)realloc(word,1*sizeof(char));
                newjob->firstProcess->argc++;
                newjob->numProcJob++;
                break;
              case '-':
                if(newjob->firstProcess->argc==nargs){
                  newjob->command=(char**)realloc(newjob->command,(njobs*=2) * sizeof(char *));
                  newjob->firstProcess->argv=(char**)realloc(newjob->firstProcess->argv,(nargs*=2) * sizeof(char *));
                }
                if(wlen==strlen(word)){
                  wlen*=2;
                  word=(char *)realloc(word,wlen*sizeof(char));
                }
                word[0]=cmdLine[i++];
                for(j=1;isalpha(cmdLine[i]);j++,i++){
                  if(wlen==strlen(word)){
                    wlen*=2;
                    word=(char *)realloc(word,wlen*sizeof(char));
                  }
                  word[j]=cmdLine[i];
                }
                word[j]='\0';
                errorString(word);
                newjob->command[newjob->numofCommand]=(char*)malloc((strlen(word))*sizeof(char));
                newjob->firstProcess->argv[newjob->firstProcess->argc]=(char *)malloc((strlen(word))*sizeof(char));
                strcat(newjob->command[newjob->numofCommand],word);
                strcpy(newjob->firstProcess->argv[newjob->firstProcess->argc],word);
                word=(char *)realloc(word,1*sizeof(char));
                newjob->firstProcess->argc++;
                break;
              case '"':
                if(newjob->firstProcess->argc==nargs){
                  newjob->command=(char**)realloc(newjob->command,(njobs*=2) * sizeof(char *));
                  newjob->firstProcess->argv=(char**)realloc(newjob->firstProcess->argv,(nargs*=2) * sizeof(char *));
                }
                for(j=0;cmdLine[i]!='"';j++,i++){
                   if(wlen==strlen(word)){
                    wlen*=2;
                    word=(char *)realloc(word,wlen*sizeof(char));
                  }
                  word[j]=cmdLine[i];
                }
                word[j]='\0';
                newjob->command[newjob->numofCommand]=(char*)malloc((strlen(word))*sizeof(char));
                newjob->firstProcess->argv[newjob->firstProcess->argc]=(char *)malloc((strlen(word))*sizeof(char));
                strcat(newjob->command[newjob->numofCommand],word);
                strcpy(newjob->firstProcess->argv[newjob->firstProcess->argc],word);
                word=(char *)realloc(word,1*sizeof(char));
                newjob->firstProcess->argc++;
                break;
                case '!':
                if(newjob->firstProcess->argc==nargs){
                  newjob->command=(char**)realloc(newjob->command,(njobs*=2) * sizeof(char *));
                  newjob->firstProcess->argv=(char**)realloc(newjob->firstProcess->argv,(nargs*=2) * sizeof(char *));
                }
                for(j=0;cmdLine[i]!=' ' && cmdLine[i]!='\n';j++,i++){
                   if(wlen==strlen(word)){
                    wlen*=2;
                    word=(char *)realloc(word,wlen*sizeof(char));
                  }
                  word[j]=cmdLine[i];
                }
                word[j]='\0';
                newjob->command[newjob->numofCommand]=(char*)malloc((strlen(word))*sizeof(char));
                newjob->firstProcess->argv[newjob->firstProcess->argc]=(char *)malloc((strlen(word))*sizeof(char));
                strcat(newjob->command[newjob->numofCommand],word);
                strcpy(newjob->firstProcess->argv[newjob->firstProcess->argc],word);
                word=(char *)realloc(word,1*sizeof(char));
                newjob->firstProcess->argc++;
                newjob->numProcJob++;
                break;
              default:
                if(newjob->firstProcess->argc==nargs){
                  newjob->command=(char**)realloc(newjob->command,(njobs*=2) * sizeof(char *));
                  newjob->firstProcess->argv=(char**)realloc(newjob->firstProcess->argv,(nargs*=2) * sizeof(char *));
                }
                  for(j=0;cmdLine[i]!=' ' && cmdLine[i]!='\n';j++,i++){
                    if(wlen==strlen(word)){
                      wlen*=2;
                      word=(char *)realloc(word,wlen*sizeof(char));
                    }
                    word[j]=cmdLine[i];
                  }
                  word[j]='\0';
                
                endofarg=1;
                newjob->command[newjob->numofCommand]=(char*)malloc((strlen(word))*sizeof(char));
                strcat(newjob->command[newjob->numofCommand],word);
                if(notaproc!=1){
                  newjob->firstProcess->argv[newjob->firstProcess->argc]=(char *)malloc((strlen(word))*sizeof(char));
                  strcpy(newjob->firstProcess->argv[newjob->firstProcess->argc],word);
                  newjob->firstProcess->argc++;
                }
                word=(char *)realloc(word,1*sizeof(char));
                newjob->numProcJob++;
                break;
            }
          }
          newjob->numofCommand++;
          //errorInt(newjob->jid);
    }
  }
  //fprintf(stderr, "%s\n","PARSE");
  //printJobList(newjob);
  //fprintf(stderr, "%s\n","PARSE");
  return newjob;
}

void errorString(char *message){
  fprintf(stderr, "%s\n",message);
}

void errorInt(int message){
  fprintf(stderr, "%d\n",message);
}

/* prints job list */
void printJobList(job* jobList){
  job* j;
  process* p;
  char **c;
  int i;
  for(j=jobList;j!=NULL;j=j->next){
    fprintf(stderr, "Job:\n");
    fprintf(stderr, "Job id: %d\n",j->jid);
    fprintf(stderr, "Job message: %s\n",j->message);
    fprintf(stderr, "Job numProcJob: %d\n",j->numProcJob);
    fprintf(stderr, "Job numPipe: %d\n",j->numPipe);
    fprintf(stderr, "Job fgbgState: %d\n",j->fgbgState);
    fprintf(stderr, "Job ioState: %d\n",j->ioState);
    fprintf(stderr, "Job status: %d\n",j->status);
    fprintf(stderr, "Job numofCommand: %d\n",j->numofCommand);
    fprintf(stderr, "Jobs: %s\n",j->command[0]);
    c=j->command;
    for(i=0;i<j->numofCommand;i++){
      fprintf(stderr, "%s ",j->command[i]);
    }
    fprintf(stderr, "\n");
    for(p=j->firstProcess;p!=NULL;p=p->next){
      fprintf(stderr, "Process:\n");
      fprintf(stderr, "Process id: %d\n",p->pid);
      fprintf(stderr, "Process argc: %d\n",p->argc);
      fprintf(stderr, "Process message: %s\n",p->message);
      fprintf(stderr, "Process status: %d\n",p->status);
      fprintf(stderr, "Process ioState: %d\n",p->ioState);
      fprintf(stderr, "Process argv: %s\n",p->argv[0]);
      c=p->argv;
      for(i=0;i<p->argc;i++){
        fprintf(stderr, "%s \n",p->argv[i]);
      }
    }
  }
}      

void printProc(process* p){
  process* p1;
  int i;
  for(p1=p;p1!=NULL;p1=p1->next){
      fprintf(stderr, "Process:\n");
      fprintf(stderr, "Process id: %d\n",p1->pid);
      fprintf(stderr, "Process argc: %d\n",p1->argc);
      fprintf(stderr, "Process message: %s\n",p1->message);
      fprintf(stderr, "Process status: %d\n",p1->status);
      fprintf(stderr, "Process ioState: %d\n",p1->ioState);
      fprintf(stderr, "Process argv: %s\n",p1->argv[0]);
      for(i=0;i<p1->argc;i++){
        fprintf(stderr, "%s \n",p1->argv[i]);
      }
    }
}

void executeCommand(job *j){
  int i;
  /*for(i=0;i<j->numPipe;i++){
    //launchProcess(j);
  }*/
  //char** c=j->command;

  //fprintf(stderr, "command\n");
  if(j->numProcJob==1 && j->numPipe<=1){
    launchProcess(j->firstProcess,NULL,j->fgbgState);
    //fprintf(stderr, "last command\n");
  }else{
  for(i=0;i<j->numofCommand;i++){
      if(strncmp(j->command[i],">",strlen(">"))==0){
        j->firstProcess->ioState=STDOUT;
        launchProcess(j->firstProcess,j->command[i+1],j->fgbgState);
      }
      if(strncmp(j->command[i],"<",strlen("<"))==0){  
        j->firstProcess->ioState=STDIN;
        launchProcess(j->firstProcess,j->command[i+1],j->fgbgState);
      }
      /*case ">>":
        if((j->ioState=open(&j->command[i+1][i+1],O_APPEND))==-1){
          fprintf(stderr, "Cannot open file\n");
          exit(1);
        }
        j->firstProcess->ioState=APPEND;
        launchProcess(j->firstProcess,&j->command[i-1][i-1]);
        return;*/
      /*case '|':
        launchPipe(j->firstProcess,j->firstProcess->next,j->fgbgState);
        return;*/
    }
  }
}


/*
void launchPipe(process* p1,process* p2, int fgbgState){
  int pipes[2];
  if(pipe(pipes)==-1){
    fprintf(stderr, "Error opeing pipe\n");
  }
  p1->pid=fork();
  if(p1->pid==0){
    close(pipes[0]);
    if(dup2(pipes[1],0)==-1){
      fprintf(stderr, "Cannot open file\n");
    }
    execvp(p1->argv[0],p1->argv);
  }
  p2->pid=fork();
  if(p2->pid==0){
    close(pipes[0]);
    if(dup2(pipes[0],1)==-1){
      fprintf(stderr, "Cannot open file\n");
    }
    execvp(p2->argv[0],p2->argv);
  }
}*/


void printProcCommand(process *p){
  int i;
  for(i=0;i<p->argc;i++){
    fprintf(stderr, "%s ",p->argv[i]);
  }
}

void launchProcess(process *p,char* file,int fgbgState){
  p->pid=fork();
  fprintf(stderr, "launch\n" );
  if(p->pid==0){
    fprintf(stderr, "STD\n" );
    if(p->ioState==STDIN){
      if((p->ioState=open(file,O_RDONLY))==-1){
        fprintf(stderr, "Cannot open file\n");
      }
      fprintf(stderr, "STDIN\n" );
      dup2(p->ioState,STDIN_FILENO);
      if(execvp(p->argv[0],p->argv)<0){
        fprintf(stderr, "Error executing:");
          printProcCommand(p);
           fprintf(stderr, "\n");
      }
    }else if(p->ioState==STDOUT){
      if((p->ioState=open(file,O_WRONLY|O_CREAT))==-1){
        fprintf(stderr, "Cannot open file\n");
      }
      fprintf(stderr, "STDOUT\n" );
      dup2(p->ioState,STDOUT_FILENO);
        if(execvp(p->argv[0],p->argv)<0){
          fprintf(stderr, "Error executing:");
          printProcCommand(p);
          fprintf(stderr, "\n");
        }
    }else if(p->ioState==APPEND){
      if((p->ioState=open(file,O_APPEND))==-1){
        fprintf(stderr, "Cannot open file\n");
      }
      dup2(p->ioState,1);
      if(execvp(p->argv[0],p->argv)<0){
        fprintf(stderr, "Error executing:");
          printProcCommand(p);
         fprintf(stderr, "\n");
      }
    }if(file==NULL){
      if(execvp(p->argv[0],p->argv)<0){
        fprintf(stderr, "Error executing:");
          printProcCommand(p);
          fprintf(stderr, "\n");
      }
    }
  }else{
    if(fgbgState==Foreground){
      if(waitpid(p->pid, &p->status, WUNTRACED)==-1){
        fprintf(stderr, "launchProcess ERROR WAITPID %s\n",strerror(errno));
      }
    }else{
      while(waitpid(-1, NULL, WNOHANG)>0);
    }
    notify();
  }
}

/* allocate space for a job */
job* makeJob(){
  job* newjob=(job *)malloc(sizeof(job));
  newjob->next=NULL;
  newjob->numProcJob=0;
  newjob->numPipe=0;
  newjob->numofCommand=0;
  newjob->message=(char *)calloc(20,sizeof(char));
  newjob->command=(char **)calloc(20,sizeof(char *));
  newjob->command[0]=(char *)calloc(20,sizeof(char ));
  if(newjob->message==NULL || newjob->command==NULL || newjob==NULL){
    fprintf(stderr, "%s\n", "ERROR on line 202");
  }
  newjob->firstProcess=NULL;
  newjob->jid=-1;
  newjob->fgbgState=0;
  newjob->ioState=-1;
  newjob->status=2;
  return newjob;
} 

/* allocate space for a process */                    
process* makeProcess(){
  process* p=(process *)malloc(sizeof(process));
  p->next=NULL;
  p->message=(char *)calloc(20,sizeof(char ));;
  p->argc=0;
  /*
  p->argv=(char **)malloc(sizeof(char *));
  for(i=0;command[i]!=NULL;i++){
    p->argv=(char **)realloc(i*sizeof(char *));
    p->argv=(char *)malloc((strlen(*command)+1)*sizeof(char)));
    strcpy(p->argv[i],command[i]);
  }*/
  p->argv=(char **)calloc(20,sizeof(char *));
  p->argv[0]=(char *)calloc(20,sizeof(char ));
  p->pid=-1;
  p->status=-1;
  p->ioState=-1;
  if(p->argv==NULL || p->message==NULL || p==NULL){
    fprintf(stderr, "%s\n", "ERROR on line 229");
  }
  return p;
}

/* add process to job list */
int addProcess(job *j){
  process *p;
  if(j->firstProcess){
    j->firstProcess=makeProcess();
    return 0;
  }else{
    for(p=j->firstProcess;p!=NULL;p=p->next)
      ;
      p=makeProcess();
    return 0;
  }
  return -1;
}  

/* delete process from job list */
int delProcess(process *p);         

/* Find the active job with the indicated processes group id. */
job* findJob(pid_t jid,job* jobList){
  job *j;
  for(j=jobList; j!=NULL; j=j->next)
    if(j->jid==jid)
      return j;
  return NULL;
}
