/* authThread.c file implements authentication 
thread that checks username and password
then return ticket secret to the user */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "pmessages.h"
#include "authThread.h"
#include "list.h"

/* function read authentication file */
List_t readAuth(char *);
/* function handle message from client thread then return a message */
void handleAuthMessage(char *, int, char *, List_t, pthread_t);

/* main function for authThread */
void
*AuthThread_startThread(void *param){
  Auth_data *data = (Auth_data *)param;
  char secret[NAME_MAX], *msg;  
  List_t users;
  int size;
  pthread_t receive;
  
  //read authentication file and store in a list
  users = readAuth(data->authfile);
  
  //read ticket secret value
  strncpy( secret, data->secret, NAME_MAX-1 );
  secret[NAME_MAX-1]='\0';
  
  //free data, not use anymore
  free(data);
  
  /*keep receiving message, for every message that recivesd
    return a message to the sender */
  while(1){
    if(receive_message( &receive, &msg, &size) == MSG_OK) {
      printf("Auth Thread receive message --%s-- size %d\n", msg, size );
      //handle message string, return a message at the end
	  handleAuthMessage(msg, size, secret, users, receive);
    } else {
      printf("Auth Thread receive failed\n");
    }
  }
  
  /* free the users list in the end */
  free_list(&users);
  
  return NULL;
}

/* read authentication file */
List_t
readAuth(char *authFile){
  char buffer[BUF_MAX], *name, *pass, *str_ptr = NULL;
  FILE *file;
  AuthUser_t *temp;
  List_t users;

  //initialize users list
  if(!List_init(&users)){
    printf("Unable to initialize list for user\n");
  } else {
    printf("reading auth file\n");
    if((file = fopen(authFile, "r")) != NULL){
      while(fscanf(file, "%s\n", &buffer[0]) != EOF){
        //use strtok to split username and password
        str_ptr = NULL;
        name = strtok_r(buffer, ":", &str_ptr);
        pass = strtok_r(NULL, ":", &str_ptr);
		
        temp = malloc(sizeof(AuthUser_t));
        strncpy( temp->name, name, NAME_MAX-1 );
        strncpy( temp->pass, pass, NAME_MAX-1 );
        temp->name[NAME_MAX-1] = '\0';
        temp->pass[NAME_MAX-1] = '\0';
		
		//store username and password into the users list
        List_add_tail(&users, temp);
      }
    } else {
      printf("Unable to open config file.\n");
    }
    fclose(file);    
  }
  return users;
}

/* handle messages that send to this thread */
void
handleAuthMessage(char *msg, int size, char *secret, List_t users, pthread_t receive){
  char *name, *pass, buffer[BUF_MAX], *str_ptr = NULL;
  void *context = NULL;
  AuthUser_t *user;
  int found = 0;
  
  /* use strtok to split message to username and password */
  name = strtok_r(msg, ":", &str_ptr);
  pass = strtok_r(NULL, ":", &str_ptr);

  /* check username and password with the data in the list */
  while ( List_next_node( &users, &context, (void *)&user ) && (user != NULL)) {
    //if ther is a data math
    if(strcmp(user->name, name) == 0 && strcmp(user->pass, pass) == 0){
      //construct the return string 
      sprintf(buffer, "%d:%s:%s", 1, name, secret);
      found = 1;
      break;
    }
  }
  
  //if there is not mathing in the list
  if(!found){
    //construct the fail return string 
    sprintf(buffer, "%d:%s:%d", 0, name, 0);
  }
  
  //free the msg string
  free(msg);
  
  //send the return message back to the sender
  if (send_message_to_thread(receive, buffer, strlen(buffer)+1) != MSG_OK) {
    printf( "Auth Thread: return message failed\n" );
  }
}

