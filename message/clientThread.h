#ifndef _CLIENTTHREAD_H_
#define _CLIENTTHREAD_H_

#include <pthread.h>
#include "list.h"

/* two client state */
#define NON_AUTH 0
#define AUTHED 1

/* struct to pass all other thread addresss */
typedef struct {
  pthread_t *authThread;
  pthread_t *ticketThread;
  pthread_t *addThread;
  pthread_t *subThread;
  pthread_t *multThread;
  List_t *availableServices;
} Client_data;

void *ClientThread_startThread(void *);

#endif
