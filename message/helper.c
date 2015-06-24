/* helper.c implements some helper function */
#include <stdlib.h>
#include "helper.h"

/* free a list, remove and free all data inside the lsit */
void
free_list(List_t *list){
  void *data;
  while(List_remove_head (list, &data) && (data != NULL)){
    free(data);
  }
  List_destroy(list);
}
