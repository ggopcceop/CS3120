#include <stdlib.h>

#include <stdio.h>
#include "list.h"

/*
 * initialize the list
 */
void
list_init(list_t *list) {
    if (list != NULL) {
        list->head = NULL;
    }
}

/*
 * destory the list and release all nodes
 * do not need to release the data 
 * data will be clean later
 */
void
list_destory(list_t *list) {
    list_node_t *node;
    if (list != NULL) {
        while (list->head != NULL) {
            node = list->head;
            list->head = list->head->next;

            node->data = NULL;
            free(node);
        }
    }
}

/*
 * add a data into list
 */
void
list_add(list_t *list, void *data) {
    list_node_t *node = NULL;
    node = malloc(sizeof (list_node_t));
    if (node != NULL) {
        node->next = list->head;
        node->data = data;

        list->head = node;
    }

}

/*
 * search the same element from two lists
 * set data if found
 */
int
list_search(list_t *list1, list_t *list2, void **data) {
    list_node_t *curr1 = list1->head;
    list_node_t *curr2 = list2->head;
    int found = 0;

    //cross compare two lists
    while (curr1 != NULL && !found) {
        while (curr2 != NULL && !found) {
            if ((curr1->data) == (curr2->data)) {
                *data = curr1->data;
                found = 1;
            }
            curr2 = curr2->next;
        }
        curr1 = curr1->next;
        curr2 = list2->head;
    }

    return found;
}

