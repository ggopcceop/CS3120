/* serviceThread.c file implements service threads that 
checks that can do calculations. if the message has the right 
secret value, return the result back to the sender */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pmessages.h"
#include "serviceThread.h"
#include "helper.h"

/* function handle message from client thread then return a message */
void handleServiceMessage(char *, int, char *, char *, pthread_t);

/* main function for serviceThread */
void
*ServiceThread_startThread(void * param){  
  char secret[NAME_MAX], service[NAME_MAX], *msg;
  Services_data *data = (Services_data *)param;
  pthread_t receive;
  int size;
  
  //use get the service name and its secret value
  strncpy(service, data->service, NAME_MAX - 1);
  strncpy(secret, data->secret, NAME_MAX - 1);
  service[NAME_MAX - 1] = '\0';
  secret[NAME_MAX - 1] = '\0';
  
  //free data, not going to use anymore
  free(data);
  
  /*keep receiving message, for every message that recivesd
    return a message to the sender */
  while(1){
    if (receive_message( &receive, &msg, &size) == MSG_OK) {
      printf ("%s Service Thread received message --%s--size %d\n", service, msg, size );
      //handle message string, return a message at the end
      handleServiceMessage(msg, size, secret, service, receive); 
    } else {
      printf ("first receive failed\n");
    }	
  }
  
}

void
handleServiceMessage(char *msg, int size, char *secret, char *service, pthread_t receive){
  char buffer[BUF_MAX], *name, *serviceName, *param1, *param2, *secretStr, *str_ptr = NULL;
  int value1, value2, result;
  
  /* user strtok to split come message */
  name = strtok_r(msg, ":", &str_ptr);
  serviceName = strtok_r(NULL, ":", &str_ptr);
  param1 = strtok_r(NULL, ":", &str_ptr);
  param2 = strtok_r(NULL, ":", &str_ptr);
  secretStr = strtok_r(NULL, ":", &str_ptr);
  
  //if service and secret is mathing, do the calculation
  if((strcmp(service, serviceName) == 0) && (strcmp(secret, secretStr) == 0 )){
    //parse char to integer value
    sscanf(param1, "%d", &value1);
    sscanf(param2, "%d", &value2);
	
	//do the calculation base on the service type
    if(strcmp(serviceName, "add") == 0){
      result = value1 + value2;	  
    } else if(strcmp(serviceName, "sub") == 0){
      result = value1 - value2;	 
    } else if(strcmp(serviceName, "mult") == 0){
      result = value1 * value2;	 
    }
    
    //build the return string with the result
    sprintf(buffer, "%d:%s:%d", 1, serviceName, result);
  } else {
    //build the fail string
    sprintf(buffer, "%d:%s:%d", 0, service, 0);
  }
  
  //free the come message
  free(msg);
  
  //send return message back to the sender
  if (send_message_to_thread(receive, buffer, strlen(buffer)+1) != MSG_OK) {
    printf("%s Service Thread: return message failed\n", service);
  }
}

