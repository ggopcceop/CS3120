/* test.c is the main file of the program
  it starts all threads that used in communication */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pmessages.h"
#include "list.h"
#include "authThread.h"
#include "serviceThread.h"
#include "ticketGrantingThread.h"
#include "clientThread.h"

// function that reads service file
Ticket_data *readService(char *);

/* functions that start the threads */
void createAuthThread(pthread_t *, char *);
void createTicketThread(pthread_t *, char *, Ticket_data *);
void createServiceThread(pthread_t *, pthread_t *, pthread_t *, Ticket_data *);
void createClientThread(pthread_t *, pthread_t *, pthread_t *,
    pthread_t *, pthread_t *, pthread_t *, List_t *);

//ticket secret that shared with auth thread and ticket thread
static char *tickSecret = "aaabbbddsd";

/* main function */
int 
main(int argc, char **argv ){
  pthread_t clientThread, authThread, ticketThread, addThread, subThread, multThread;  
  Ticket_data *tickData;
  
  //make sure user input the correct run command
  if(argc != 3){
    printf("plase type ./test auth_filename service_filename\n");
  } else {
    //read the service file, and store into tickData
    tickData = readService(argv[2]);
    
    //initialize message passing system
    if (messages_init() != MSG_OK) {
      printf("Unable to initialize message system\n");
    } else {
      /* create threads */
      createAuthThread(&authThread, argv[1]);  
      createTicketThread(&ticketThread, tickSecret, tickData);
      createServiceThread(&addThread, &subThread, &multThread, tickData);
      createClientThread(&clientThread, &authThread, &ticketThread, 
          &addThread, &subThread, &multThread, tickData->availableServices);
  
      /* wait threads to complete */
      pthread_join(authThread, NULL);
      pthread_join(ticketThread, NULL);
      pthread_join(addThread, NULL);
      pthread_join(subThread, NULL);
      pthread_join(multThread, NULL);
      pthread_join(clientThread, NULL);
      
      // end the message system
      messages_end();
    }
  }
  return 0;
}

/* function to read service file */
Ticket_data 
*readService(char *serviceFile){
  char buffer[BUF_MAX], *service, *secret, *users, *user, *temp, *data;
  char *str_ptr = NULL;
  FILE *file;
  
  List_t *availableServices;
  List_t *secrets;
  List_t *userList;
  List_t *tempList;
  
  Ticket_data *ticketData;
  
  /* initialize 3 main lists to store the information */
  availableServices = malloc(sizeof(List_t));
  secrets = malloc(sizeof(List_t));
  userList = malloc(sizeof(List_t));
  
  List_init(availableServices);
  List_init(secrets);
  List_init(userList);
  
  /* read the file */
  printf("reading service file\n");
  if((file = fopen(serviceFile, "r")) != NULL){
    while(fscanf(file,"%s\n", &buffer[0]) != EOF){
      /* for each line, user strtok to split string */
      str_ptr = NULL;
      service = strtok_r(buffer, ":", &str_ptr);
      secret = strtok_r(NULL, ":", &str_ptr);
      users = strtok_r(NULL, ":", &str_ptr);
	  
	  /* put services string into service list */
      data = malloc(sizeof(char) * NAME_MAX);
      strncpy( data, service, NAME_MAX-1 );
      data[NAME_MAX-1] = '\0';
      List_add_tail(availableServices, data);
	  
	  /* put secrets string into secret list */
      data = malloc(sizeof(char) * NAME_MAX);
      strncpy( data, secret, NAME_MAX-1 );
      data[NAME_MAX-1] = '\0';
      List_add_tail(secrets, data);
	  
	  /* put users lists into users list */
      tempList = malloc(sizeof(List_t));
      List_init(tempList);
  
      user = strtok(users, ",");
      while(user != NULL){
        temp = malloc(sizeof(char) * NAME_MAX);
        strncpy(temp, user, NAME_MAX-1 );
        temp[NAME_MAX-1] = '\0';
		
        List_add_tail(tempList, temp);
        user = strtok(NULL, ",");
      }
      List_add_tail(userList, tempList);
    }
  } else {
    printf("Unable to open config file.\n");
  }
  fclose(file);  
  
  /* put lists into tickData stuct */
  ticketData = malloc(sizeof(Ticket_data));
  ticketData->availableServices = availableServices;
  ticketData->secrets = secrets;
  ticketData->users = userList;
  
  return ticketData;
}

