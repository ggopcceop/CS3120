/* this file implements the test cases of the program
   to make this file, type: make test 
   to run test case, type: ./test */
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "list.h"
#include "contextSwitch.h"
#include "signalHandler.h"

/* test list.c */
int
test_list(){
  int ok = 1;
  pcb_t *a, *b, *c, *temp, *temp2;
  List_t list;

  /* initialize test data */
  a = (pcb_t *) malloc( sizeof( pcb_t ) );
  strncpy( a->name, "a", NAME_MAX-1 );
  b = (pcb_t *) malloc( sizeof( pcb_t ) );
  strncpy( b->name, "b", NAME_MAX-1 );
  c = (pcb_t *) malloc( sizeof( pcb_t ) );
  strncpy( c->name, "c", NAME_MAX-1 );

  if(!List_init( &list)){
    printf("Test list: Unable to init list\n");
    ok = 0;
  }
  
  /* add data into list */
  List_add_head( &list, (void *)a);
  List_add_tail( &list, (void *)b);
  List_add_tail( &list, (void *)c);

  /* check head */
  List_head_info( &list, (void **)&temp);
  if(temp != a){
    printf("Test list: Head should be node a\n");
    ok = 0;          
  }
  
  /* check next of head */
  temp2 = NULL;
  List_next_node( &list, (void **)&temp2, (void **)&temp);
  List_next_node( &list, (void **)&temp2, (void **)&temp);
  if(temp != b){
    printf("Test list: Next of head should be node b but is %s\n", temp->name);
    ok = 0;
  }
  
  /* check remove head */
  List_remove_head( &list, (void **)&temp);
  if(temp != a){
    printf("Test list: Head should be node a but is %s\n", temp->name);
    ok = 0;
  }

  /* check new head */
  List_head_info( &list, (void **)&temp);
  if(temp != b){
    printf("Test list: Head should be node b but is %s\n", temp->name);
    ok = 0;
  }

  /* deallocate resource */
  free(a);
  free(b);
  free(c);
  
  List_destroy(&list);

  return ok;
}

/* test contextSwitch.c */
int
test_contextSwitch(){
  int ok = 1;
  pcb_t *a, *b, *c, *temp;

  /* initialize test data */
  a = (pcb_t *) malloc( sizeof( pcb_t ) );
  strncpy( a->name, "a", NAME_MAX-1 );
  b = (pcb_t *) malloc( sizeof( pcb_t ) );
  strncpy( b->name, "b", NAME_MAX-1 );
  c = (pcb_t *) malloc( sizeof( pcb_t ) );
  strncpy( c->name, "c", NAME_MAX-1 );

  Switch_init();

  /* add pcbs into ready queue */  
  Switch_add_new_process(a);
  Switch_add_new_process(b);
  Switch_add_new_process(c);

  /* show current queues */
  Switch_Print_State();

  /* test move ready process to running */
  Switch_move_ready_to_running();  
  if((temp = Switch_running_process()) != a){
    printf("Test contextSwitch: running process should be node a but %s\n", 
        temp->name);
    ok = 0;
  }

  /* test move runing to block queue */
  Switch_move_running_to_block();
  if((temp = Switch_blocking_head()) != a){
    printf("Test contextSwitch: head of blocking queue should be node a but %s\n", 
        temp->name);
    ok = 0;
  }

  /* show current queues */
  Switch_Print_State();

  /* test new ready process */
  Switch_move_ready_to_running();  
  if((temp = Switch_running_process()) != b){
    printf("Test contextSwitch: running process should be node b but %s\n", 
        temp->name);
    ok = 0;
  }

  Switch_move_running_to_ready();
  Switch_move_ready_to_running();
  if((temp = Switch_running_process()) != c){
    printf("Test contextSwitch: running process should be node c but %s\n", 
        temp->name);
    ok = 0;
  }

  /* test blocking queue to ready queue */
  Switch_move_blocking_to_ready();
  Switch_move_ready_to_running();
  Switch_move_ready_to_running();
  if((temp = Switch_running_process()) != a){
    printf("Test contextSwitch: running process should be node a but %s\n", 
        temp->name);
    ok = 0;
  }

  /* print final queues */
  Switch_Print_State();

  /* deallocate resource */
  Switch_Deallocate();
  return ok;
}

/* test signalHandler.c */
int
test_signalHandler(){
  int ok = 1;
  pcb_t *a, *b, *c, *temp;

  a = (pcb_t *) malloc( sizeof( pcb_t ) );
  strncpy( a->name, "a", NAME_MAX-1 );
  b = (pcb_t *) malloc( sizeof( pcb_t ) );
  strncpy( b->name, "b", NAME_MAX-1 );
  c = (pcb_t *) malloc( sizeof( pcb_t ) );
  strncpy( c->name, "c", NAME_MAX-1 );

  Switch_init();
  
  Switch_add_new_process(a);
  Switch_add_new_process(b);
  Switch_add_new_process(c);
  
  Signal_handler(SIGALRM);
  if((temp = Switch_running_process()) != a){
    printf("Test signalHandler: running process should be node a but %s\n", 
        temp->name);
    ok = 0;
  }

  Signal_handler(SIGUSR2);
  if((temp = Switch_blocking_head()) != a){
    printf("Test signalHandler: head of blocking queue should be node a but %s\n", 
        temp->name);
    ok = 0;
  }
  if((temp = Switch_running_process()) != NULL){
    printf("Test signalHandler: running process should be empty but has %s\n", 
        temp->name);
    ok = 0;
  }  
  
  Signal_handler(SIGALRM);
  if((temp = Switch_running_process()) != b){
    printf("Test signalHandler: running process should be node b but %s\n", 
        temp->name);
    ok = 0;
  }  

  Signal_handler(SIGUSR1);

  Signal_handler(SIGHUP);

  return ok;
}

/* main function 
   print pass message for each test */ 
int
main(){
  if(!test_list()){ 
    printf("Test list not pass\n");
	return 1;
  }
  if(!test_contextSwitch()){
    printf("Test contextSwitch not pass\n"); 
	return 1;
  }
  if(!test_signalHandler()){
    printf("Test signalHandler not pass\n"); 
	return 1;
  }

  /* test SIGINT
     all resource should be deallocated and quit */
  Signal_handler(SIGINT);

  return 0;
}

