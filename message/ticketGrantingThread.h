#ifndef _TICKETGRANTINGTHREAD_H_
#define _TICKETGRANTINGTHREAD_H_

#include "list.h"
#include "helper.h"

/* list of all service name, secret and users address
   its ticket secret value */
typedef struct{
  char secret[NAME_MAX];
  List_t *availableServices;
  List_t *secrets;
  List_t *users;
} Ticket_data;

void *TicketThread_startThread(void *);

#endif
