#include "parse.h" 
job* jobList=NULL;
history* jobHist=NULL;
int main (int argc, char *argv[]){
	//signal (SIGINT,  sighandler);
  //signal (SIGCHLD, sighandler);

  struct sigaction act; 
  memset (&act, 0, sizeof(act));
  act.sa_handler = sighandler;
  act.sa_flags = SA_RESTART | SA_NOCLDSTOP | SA_SIGINFO;
 
  if (sigaction(SIGCHLD, &act, 0)) {
    perror ("sigaction");
    return 1;
  }else{
    //fprintf(stderr, "Sig\n");
  }

	while(1){
		int childPid;
		char *cmdLine;
		job *temp;
	  cmdLine=readline(printPrompt()); //or GNU readline("");
	  cmdLine[strlen(cmdLine)]='\n';
	  temp=parseCommand(cmdLine);
		if(addJob(temp)){
			fprintf(stderr, "Failed to add job! \n");
		}
		addHistory(temp);
		//printJobList(jobList);

    //notify();
		if(isBuiltInCommand(temp->command[0])){
		  executeBuiltInCommand(temp->command);
		}else{		
			executeCommand(temp);   
			if(checksBackground(temp)){
			   
			}else{
				//waitJob(temp);
			}		
   	}

	}
}

/* prints the prompt */
char* printPrompt(){
  char *cwd;
  if((cwd=getcwd(NULL,128))==NULL){
      perror("pwd");
      exit(2);
  }
  strncat(cwd,"%",strlen("%"));
  return cwd;
  //fprintf(stdout,"%s\n",cwd);
}

void sighandler(int signum){
  //while(waitpid(-1, NULL, WNOHANG)>0);
  //notify();
}

/* executes built in command from input */  
void executeBuiltInCommand(char** cmd){
  if(strncmp(cmd[0],"exit",strlen("exit"))==0){
    if(checksBackground()){
      exit(1);
    }else{
      fprintf(stderr, "%s\n","Background jobs have not been completed.");
    }
  }
  if(strncmp(cmd[0],"jobs",strlen("jobs"))==0){
    printBackground();
  }
  if(strncmp(cmd[0],"cd",strlen("cd"))==0){
    changeDirectory(cmd[1]);
  }
  if(strncmp(cmd[0],"history",strlen("history"))==0){
    printHistory();
  }
  if(strncmp(cmd[0],"kill",strlen("kill"))==0){
    killing(cmd[1]);
  }
  if(strncmp(cmd[0],"help",strlen("help"))==0){
    help();
  }
  if(strncmp(*cmd,"!",1)==0){
    job* j=NULL;
    int num;
    if(cmd[0][1]=='-'){
      fprintf(stderr, "previously %d\n",jobHist->jobNum);
      j=findJobHistory(jobHist->jobNum);
    }else{
      int i=1;
      int num=0;
      for(i=1;i<strlen(cmd[0]);i++){
          num=10*num+cmd[0][i]-'0';
          fprintf(stderr, "%s %d\n",cmd[0],num);
      }
      fprintf(stderr, "%d\n",num);
      j=findJobHistory(num);
    }
    if(j!=NULL){
      previousCommand(j);
    }else{
      fprintf(stderr, "Job does not exist in jobs history\n");
    }
  }
}

/* checks to see if command is built in */           
int isBuiltInCommand(char* cmd){
  if(strncmp(cmd,"exit",strlen("exit"))==0){
    return EXIT;
  }
  if(strncmp(cmd,"jobs",strlen("jobs"))==0){
    return EXIT;
  }
  if(strncmp(cmd,"cd",strlen("cd"))==0){
    return EXIT;
  }
  if(strncmp(cmd,"history",strlen("history"))==0){
    return EXIT;
  }
  if(strncmp(cmd,"kill",strlen("kill"))==0){
    return EXIT;
  }
  if(strncmp(cmd,"help",strlen("help"))==0){
    return EXIT;
  }
  if(strncmp(cmd,"!",1)==0){
    return EXIT;
  }
  return NO_BUILT_IN;
}


void previousCommand(job *j){
  addHistory(j);
  printJobList(jobList); 

  if(isBuiltInCommand(j->command[0])){
    executeBuiltInCommand(j->command);
  }else{    
    executeCommand(j);   
    if(checksBackground(j)){
         
    }else{
      //waitJob(j);
    }   
  }
}  

