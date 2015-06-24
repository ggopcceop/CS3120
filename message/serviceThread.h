#ifndef _SERVICETHREAD_H_
#define _SERVICETHREAD_H_

#include "list.h"
#include "helper.h"

/* service only care its name and secret */
typedef struct{
  char service[NAME_MAX];
  char secret[NAME_MAX];
} Services_data;

void *ServiceThread_startThread(void *);

#endif
