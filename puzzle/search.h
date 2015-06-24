#ifndef SEARCH_H
#define	SEARCH_H

#include <pthread.h>
#include "list.h"
#include "puzzle.h"

typedef struct {
    list_t *hash_north;
    pthread_mutex_t *lock_north;
    list_t *hash_east;
    pthread_mutex_t *lock_east;
    list_t *hash_south;
    pthread_mutex_t *lock_south;
    list_t *hash_west;
    pthread_mutex_t *lock_west;
    int size;
} search_t;

void create_hash_array(search_t *, piece_list_t *);

int search_piece(piece_list_t *, search_t *, int, int, int, int, piece_t **);

#endif	/* SEARCH_H */

