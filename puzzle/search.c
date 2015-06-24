/* 
 * create 4 hash tables for fast concurrenct search for pieces
 *  */
#include <stdlib.h>

#include "search.h"
#include "puzzle.h"

/* create 4 hash tables */
void
create_hash_array(search_t *table, piece_list_t *piece_list) {
    int i;
    piece_t *piece;

    //malloc spaces for hash tables
    table->hash_north = (list_t *) malloc(table->size * sizeof (list_t));
    table->hash_east = (list_t *) malloc(table->size * sizeof (list_t));
    table->hash_south = (list_t *) malloc(table->size * sizeof (list_t));
    table->hash_west = (list_t *) malloc(table->size * sizeof (list_t));

    //malloc locks for each cell of hash tables
    table->lock_north = (pthread_mutex_t *) malloc(table->size * sizeof (pthread_mutex_t));
    table->lock_east = (pthread_mutex_t *) malloc(table->size * sizeof (pthread_mutex_t));
    table->lock_south = (pthread_mutex_t *) malloc(table->size * sizeof (pthread_mutex_t));
    table->lock_west = (pthread_mutex_t *) malloc(table->size * sizeof (pthread_mutex_t));

    //initialize all lists in the table and the locks
    for (i = 0; i < table->size; i++) {
        list_init(&table->hash_north[i]);
        list_init(&table->hash_east[i]);
        list_init(&table->hash_south[i]);
        list_init(&table->hash_west[i]);

        pthread_mutex_init(&table->lock_north[i], NULL);
        pthread_mutex_init(&table->lock_east[i], NULL);
        pthread_mutex_init(&table->lock_south[i], NULL);
        pthread_mutex_init(&table->lock_west[i], NULL);
    }

    /* place all piece pointers into 4 hash table
     * the index is the tab value
     */  
    for (i = 0; i < piece_list->numpieces; i++) {
        piece = &piece_list->pieces[i];
        list_add(&table->hash_north[piece->tab[NORTH_TAB]], (void *) piece);
        list_add(&table->hash_east[piece->tab[EAST_TAB]], (void *) piece);
        list_add(&table->hash_south[piece->tab[SOUTH_TAB]], (void *) piece);
        list_add(&table->hash_west[piece->tab[WEST_TAB]], (void *) piece);
    }
}

/* search piece base on at least of 2 tabs
 * function will get 2 lists, each list contains
 * all same tab value in same dirction.
 * then cross compare all pieces of 2 list to find one match
 */
int
search_piece(piece_list_t *list, search_t *search, int n, int e, int s, int w,
        piece_t **piece) {
    int found = 0, count = 0;
    list_t *temp_list1 = NULL;
    list_t *temp_list2 = NULL;
    piece_t *temp_piece = NULL;
    pthread_mutex_t *lock1 = NULL;
    pthread_mutex_t *lock2 = NULL;

    //if north has index
    if (n != NO_PIECE_INDEX) {
        //get first list from north hash table index n
        temp_list1 = &search->hash_north[n];
        //get the lock of the list
        lock1 = &search->lock_north[n];
        
        //count one list is set
        count++;
    }
    //if east has index
    if (e != NO_PIECE_INDEX) {
        //if list one is set
        if (count > 0) {
            //get second list and lock
            temp_list2 = &search->hash_east[e];
            lock2 = &search->lock_east[e];

            //lock 2 lists and do the search
            pthread_mutex_lock(lock1);
            pthread_mutex_lock(lock2);
            found = list_search(temp_list1, temp_list2, (void **) &temp_piece);
            pthread_mutex_unlock(lock1);
            pthread_mutex_unlock(lock2);
        } else {
            //get firsst list
            temp_list1 = &search->hash_east[e];
            lock1 = &search->lock_east[e];
            
            //count one list is set
            count++;
        }
    }
    //skip if found or s has not index
    if (!found && s != NO_PIECE_INDEX) {
        //set the second list and search
        if (count > 0) {
            temp_list2 = &search->hash_south[s];
            lock2 = &search->lock_south[s];

            pthread_mutex_lock(lock1);
            pthread_mutex_lock(lock2);
            found = list_search(temp_list1, temp_list2, (void **) &temp_piece);
            pthread_mutex_unlock(lock1);
            pthread_mutex_unlock(lock2);
        } else {
            //set the first list
            temp_list1 = &search->hash_south[s];
            lock1 = &search->lock_south[s];
            count++;
        }
    }

    //skip if found or w has not index
    if (!found && w != NO_PIECE_INDEX) {
        //set the second list and search
        if (count > 0) {
            temp_list2 = &search->hash_west[w];
            lock2 = &search->lock_west[w];

            pthread_mutex_lock(lock1);
            pthread_mutex_lock(lock2);
            found = list_search(temp_list1, temp_list2, (void **) &temp_piece);
            pthread_mutex_unlock(lock1);
            pthread_mutex_unlock(lock2);
        }
    }

    //if found then set the piece for parent function
    if (found) {
        *piece = temp_piece;
    }

    //return 1 if found
    return found;
}
