/* contextSwitch.c implenments the the simple version of
   context switch. It have ready queue, blocking queue
   and one running process */
#include <stdio.h>
#include <stdlib.h>
#include "contextSwitch.h"
#include "list.h"

const char* State_string(int);

//queues
static List_t readyQueue;
static List_t blockingQueue;
static pcb_t *runningProcess;
static int tick;

/* move new created process into ready queue
   new process should be at the tail */
void
Switch_add_new_process(pcb_t *data){
  if(data != NULL){
    data->state = STATE_READY;
    data->inTick = tick;
  }
  if (List_add_tail( &readyQueue, (void *)data ) == 0) {
    printf ("Error in inserting the process into the list.\n");
  }
}

/* move the current running process to ready queue
   when the running time is exprie. the process should
   go to the head of queue (top priority)*/
void
Switch_move_running_to_ready(){
  printf("%s transition from %s (%d) to %s\n", runningProcess->name,
      State_string(runningProcess->state), tick - runningProcess->inTick, 
      "state ready");
  runningProcess->state = STATE_READY;
  runningProcess->tick = 0;
  runningProcess->inTick = tick;
  if (List_add_tail( &readyQueue, (void *)runningProcess ) == 0) {
    printf ("Error in inserting the process into the list.\n");
  }
  runningProcess = NULL;
}

/* move the head of ready queue to the running state
   when the processer is ready */
void
Switch_move_ready_to_running(){
  pcb_t *temp = NULL;
  if(List_remove_head ( &readyQueue, (void **)&temp ) != 0){
    printf("%s transition from %s (%d) to %s\n", temp->name, 
        State_string(temp->state), tick - temp->inTick
        , "state running");
    temp->state = STATE_RUNNING;
    if(temp->lifetime < temp->runningStateDurtion){
      temp->tick = temp->lifetime;
    } else {
      temp->tick = temp->runningStateDurtion;
    }
    temp->inTick = tick;
    runningProcess = temp;
  } else {
    runningProcess = NULL;
  }
}

/* move the running process to the blocking queue
   process at the head of blocking queue should wait 
   5 time units */
void
Switch_move_running_to_block(){
  if(runningProcess != NULL){
    printf("%s transition from %s (%d) to %s\n", runningProcess->name,
      State_string(runningProcess->state), tick - runningProcess->inTick, 
      "state blocking");
    runningProcess->state = STATE_BLOCKING;
    runningProcess->tick = 5;
    runningProcess->inTick = tick;
    if (List_add_tail( &blockingQueue, (void *)runningProcess ) == 0) {
      printf ("Error in inserting the process into the list.\n");
    }
    runningProcess = NULL;
  } else {
    printf("Can not block current process, no process is running\n");
  }
}

/* move the blocking process back to the ready queue
   the process should be at the tail */
void
Switch_move_blocking_to_ready(){
  pcb_t *temp;
  List_remove_head(&blockingQueue, (void **)&temp);
  if(temp != NULL){
    printf("%s transition from %s (%d) to %s\n", temp->name, 
        State_string(temp->state), tick - temp->inTick, "state ready");
    temp->state = STATE_READY;
    temp->inTick = tick;
    if (List_add_tail( &readyQueue, (void *)temp ) == 0) {
      printf ("Error in inserting the process into the list.\n");
    }
  }
}

/* free running process when ends */ 
void
Switch_move_running_to_finish(){
  if(runningProcess != NULL){
    free(runningProcess);
    runningProcess = NULL;
  }
  else{
    printf("Can not finish running process, no process is running\n");
  }  
}

/* print all the current queues */
void
Switch_Print_State(){
  pcb_t *temp = NULL, *temp2 = NULL;
  int time = 0, tTime = 0;
  if(runningProcess != NULL){
    printf("running process: %s\n", runningProcess->name);
    tTime = runningProcess->tick;
  } else {
    printf("running process:\n");
  }
  
  temp=NULL;
  time=tTime;
  printf("ready queue:\n");
  List_next_node( &readyQueue, (void **)&temp, (void **)&temp2);
  while( temp2 != NULL){
    printf("\t%s %d\n", temp2->name, time);
    time+=temp2->runningStateDurtion;
    List_next_node( &readyQueue, (void **)&temp, (void **)&temp2);
  }

  temp=NULL;
  time=0;
  printf("blocking queue:\n");
  List_next_node( &blockingQueue, (void **)&temp, (void **)&temp2);
  while( temp2 != NULL){
    time+=temp2->tick;
    printf("\t%s %d\n", temp2->name, time);    
    List_next_node( &blockingQueue, (void **)&temp, (void **)&temp2);
  }
}

/* get the current running process */
pcb_t
*Switch_running_process(){
  return runningProcess;
}

/* get the process at the head of blocking queue */
pcb_t 
*Switch_blocking_head(){
  pcb_t *temp;
  List_head_info ( &blockingQueue , (void **)&temp );
  return temp;
}

void
Switch_update_tick(){
  tick++;
}
/* initialize all the queues */
void
Switch_init(){
  if(!List_init( &readyQueue )){
    printf("Unable to initialize ready queue\n");
  }
  if(!List_init( &blockingQueue )){
    printf("Unable to initialize blockingQueue queue\n");
  }  
  runningProcess = NULL;
  
  tick = 0;
}

/* deallocate all the queues and data */
void
Switch_Deallocate(){
  List_destroy( &readyQueue);
  List_destroy( &blockingQueue);
  
  if(runningProcess != NULL){
    free(runningProcess);
  }
}

/* get string of each state */
const char*
State_string(int state){
  switch(state){
    case STATE_READY:
      return "state ready";
    case STATE_RUNNING:
      return "state running";
    case STATE_BLOCKING:
      return "state blocking";
    default:
      return "";
  }
}

