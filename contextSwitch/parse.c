/* this is the main file of the program
   it initialize the program and contians the input 
   handler for any input or command from user */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include "list.h"
#include "contextSwitch.h"
#include "signalHandler.h"

void init(void);
void readConfig(void);
void inputHandler(void);

/* Initialize the program */
void
init(){
  Switch_init();
  Register_signal();
}

/* input handler to handle all the input and command of
   the program. there is help command to print all aviable 
   commands */
void
inputHandler(){
  pcb_t  *pcb;
  char buffer[BUF_MAX];
  char processName[NAME_MAX];
  int lifeTime, runningStateDurtion;
  while(1){
    if(fgets(buffer, BUF_MAX-1, stdin)){
      if(sscanf(buffer, "%s %d %d", processName, &lifeTime,  
          &runningStateDurtion) == 3){
        if(runningStateDurtion > lifeTime){
          printf("Durtion is larger than lifetime\n");
		  continue;
        }
        pcb = (pcb_t *) malloc( sizeof( pcb_t ) );
        if (pcb != NULL) {
          strncpy( pcb->name, processName, NAME_MAX-1 );
          pcb->name[NAME_MAX] = '\0'; /* Make sure that it is null-terminated. */
          pcb->lifetime = lifeTime;
          pcb->runningStateDurtion = runningStateDurtion;
          printf ("Read and stored process %s with lifetime %d and each durtion of %d\n"
              , pcb->name, pcb->lifetime,pcb->runningStateDurtion);
          Switch_add_new_process(pcb);
        } else {
          printf("Unable to allocate memery.\n");
        }
      } else if(strncmp("list", processName, NAME_MAX) == 0){
        //if command is list, send SIGUSR1 signal to the program
        kill((int) getpid(), SIGUSR1);
      } else if(strncmp("block", processName, NAME_MAX) == 0){
        //if command is block, send SIGUSR2 signal to the program
        kill((int) getpid(), SIGUSR2);
      } else if(strncmp("config", processName, NAME_MAX) == 0){
        //if command is config, send SIGHUP signal to the program
        kill((int) getpid(), SIGHUP);
      } else if(strncmp("help", processName, NAME_MAX) == 0){
        //print all aviable commands
        printf("processname total_duration running_state_duration - add a process into ready queue\n");
        printf("list - list all current queues\n");
        printf("block - block current running process\n");
        printf("config - re-read the config file\n");
      } else {
        printf("Unkonwn input. type \"help\"\n");
      }
    } else {
      printf("No reading\n");
    }
  }
}

int
main( int argc, char **argv ) {
  init();
  inputHandler();

  return 0;
}

