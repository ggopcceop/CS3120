#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

/*
 * initialize the queue
 */
void
queue_init(queue_t *queue) {
    queue->head = NULL;
    queue->last = NULL;
    queue->size = 0;
    pthread_mutex_init(&queue->lock, NULL);
    sem_init(&queue->block, 0, 0);
}

/*
 * thread safe put a data into the queue
 */
void
queue_push(queue_t *queue, void *data) {
    node_t *node;
    node = malloc(sizeof (node_t));
    node->next = NULL;

    node->data = data;

    pthread_mutex_lock(&queue->lock);
    if (queue->last != NULL) {
        queue->last->next = node;
        queue->last = node;
    } else {
        queue->head = node;
        queue->last = node;
    }
    queue->size++;
    
    sem_post(&queue->block);

    pthread_mutex_unlock(&queue->lock);
}

/*
 * thread safe pop a data from queue
 */
int
queue_pop(queue_t *queue, void **data) {
    node_t *node;
    int return_code;
    
    sem_wait(&queue->block);

    pthread_mutex_lock(&queue->lock);

    if (queue->size > 0) {
        node = queue->head;
        queue->head = node->next;

        if (queue->head == NULL) {
            queue->last = NULL;
        }
        queue->size--;

        *data = node->data;
        free(node);

        return_code = 1;
    } else {
        return_code = 0;
    }
    pthread_mutex_unlock(&queue->lock);

    return return_code;

}