/* create authentication thread */
void
createAuthThread(pthread_t *authThread, char *authfile){
  Auth_data *authData;
  
  /* pass authentication file name and ticket secret value
     to the authThread */
  authData = malloc(sizeof(Auth_data));
  strncpy(authData->authfile, authfile, NAME_MAX-1);
  strncpy(authData->secret, tickSecret, NAME_MAX-1);
  authData->authfile[NAME_MAX-1] = '\0';
  authData->secret[NAME_MAX-1] = '\0';
  
  // start the thread
  if(pthread_create(authThread, NULL, AuthThread_startThread, authData)) {
    printf("Error creating thread\n");
  }
}

/* create ticket granting thread */
void
createTicketThread(pthread_t *ticketThread, char *secret, Ticket_data *ticketData){  
  /* pass ticket secret and 3 lists to the ticketThread */
  strncpy( ticketData->secret, secret, NAME_MAX-1 );
  ticketData->secret[NAME_MAX-1] = '\0';
  
  // start the thread
  if(pthread_create(ticketThread, NULL, TicketThread_startThread, ticketData)) {
    printf("Error creating ticket thread\n");
  }
}

/* create service threads */
void
createServiceThread(pthread_t *addThread, pthread_t *subThread, pthread_t *multThread, 
    Ticket_data *ticketData){
  char *serviceStr, *secretStr;
  List_t *availableServices = ticketData->availableServices;
  List_t *secrets = ticketData->secrets;
  void *context1 = NULL, *context2 = NULL;
  Services_data *data;
  pthread_t *st;
  
  // each service need thread to run
  while ( List_next_node( availableServices, &context1, (void *)&serviceStr ) 
      && List_next_node( secrets, &context2, (void *)&secretStr )
      && (serviceStr != NULL)) {
    /* pass the correct thread address */
    if(strcmp(serviceStr, "add") == 0){
      st = addThread;	  
    } else if(strcmp(serviceStr, "sub") == 0){
      st = subThread;	
    } else if(strcmp(serviceStr, "mult") == 0){
      st = multThread;	
    }
    
    //pass the service name and service secret to the thread
    data = malloc(sizeof(Services_data));
    strncpy(data->service, serviceStr, NAME_MAX-1);
    strncpy(data->secret, secretStr, NAME_MAX-1);
	
    data->service[NAME_MAX-1] = '\0';
    data->secret[NAME_MAX-1] = '\0';
	
	// start the thread
    if(pthread_create(st, NULL, ServiceThread_startThread, data)) {
      printf("Error creating %s service thread\n", serviceStr);
    }
  }
}

void
createClientThread(pthread_t *clientThread, pthread_t *authThread, pthread_t *ticketThread,
    pthread_t *addThread, pthread_t *subThread, pthread_t *multThread, List_t *availableServices){
  Client_data *clientData;
  
  /* pass all others thread address to the client thread
     for message sending */
  
  clientData = malloc(sizeof(Client_data));
  clientData->authThread = authThread;
  clientData->ticketThread = ticketThread;
  clientData->addThread = addThread;
  clientData->subThread = subThread;
  clientData->multThread = multThread;
  clientData->availableServices = availableServices;
  
  // start the thread
  if(pthread_create(clientThread, NULL, ClientThread_startThread, clientData)) {
    printf("Error creating client thread\n");
  }
  
}


