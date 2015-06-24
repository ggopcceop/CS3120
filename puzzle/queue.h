#ifndef QUEUE_H
#define	QUEUE_H

typedef struct node_t {
    struct node_t *next;
    void *data;
} node_t;

typedef struct {
    node_t *head;
    node_t *last;
    int size;
    pthread_mutex_t lock;
    sem_t block;
} queue_t;

void queue_init(queue_t*);
void queue_push(queue_t*, void*);
int queue_pop(queue_t*, void**);

#endif	/* QUEUE_H */

