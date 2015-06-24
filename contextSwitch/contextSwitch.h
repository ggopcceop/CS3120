#ifndef _contextSwitch_h_
#define _contextSwitch_h_

#define BUF_MAX  (81)
#define NAME_MAX (21)

#define STATE_READY 0
#define STATE_RUNNING 1
#define STATE_BLOCKING 3

//process control block
typedef struct {
  char name[NAME_MAX];
  int  lifetime;
  int  runningStateDurtion;
  int  tick;
  int  state;
  int  inTick;
} pcb_t;

//functions to move process control block btween queues
void Switch_add_new_process(pcb_t *);
void Switch_move_running_to_ready(void);
void Switch_move_ready_to_running(void);
void Switch_move_running_to_block(void);
void Switch_move_blocking_to_ready(void);
void Switch_move_running_to_finish(void);

//helper funtions
pcb_t *Switch_running_process(void);
pcb_t *Switch_blocking_head(void);
void Switch_update_tick(void);
void Switch_Print_State(void);
void Switch_init(void);
void Switch_Deallocate(void);

#endif
