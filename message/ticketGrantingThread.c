/* authThread.c file implements ticket granting 
thread that checks user's permision if can run a serviceName
if the user has permision to run the service, return the 
service's secret value */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pmessages.h"
#include "ticketGrantingThread.h"
#include "helper.h"

/* function handle message from client thread then return a message */
void handleTicketMessage(char *, int, char *, List_t *, 
    List_t *, List_t *, pthread_t);

/* main function for ticketGrantingThread */
void
*TicketThread_startThread(void *param){
  Ticket_data *data = (Ticket_data *)param;
  char secret[NAME_MAX], *msg;
  int size;
  pthread_t receive;
  List_t *availableServices, *secrets, *users;
  
  //read ticket secret value
  strncpy( secret, data->secret, NAME_MAX-1 );
  secret[NAME_MAX-1]='\0';
  
  //get all the available services
  availableServices = data->availableServices;
  //get all services' secret
  secrets = data->secrets;
  //get all users that can run services
  users = data->users;
  
  //free the data, not using anymore
  free(data);
  
  /*keep receiving message, for every message that recivesd
    return a message to the sender */
  while(1){
    if (receive_message( &receive, &msg, &size) == MSG_OK) {
      printf ("Ticket Thread received message --%s--size %d\n", msg, size );
      //handle message string, return a message at the end
      handleTicketMessage(msg, size, secret, availableServices, secrets, users, receive);
    } else {
      printf ("first receive failed\n");
    }	
  }
  
  /* free all the list in the end */
  free_list(availableServices);
  free_list(secrets);
  free_list(users);
  
  return NULL;
}

/* handle messages that send to this thread */
void
handleTicketMessage(char *msg, int size, char *secret, List_t *availableServices,
    List_t *secrets, List_t *users, pthread_t receive){
  char *name, *serviceName, *authSecret, buffer[BUF_MAX];
  char *serviceStr, *secretStr, *userStr, *str_ptr=NULL;
  List_t *userList;
  void *context1 = NULL, *context2 = NULL, *context3 = NULL, *context4 = NULL;
  int found = 0;
  
  /* user strtok to split come message */
  name = strtok_r(msg, ":", &str_ptr);
  serviceName = strtok_r(NULL, ":", &str_ptr);
  authSecret = strtok_r(NULL, ":", &str_ptr);
  
  /* iterate all list at once, all service name, secret and users and store in
     the same order and the same size */
  while ( List_next_node( availableServices, &context1, (void *)&serviceStr ) 
      && List_next_node( secrets, &context2, (void *)&secretStr )
      && List_next_node( users, &context3, (void *)&userList )
      && (serviceStr != NULL) && !found) {
    //if the service name is mathing, check the user if is in the list
    if(strcmp(serviceStr, serviceName) == 0){
      context4 = NULL;
      while(List_next_node( userList, &context4, (void *)&userStr) 
          && (userStr != NULL) && !found){
        //if the user is in the list
        if(strcmp(userStr, name) == 0){
          found = 1;
          //construct the return string
          sprintf(buffer, "%d:%s:%s:%s", 1, name, serviceName, secretStr);
        }
      }
    }
  }
  
  //construct the fail return string if not found
  if(!found){
    sprintf(buffer, "%d:%s:%s:%d", 0, name, serviceName, 0);
  }

  //free the coming message
  free(msg);
  
  //send the return string back to the sender
  if (send_message_to_thread(receive, buffer, strlen(buffer)+1) != MSG_OK) {
    printf( "Ticket Thread: return message failed\n" );
  }
  
}