/* kill process selected */
void killing(char *cmd){
  if(cmd==NULL){
    fprintf(stderr, "%s\n", "Invalid syntax for this command");
  }else if(!isdigit(cmd[0])){
    fprintf(stderr, "%s\n", "Invalid syntax for this command");
  }else{
    int i=atoi(cmd);
    fprintf(stderr, "%d\n",i);
    process* p=findProc(i);
    fprintf(stderr, "kill\n");
    printProc(p);
    if(kill(i,SIGTERM)){
      p->status=COMPLETED;
      fprintf(stderr, "Kill Successful: %d\n",i);
    }else{
       fprintf(stderr, "Failed to Kill process: %d\n",i);

    }
  }
}

/* changes current working directory */
void changeDirectory(char *dir){
  char *cwd;
  if((cwd=getcwd(NULL,128))==NULL){
      perror("pwd");
      exit(2);
  }
  if(strncmp(cwd,".",strlen("."))!=0){
    strncat(cwd,"/",strlen("/"));
    strncat(cwd,dir,strlen(dir));
  }
  if(chdir(cwd)!=0){
    perror("NO DIRECTORY OF NAME PROVIDED");
  }
  //fprintf(stderr, "cwd:%s\n",cwd);
}

void help(){
  fprintf(stderr,"\nhelp prints syntax for builtin command.\n jobs provide a numbered list of processes currently executing in the background.\ncd should change the working directory.\nhistory should print the list of previously executed commands.\nexit should terminate your shell process.\nkill $num should terminate the process numbered in the list of background processes returned by jobs\n");
}

int markStatus(pid_t pid, int status){
  job *j;
  process *p;
  if(pid>0){
    for(j=jobList;j!=NULL;j=j->next){
      for (p=j->firstProcess;p!=NULL;p=p->next){
        if(p->pid==pid){
          p->status=status;
          if(WIFSTOPPED(status)){
            p->status=STOPPED;
            fprintf(stderr,"MarkStatus: Process: %d Stopped by signal %d.\n",pid,WSTOPSIG(p->status));
          }else{
            p->status=COMPLETED;
            if(WIFSIGNALED(status)){
              fprintf(stderr,"MarkStatus: Process: %d Terminated by signal %d.\n",pid,WTERMSIG(p->status));
            }
          }
          if(p->status==2){
            fprintf(stderr, "MarkStatus: Error not complete %d\n",pid);
          }
          return 0;
        }
      }
    }
  }
  return -1;
}

void waitJob(job *j){
  process* p;
  for(p=j->firstProcess;p!=NULL;p=p->next){
    if(waitpid(p->pid, &p->status, WUNTRACED)==-1){
      fprintf(stderr, "ERROR WAITPID %s\n",strerror(errno));
    }
    markStatus(p->pid,p->status);
  }
  if(jobStopped(j)&&jobCompleted(j)){
    fprintf(stderr, "JOB COMPLETED\n");
  }

  /*
  int status;
  pid_t pid;
  do{
    pid=waitpid(WAIT_ANY, &status, WUNTRACED);
  }while(!markStatus(pid, status)&&!jobStopped(j)&&!jobCompleted(j));*/

}

void updateStatus(){
  int status;
  pid_t pid;
  do{
    pid=waitpid(WAIT_ANY,&status,WUNTRACED|WNOHANG|WCONTINUED);
  }while(!markStatus(pid, status));
}

void notify(){
  job *j;
  updateStatus();
  for(j=jobList;j!=NULL;j=j->next){
    if(jobCompleted(j)){
      jobInfo(j,"COMPLETED");
      delJob(j->jid);
    }else if(jobStopped(j)&& !j->status==ONWARDS){
      jobInfo(j,"STOPPED");
    }else{
      continue;
    }
  }
}

void jobInfo(job *j,char *message){
	fprintf(stderr, "Job[%d] %s ", j->jid,message);
  printJobcommand(j);
  fprintf(stderr, "\n");
}


/* Return true if all processes in the job have stopped or completed. */
int jobStopped(job *j){
  process *p;
  for(p=j->firstProcess; p!=NULL; p=p->next)
    if(p->status==COMPLETED&&p->status==STOPPED)
      return -1;
  return 1;
}

/* Return true if all processes in the job have completed. */
int jobCompleted(job *j){
  process *p;
  for(p=j->firstProcess; p!=NULL; p=p->next)
    if(p->status==COMPLETED)
      return -1;
  return 0;
}


