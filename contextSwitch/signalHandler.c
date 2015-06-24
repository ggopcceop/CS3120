/* signalHandler.c handles all the signal part
   of this system */
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "signalHandler.h"
#include "contextSwitch.h"

void Signal_handler(int);
void Sorttrem_handler(void);

static int timer;  //the length of one time until

/* read the config file and the register all the 
   signal that used in this system */
void Register_signal(void){
  Read_config();

  if (signal(SIGALRM, Signal_handler) == SIG_ERR) {
    printf("Unable to register SIGALRM.\n");
  }
  if (signal(SIGUSR1, Signal_handler) == SIG_ERR) {
    printf("Unable to register SIGUSR1.\n");
  }
  if (signal(SIGUSR2, Signal_handler) == SIG_ERR) {
    printf("Unable to register SIGUSR2.\n");
  }
  if (signal(SIGHUP, Signal_handler) == SIG_ERR) {
    printf("Unable to register SIGHUP.\n");
  }
  if (signal(SIGINT, Signal_handler) == SIG_ERR) {
    printf("Unable to register SIGINT.\n");
  }
  
  alarm(1);
}

/* read config from file config.txt */
void
Read_config(){
  char buffer[BUF_MAX];
  FILE *configFile;
  if((configFile = fopen("config.txt", "r")) != NULL){
    if(fgets(buffer, BUF_MAX-1, configFile)){
      if(sscanf(buffer, "timer=%d", &timer)!=1){
        printf("Format of config file must be timer=X.\n");
      } else {
        printf("Set timer to %d.\n", timer);
      }   
    } else {
      printf("Config file read error.\n");
    }
  } else {
    printf("Unable to open config file.\n");
  }
  fclose(configFile);
}

/* handler that handle all the signals */
void
Signal_handler(int signal){
  switch(signal){
    case SIGALRM:
      Sorttrem_handler();
	  //timer should >= 1
      if(timer > 0){
        alarm(timer);
      }
      break;
    case SIGUSR1:
      Switch_Print_State();
      break;
    case SIGUSR2:
      Switch_move_running_to_block();
      break;
    case SIGHUP:
      Read_config();
      if(timer > 0){
        alarm(0);
      }
      break;
    case SIGINT:
      printf("deallocating resource and close the simulater\n");
      Switch_Deallocate();
      exit(0);    
      break;
    default:
      break;
  }
}

/* sorttrem handler that move process between
   queues */
void
Sorttrem_handler(){
  pcb_t *process;
  int lifetime, tick;
  
  Switch_update_tick();
  
  /* handle running process */
  process = Switch_running_process();
  if(process != NULL){
    lifetime = (process->lifetime--);
    tick = (process->tick--);
    if(lifetime > 1){
      if(tick <= 1){
        process->tick = 0;
        Switch_move_running_to_ready();
      }
    } else {
      Switch_move_running_to_finish();
    }
  }
  
  /* handle blocking queue */
  process = Switch_blocking_head();
  if(process != NULL){
    //tick the head of blocking queue
    tick = (process->tick--);
    if(tick <= 1) {
      /* if the process is done
         move black to ready queue */
      Switch_move_blocking_to_ready();
    }
  }

  /* if nothing is running, move head of ready queue
     to run state */
  process = Switch_running_process();
  if(process == NULL){
    Switch_move_ready_to_running();
  }
}
