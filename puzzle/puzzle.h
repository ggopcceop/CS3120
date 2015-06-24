#ifndef PUZZLE_H
#define	PUZZLE_H

#include <pthread.h>
#include <semaphore.h>

#define LABEL_LEN (12)
#define MAXLINELEN (8096)

/* Each puzzle piece is an array of 4 tabs ordered clockwise and starting
   at the top (north) tab. */

#define NORTH_TAB (0)
#define EAST_TAB (1)
#define SOUTH_TAB (2)
#define WEST_TAB (3)

#define NO_PIECE_INDEX (-1)

typedef struct {
    int tab[4];
    char name[LABEL_LEN + 1];
} piece_t;

/* We'll want to keep all the pieces and the number of them together. */

typedef struct {
    piece_t *pieces;
    int numpieces;
} piece_list_t;

/* A cell in the grid knows its north and west tabs.  Since this cell is
   expected to be in a grid, its east tab is the same as the west tab of the
   next cell to the right.  Its south tab is the same as the north tab of the 
   cell immediately below. 

    When we define our grid, cell entry (0, 0) will be the top left
    corner with the "y" values increasing as you go down the page.
    That's the opposite of typical geometry from high school but
    is not uncommon in graphics systems.
 */

typedef struct {
    int north;
    int west;
    piece_t *piece;
    pthread_mutex_t lock;   //lock when it is editting tabs
    sem_t queue_lock; //counter to test if the cell is queued
    sem_t tab_lock;   //counter to test if this cell is solvable
} cell_t;

typedef struct {
    cell_t **cells;
    int numcols;
    int numrows;
} grid_t;

#endif	/* PUZZLE_H */

