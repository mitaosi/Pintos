/* 
 * Main source code file for lsh shell program
 *
 * You are free to add functions to this file.
 * If you want to add functions in a separate file 
 * you will need to modify Makefile to compile
 * your additional functions.
 *
 * Add appropriate comments in your code to make it
 * easier for us while grading your assignment.
 *
 * Submit the entire lab1 folder as a tar archive (.tgz).
 * Command to create submission archive: 
      $> tar cvf lab1.tgz lab1/
 *
 * All the best 
 */


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "parse.h"

#define READ_END 0
#define WRITE_END 1

/*
 * Function declarations
 */

void PrintCommand(int, Command *);
void PrintPgm(Pgm *);
void RunPgm(Pgm *);
void Redirection(Command*);
void stripwhite(char *);
void SIGCHLD_handler(int sig);
void SIGINT_handler(int sig);
/* When non-zero, this global means the user is done using this program. */
int done = 0;
pid_t foreground_pid = 0;
/*
 * Name: main
 *
 * Description: Gets the ball rolling...
 *
 */
int main(void)
{
  Command cmd;
  int n;

  while (!done) {

    signal(SIGCHLD,SIGCHLD_handler);
    signal(SIGINT,SIGINT_handler);
    char *line;
    line = readline("> ");

    if (!line) {
      /* Encountered EOF at top level */
      done = 1;
    }
    else {
      /*
       * Remove leading and trailing whitespace from the line
       * Then, if there is anything left, add it to the history list
       * and execute it.
       */
      stripwhite(line);

      if(*line) {
        add_history(line);
        /* execute it */
        n = parse(line, &cmd);
        PrintCommand(n, &cmd);

        if(!strcmp(cmd.pgm->pgmlist[0], "cd")) { chdir(cmd.pgm->pgmlist[0]); }   

        if(!strcmp(cmd.pgm->pgmlist[0], "exit")) { exit(0); }

        pid_t pid = fork();

        if(pid < 0)
        {
            printf("Fork failed");
        }
        // Child process
        else if(pid == 0)
        {
            Redirection(&cmd);
            RunPgm(cmd.pgm);
        }
        // Parent process
        else if(pid > 0)
        {
            int stat_val;
            foreground_pid=pid;

            if(!cmd.bakground && wait(&stat_val)== -1){
                printf("Error to waiting child process terminated \n");
                exit(-1);
            }
            // None  process running in the foreground
            foreground_pid = 0;
        }
        
      }
    }
    
    if(line) {
      free(line);
    }
  }
  return 0;
}

void
RunPgm(Pgm* pgm)
{
      if(pgm == NULL) return;

      if(pgm->next)
      {
        int fd[2];
        if(pipe(fd) == -1) { printf("Pipe failed"); }

        pid_t pid = fork();

        if(pid < 0) { printf("Fork failed"); }
        // Child process
        else if(pid == 0)
        {
           close(fd[READ_END]);
           dup2(fd[WRITE_END], 1);
           close(fd[WRITE_END]);  
           RunPgm(pgm->next);
        }
        // Parent process
        else if(pid > 0)
        {
           close(fd[WRITE_END]);  
           dup2(fd[READ_END], 0);
           close(fd[READ_END]);
           wait(NULL);
        }

      }

      if(execvp(pgm->pgmlist[0], pgm->pgmlist) == -1) { printf("Error"); };
}

void
Redirection(Command* cmd)
{
     if(cmd->rstdin)
     {
         int in_fd;
         if((in_fd = open(cmd->rstdin, O_RDONLY)) == -1) { printf("Error opening file"); }
         dup2(in_fd, 0);
         close(in_fd);
     }

     if(cmd->rstdout)
     {
         int out_fd;
         if((out_fd = open(cmd->rstdout, O_WRONLY| O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) { printf("Error opening file"); }
         if(dup2(out_fd, 1) != 1) { printf("dup2 error"); }
         close(out_fd);
     }
}

void SIGCHLD_handler(int sig){
    if(sig ==SIGCHLD){
      int stat_val;
      pid_t pid;
      // WNOHANG = return immediately if no child process existed
      while((pid=waitpid(-1,&stat_val,WNOHANG))>0){
        printf("Process=%d terminated.\n ",pid);
      }
    }
}


/*
 * Name:  SIGINT handler
 *
 * Description: handle SIGINT signal(ctrl+C) and terminate 
 * foreground process
 *
 */
void SIGINT_handler(int sig){
  if(sig == SIGINT && foreground_pid > 0){
    if((kill(foreground_pid,SIGTERM))==-1)
        printf("Failed to terminate process, error");
    else
    {
      foreground_pid = 0;
    }
    
  }
}


/*
 * Name: PrintCommand
 *
 * Description: Prints a Command structure as returned by parse on stdout.
 *
 */
void
PrintCommand (int n, Command *cmd)
{
  printf("Parse returned %d:\n", n);
  printf("   stdin : %s\n", cmd->rstdin  ? cmd->rstdin  : "<none>" );
  printf("   stdout: %s\n", cmd->rstdout ? cmd->rstdout : "<none>" );
  printf("   bg    : %s\n", cmd->bakground ? "yes" : "no");
  PrintPgm(cmd->pgm);
}

/*
 * Name: PrintPgm
 *
 * Description: Prints a list of Pgm:s
 *
 */
void
PrintPgm (Pgm *p)
{
  if (p == NULL) {
    return;
  }
  else {
    char **pl = p->pgmlist;

    /* The list is in reversed order so print
     * it reversed to get right
     */
    PrintPgm(p->next);
    printf("    [");
    while (*pl) {
      printf("%s ", *pl++);
    }
    printf("]\n");
  }
}

/*
 * Name: stripwhite
 *
 * Description: Strip whitespace from the start and end of STRING.
 */
void
stripwhite (char *string)
{
  register int i = 0;

  while (isspace( string[i] )) {
    i++;
  }
  
  if (i) {
    strcpy (string, string + i);
  }

  i = strlen( string ) - 1;
  while (i> 0 && isspace (string[i])) {
    i--;
  }

  string [++i] = '\0';
}
