#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

enum {NO_BUILT_IN=0,KILL,HISTORY,CD,JOBS,EXIT,HELP,PREVIOUS}; /* built in commands that can be executed */
enum {Foreground=0,Background};                           /* determine if job is foreground or background */
enum {STDIN=0, STDOUT, STDERR, APPEND, PIPE};               /* standard i/o channels */
enum {COMPLETED=0,STOPPED,ONWARDS};                     /* process has completed or stopped or continue */

/* A process is a single process.  */
typedef struct process{
  struct process *next;             /* next process in list */
  char **argv;                     /* for exec or execvp */
  char *message;                    /* message handling, used for messages */
  pid_t pid;                        /* process ID */
  int status;                       /* process has completed or stopped or continue */
  int ioState;                      /* standard i/o channels */
  int argc;                         /* number of argv */
} process;

/* A job is a list of processes.  */
typedef struct job{
  struct job *next;                 /* next active job */
  int numProcJob;                   /* number of processes in job */
  int numofCommand;
  char *message;                    /* message handling, used for messages */
  char **command;                  /* commands to be executed */
  process *firstProcess;            /* list of processes in this job */
  pid_t jid;                       /* processes group ID */
  int fgbgState;                    /* determine if job is foreground or background */
  int ioState;                      /* standard i/o channels */
  int status;                      /* job has completed or stopped or continue */
  int numPipe;                       /* number of pipes in job */
} job;

/* A history is a list of jobs  */
typedef struct history{
  struct history* next;   /* next job in history */
  struct job *j;          /* job  */
  int jobNum;             /* number of job in history */
} history;

/* list of prototype */
char* printPrompt();                    /* prints the prompt */
void executeBuiltInCommand(char **cmd);  /* executes built in commands */
void launchJob(job* jobList);           /* launch a job */
int isBuiltInCommand(char *cmd);        /* execute command from the OS */
job* parseCommand(char* cmdLine);       /* parse the input */
void printJobList();                    /* prints job list */
void executeCommand(job *j);         /* executes command from input */
int isBackgroundJob(job* j);         /* checks to see if job is on Background or Foreground */
job* findJob(pid_t jid,job* jobList);   /* Find the active job with the indicated processes group id. */
int jobStopped(job *j);                 /* Return true if all processes in the job have stopped or completed. */
int jobCompleted(job *j);               /* Return true if all processes in the job have completed. */
int addHistory(job *j);                 /* add job to history */
job* findJobHistory(int num);
void printHistory();
int addJob(job* j);                    /* add job to list */
int delJob(pid_t jid);                     /* delete job from list */
int addProcess(job *j);    /* add process to job list */
int delProcess(process *p);             /* delete process from job list */
job* makeJob();                         /* allocate space for a job */
process* makeProcess();    /* allocate space for a process */
void errorInt(int message);
void errorString(char *message);
int checksBackground();
void updateStatus();
void notify();
int markStatus(pid_t pid, int status);
void jobInfo(job *j,char *message);
void sighandler(int signum);
void launchProcess(process *p,char* file, int fgbgState);
void launchPipe(process* p1,process* p2, int fgbgState);
void changeDirectory(char *dir);
void killing(char *cmd);
void previousCommand(job *j);
void waitJob(job *j);
void help();
void printBackground();
void printJobcommand(job *j);
void printProcCommand(process *p);
process* findProc(int num);
void printProc();

