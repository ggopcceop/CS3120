#ifndef _AUTHTHREAD_H_
#define _AUTHTHREAD_H_

#include "helper.h"

/* struct to store authentication infomation
   for each user */
typedef struct{
  char name[NAME_MAX];
  char pass[NAME_MAX];
} AuthUser_t;

/* struct use to pass value when creating auth
   thead */
typedef struct{
  char authfile[NAME_MAX];
  char secret[NAME_MAX];
} Auth_data;

void *AuthThread_startThread(void *);

#endif
