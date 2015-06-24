#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "pmessages.h"
#include "clientThread.h"
#include "helper.h"

int authUser(pthread_t *, char *, char *, char *);
int grantTicket(pthread_t *, char *, char *, char *);
int askService(pthread_t *, char *, char *, char *, int, int, int *);

void
*ClientThread_startThread(void *param){
  pthread_t *authThread, *ticketThread, *serviceThread;
  pthread_t *addThread, *subThread, *multThread;
  Client_data *clientData = (Client_data *)param;
  char buffer[BUF_MAX], name[NAME_MAX], pass[NAME_MAX], secret[NAME_MAX];
  char serviceName[NAME_MAX];
  int a, b, result;
  int state = NON_AUTH;
  
  authThread = clientData->authThread;
  ticketThread = clientData->ticketThread;
  addThread = clientData->addThread;
  subThread = clientData->subThread;
  multThread = clientData->multThread;
  
  free(clientData);
  
  while(1){
    if(fgets(buffer, BUF_MAX-1, stdin)){
      switch(state){
        case NON_AUTH:
          if(sscanf(buffer, "%s %s\n", name, pass) == 2){
            if(!authUser(authThread, name, pass, secret)){
              printf("wrong username or passworld\n");
            } else {
              state = AUTHED;
              printf("you are logined!\n");
            }
          } else {
            printf("Please enter \"username password\" \n");
          }
          break;
        case AUTHED:
          if(sscanf(buffer, "%s %d %d\n", serviceName, &a, &b) == 3){
            if(!grantTicket(ticketThread, secret, name, serviceName)){
              printf("can not giant tick for the service %s\n", serviceName);
            } else {
              switch(serviceName[0]){
                case 'a':
                  serviceThread = addThread;
                  break;
                case 's':
                  serviceThread = subThread;
                  break;
                case 'm':
                  serviceThread = multThread;
                  break;
                default:
                  printf("unknow service name\n");
                  break;
              }
              if(!askService(serviceThread, secret, name, serviceName, a, b, &result)){
                printf("can not ask service %s\n", serviceName);
              }
              else{
                printf("Answer %d %s %d is %d\n", a, serviceName, b, result);
              }
            }
          } else {
            printf("Please enter \"service_name p1 p2\" \n");
          }
          break;
        default:
          break;        
      }
    }
  }
  
  return NULL;
}

int
authUser(pthread_t *authThread, char *name, char *pass, char *secret){
  char buffer[BUF_MAX], *comeMSG, *code, *comeName, *secretStr, *str_ptr = NULL;
  int size, returnCode = 0;
  pthread_t receive;
  
  sprintf(buffer, "%s:%s", name, pass);
  if (send_message_to_thread( *authThread, buffer, strlen(buffer)+1) != MSG_OK) {
    printf( "fail to send auth message to auth thread\n" );
  } else { 
    if (receive_message( &receive, &comeMSG, &size) != MSG_OK) {
      printf ("can not receive message from auth thread \n");
    } else {
      printf("Client Thread receive message --%s-- size %d\n", comeMSG, size);
      code = strtok_r(comeMSG, ":", &str_ptr);
      if(strlen(code) > 0 && code[0] == '1'){
        comeName = strtok_r(NULL, ":", &str_ptr);
        secretStr = strtok_r(NULL, ":", &str_ptr);
        if(strcmp(comeName, name) == 0){
          strncpy(secret, secretStr, NAME_MAX-1);
          secret[NAME_MAX-1] = '\0';
          returnCode = 1;
        }
      }
      free(comeMSG);
    }
  }
  return returnCode;
}

int
grantTicket(pthread_t *ticketThread, char *secret, char *name, char *serviceName){
  char buffer[BUF_MAX], *comeMSG, *code, *comeName, *comeService, *secretStr, *str_ptr = NULL;
  int size, returnCode = 0;
  pthread_t receive;
  
  sprintf(buffer, "%s:%s:%s", name, serviceName, secret);
  if (send_message_to_thread( *ticketThread, buffer, strlen(buffer)+1) != MSG_OK) {
    printf( "fail to send message to ticket thread\n" );
  } else {
    if (receive_message( &receive, &comeMSG, &size) != MSG_OK) {
	  printf ("can not receive message from ticket thread \n");
	} else {
      printf("Client Thread receive message --%s-- size %d\n", comeMSG, size);
      code = strtok_r(comeMSG, ":", &str_ptr);
      if(strlen(code) > 0 && code[0] == '1'){
        comeName = strtok_r(NULL, ":", &str_ptr);
        comeService = strtok_r(NULL, ":", &str_ptr);
        secretStr = strtok_r(NULL, ":", &str_ptr);
        if((strcmp(comeName, name) == 0) && (strcmp(comeService, serviceName) == 0)){
          strncpy(secret, secretStr, NAME_MAX-1);
          secret[NAME_MAX-1] = '\0';
          returnCode = 1;
        }
      }
      free(comeMSG);
    }
  }
  return returnCode;
}

int
askService(pthread_t *serviceThread, char *secret, char *name, 
    char *serviceName, int value1, int value2, int *result){
  char buffer[BUF_MAX], *comeMSG, *code, *comeService, *answer, *str_ptr = NULL;
  int size, returnCode = 0;
  pthread_t receive;
  
  sprintf(buffer, "%s:%s:%d:%d:%s", name, serviceName, value1, value2, secret);
  
  if (send_message_to_thread( *serviceThread, buffer, strlen(buffer)+1) != MSG_OK) {
    printf( "fail to send message to service %s thread\n", serviceName);
  } else {
    if (receive_message( &receive, &comeMSG, &size) != MSG_OK) {
	  printf ("can not receive message from ticket thread \n");
	} else {
      printf("Client Thread receive message --%s-- size %d\n", comeMSG, size);
      code = strtok_r(comeMSG, ":", &str_ptr);
      if(strlen(code) > 0 && code[0] == '1'){
        comeService = strtok_r(NULL, ":", &str_ptr);
        answer = strtok_r(NULL, ":", &str_ptr);
        if(strcmp(comeService, serviceName) == 0){
          sscanf(answer, "%d", result);
          returnCode = 1;
        }
      }
    }
  }
  return returnCode;
}