int addHistory(job *j){
	history* hist=(history *)malloc(sizeof(history));
	if(j==NULL){
 		return -1;
 	}
 	if(jobHist==NULL){
 		hist->jobNum=1;
 	}else{
 		hist->jobNum=jobHist->jobNum+1;
 	}
  hist->next=jobHist;
  hist->j=j;
  jobHist=hist;
 // fprintf(stderr, "addHist\n");
  return 0;
}

job* findJobHistory(int num){
  //fprintf(stderr, "find job\n" );
	history* temp;
	if(jobHist==NULL){
		return NULL;
	}
	for(temp=jobHist;temp->jobNum!=num && temp!=NULL ;temp=temp->next){
    fprintf(stderr, "%d\n",temp->jobNum);
		if(temp==NULL){
      break;
    }
	}/*
  job* j=makeJob();
  
  j->numProcJob=0;
  j->numPipe=temp->j->numPipe;
  j->numofCommand=temp->j->numofCommand;
  j->command=(char **)realloc(j->command,10*temp->j->numofCommand*sizeof(char *));
  int i;
  for(i=0;i<temp->j->numofCommand;i++){
    j->command[i]=(char*)malloc((strlen(temp->j->command[i])+1)*sizeof(char));
    strcpy(j->command[i],temp->j->command[i]);
    j->command[i][i+1]='\0';
  }

  j->firstProcess=makeProcess();
  j->jid=-1;
  j->fgbgState=temp->j->fgbgState;
  j->ioState=temp->j->ioState;
  j->status=2;

  j->firstProcess->argc=temp->j->firstProcess->argc;
  j->firstProcess->argv=(char **)realloc(j->firstProcess->argv,10*temp->j->firstProcess->argc*sizeof(char *));
  for(i=0;i<temp->j->firstProcess->argc;i++){
    j->firstProcess->argv[i]=(char*)malloc((strlen(temp->j->firstProcess->argv[i])+1)*sizeof(char));
    strcpy(j->firstProcess->argv[i],temp->j->firstProcess->argv[i]);
    j->firstProcess->argv[i][i+1]='\0';
  }
  j->firstProcess->pid=-1;
  j->firstProcess->status=-1;
  j->firstProcess->ioState=-1;*/
	return temp->j;
}

void printHistory(){
	history* hist;
	for(hist=jobHist;hist!=NULL;hist=hist->next){
		fprintf(stderr, "JOB[%d] ",hist->jobNum);
    printJobcommand(hist->j);
    fprintf(stderr, "\n");
	}
}

void printJobcommand(job *j){
  int i;
  for(i=0;i<j->numofCommand;i++){
    fprintf(stderr, "%s ",j->command[i]);
  }
}


process* findProc(int num){
  job* j;
  process* p;
  for(j=jobList;j!=NULL;j=j->next){
    if(strncmp(j->command[0],"kill",strlen("kill"))==0){
      if(atoi(j->command[1])==num){
        p=j->firstProcess;
        return p;
      }
    }
  }
}    

/* checks to see if job is on Background or Foreground */    
int isBackgroundJob(job* j){
  return j->fgbgState;
}

/* checks to see if all jobs in background have completed */
int checksBackground(){
  job *j;
  for(j=jobList;j!=NULL;j=j->next){
    if(j->fgbgState==Background&&j->status!=COMPLETED){
      return -1;
    }
  }
  return 0;
}

/* print background processes */
void printBackground(){
  job* j;
  process* p;
  for(j=jobList;j!=NULL;j=j->next){
    if(j->fgbgState==Background){
      for(p=j->firstProcess;p!=NULL;p=p->next){
        if(p->status==2){
          fprintf(stderr, "Process: %d ",p->pid);
          printJobcommand(j);
          fprintf(stderr, "\n");
        }
      }
    }
  }
}

/*
char* printCommand(char** command,num){
  int i,numProc=num;
  char* command=(char *)malloc(sizeof(char));
  for(i=0;i<j->numProc;i++){
    command=(char *)realloc(command,(strlen(word))*sizeof(char));
  }
}*/
/* add job to list */
int addJob(job* j){
 	if(j==NULL){
 		return -1;
 	}
 	if(jobList==NULL){
 		j->jid=1;
 	}else{
 		j->jid=jobList->jid+1;
 	}
  j->next=jobList;
  jobList=j;
  return 0;
}  

/* delete job from list */
int delJob(pid_t jid){
  job *del,*prev;
  del=jobList;
  prev=jobList;
  if(del==NULL){
    return -1;
  }
  while(del->jid!=jid){
  	if(del==NULL){
  		return -1;
  	}
  	prev=del;
  	del=del->next;
  }
  prev->next=del->next;
  return 0;
}
