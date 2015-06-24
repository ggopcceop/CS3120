#ifndef LIST_H
#define	LIST_H

typedef struct list_node_t{
    struct list_node_t *next;
    void *data;
} list_node_t;

typedef struct {
    list_node_t *head;
} list_t;

void list_init(list_t *);

void list_destory(list_t *);

void list_add(list_t *, void *);

int list_search(list_t *, list_t *, void **);

#endif	/* LIST_H */

