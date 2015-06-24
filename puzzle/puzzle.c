#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "puzzle.h"
#include "queue.h"
#include "list.h"
#include "search.h"

/* Display the names of all the pieces in the grid. */

void
print_grid(grid_t *grid) {
    int i, j;

    for (j = 0; j < grid->numrows; j++) {
        for (i = 0; i < grid->numcols; i++) {
            if (grid->cells[i][j].piece == NULL) {
                printf(". ");
            } else {
                printf("%s ", grid->cells[i][j].piece->name);
            }
        }
        printf("\n");
    }
}

/* Display the set of tabs of the puzzle. */

void
print_edges(grid_t *grid) {
    int i, j;

    for (j = 0; j < grid->numrows; j++) {
        for (i = 0; i < grid->numcols; i++) {
            printf("   %3d", grid->cells[i][j].north);
        }
        printf("\n");
        for (i = 0; i <= grid->numcols; i++) {
            printf("%3d   ", grid->cells[i][j].west);
        }
        printf("\n");
    }
    for (i = 0; i < grid->numcols; i++) {
        printf("   %3d", grid->cells[i][grid->numrows].north);
    }
    printf("\n");
}

/* Retrieve the puzzle configuration from stdin. */

int
get_input(grid_t *grid, piece_list_t *piece_list, search_t *table) {
    int return_value = 0;
    char line[MAXLINELEN + 1];
    cell_t *space;
    char *token;
    char *context;
    int i, max_num = 0;
    int *cols = &(grid->numcols);
    int *rows = &(grid->numrows);
    piece_t **piece = &(piece_list->pieces);

    /* Get the grid size. */

    fgets(line, MAXLINELEN, stdin);
    line[MAXLINELEN] = '\0';

    sscanf(line, "%d %d", cols, rows);

    /* Use a "trick" for two dimensional array space management.  Allocate
       the entire 2d array as a sing sequence of cells and then build up
       the 2d index by pointing into parts of that space.  The trick means
       that we can release the whole 2d array with just two calls to "free". */

    space = (cell_t *) malloc((*rows + 1) * (*cols + 1) * sizeof ( cell_t));
    grid->cells = (cell_t **) malloc((*cols + 1) * sizeof ( cell_t *));

    if ((space != NULL) && (grid->cells != NULL)) {
        /* Initialize the space. */

        for (i = 0; i < *cols + 1; i++) {
            grid->cells[i] = space + i * (*rows + 1);
        }

        for (i = 0; i < (*rows + 1)*(*cols + 1); i++) {
            space[i].north = NO_PIECE_INDEX;
            space[i].west = NO_PIECE_INDEX;
            space[i].piece = NULL;
            pthread_mutex_init(&space[i].lock, NULL);
            sem_init(&space[i].queue_lock, 0, 1);
            sem_init(&space[i].tab_lock, 0, 1);
        }

        /* Get the top. */

        fgets(line, MAXLINELEN, stdin);
        context = NULL;
        token = strtok_r(line, " \n", &context);
        for (i = 0; i < *cols; i++) {
            grid->cells[i][0].north = atoi(strtok_r(NULL, " \n", &context));
            sem_trywait(&grid->cells[i][0].tab_lock);
        }

        /* Get the bottom. */

        fgets(line, MAXLINELEN, stdin);
        context = NULL;
        token = strtok_r(line, " \n", &context);
        for (i = 0; i < *cols; i++) {
            grid->cells[i][*rows].north = atoi(strtok_r(NULL, " \n", &context));
            sem_trywait(&grid->cells[i][*rows].tab_lock);
        }

        /* Get the left side. */

        fgets(line, MAXLINELEN, stdin);
        context = NULL;
        token = strtok_r(line, " \n", &context);
        for (i = 0; i < *rows; i++) {
            grid->cells[0][i].west = atoi(strtok_r(NULL, " \n", &context));
            sem_trywait(&grid->cells[0][i].tab_lock);
        }

        /* Get the right. */

        fgets(line, MAXLINELEN, stdin);
        context = NULL;
        token = strtok_r(line, " \n", &context);
        for (i = 0; i < *rows; i++) {
            grid->cells[*cols][i].west = atoi(strtok_r(NULL, " \n", &context));
            sem_trywait(&grid->cells[*cols][i].tab_lock);
        }

        /* Get the pieces now. */

        *piece = (piece_t *) malloc(*rows * *cols * sizeof ( piece_t));
        piece_list->numpieces = *rows * *cols;
        if (*piece != NULL) {
            for (i = 0; i < *rows * *cols; i++) {
                fgets(line, MAXLINELEN, stdin);
                sscanf(line, "%s %d %d %d %d", (*piece)[i].name,
                        &((*piece)[i].tab[NORTH_TAB]), &((*piece)[i].tab[EAST_TAB]),
                        &((*piece)[i].tab[SOUTH_TAB]), &((*piece)[i].tab[WEST_TAB]));

                //find the maximum number of the piece
                if ((*piece)[i].tab[NORTH_TAB] > max_num) {
                    max_num = (*piece)[i].tab[NORTH_TAB];
                } else if ((*piece)[i].tab[EAST_TAB] > max_num) {
                    max_num = (*piece)[i].tab[EAST_TAB];
                } else if ((*piece)[i].tab[SOUTH_TAB] > max_num) {
                    max_num = (*piece)[i].tab[SOUTH_TAB];
                } else if ((*piece)[i].tab[WEST_TAB] > max_num) {
                    max_num = (*piece)[i].tab[WEST_TAB];
                }
            }
        }

        //create hash tables from pieces above
        table->size = max_num + 1;
        create_hash_array(table, piece_list);

        return_value = 1;
    }

    return return_value;
}

/* Free up the memory that get_input allocates. */

void
release_memory(grid_t *grid, piece_list_t *piece_list, search_t *search) {
    int i;

    /* free hash tables */
    for (i = 0; i < search->size; i++) {
        list_destory(&search->hash_north[i]);
        list_destory(&search->hash_east[i]);
        list_destory(&search->hash_south[i]);
        list_destory(&search->hash_west[i]);
    }

    free(search->hash_north);
    free(search->hash_east);
    free(search->hash_south);
    free(search->hash_west);



    /* Get rid of all the pieces. */

    free(piece_list->pieces);
    piece_list->pieces = NULL;

    /* Get rid of the puzzle grid. */

    free(grid->cells[0]);
    free(grid->cells);
    grid->cells = NULL;
}

#define END_INDEX (-1)

typedef struct {
    int col;
    int row;
} task_t;

typedef struct {
    grid_t *grid;
    piece_list_t *piece_list;
    queue_t *queue;
    search_t *search;
    sem_t *counter_lock;
} thread_data_t;

/*
 * main function for each thread. each thread will get a task from
 * the queue, find the right piece to fit the cell. then try to find
 * next solvable cells and put the task into queue.
 */
void
*solve_puzzle(void *param) {
    int row, col;
    task_t *task, *temp;
    cell_t *cell;
    piece_t *temp_piece;
    thread_data_t *data = (thread_data_t*) param;

    //loop until not more piece to solve
    while (queue_pop(data->queue, (void **) &task)) {
        row = task->row;
        col = task->col;

        //if row is end index, quit the thread
        if (row == END_INDEX) {
            queue_push(data->queue, (void *) task);
            return NULL;
        }

        //get the cell to fit piece
        cell = &data->grid->cells[col][row];

        /* we need 3 locks to protect critical section: it self, one on the right
         and one at the buttom. if we can not get all locks, we will skip this
         task first and put this back to queue. */
        pthread_mutex_lock(&cell->lock);
        if (pthread_mutex_trylock(&data->grid->cells[col][row + 1].lock) != 0) {
            //if can not get lock 
            pthread_mutex_unlock(&cell->lock);
            queue_push(data->queue, (void *) task);
            continue;
        }
        if (pthread_mutex_trylock(&data->grid->cells[col + 1][row].lock) != 0) {
            //if can not get lock 
            pthread_mutex_unlock(&cell->lock);
            pthread_mutex_unlock(&data->grid->cells[col][row + 1].lock);
            queue_push(data->queue, (void *) task);
            continue;
        }

        //continue if we get all locks
        //search piece base on information at this cell
        if (search_piece(data->piece_list, data->search, cell->north,
                data->grid->cells[col + 1][row].west, data->grid->cells[col][row + 1].north,
                cell->west, &temp_piece)) {
            //if we found the piece, place the piece and update the tabs
            data->grid->cells[col][row].piece = temp_piece;
            data->grid->cells[col][row].north = temp_piece->tab[NORTH_TAB];
            data->grid->cells[col + 1][row].west = temp_piece->tab[EAST_TAB];
            data->grid->cells[col][row + 1].north = temp_piece->tab[SOUTH_TAB];
            data->grid->cells[col][row].west = temp_piece->tab[WEST_TAB];

            //count how many pieces let, if 0 then let every thread quit
            if (sem_trywait(data->counter_lock) != 0) {
                //queue a task has quit condition then quit itself
                temp = malloc(sizeof (task_t));
                temp->row = END_INDEX;
                queue_push(data->queue, (void *) temp);
                return NULL;
            }

            //release all locks
            pthread_mutex_unlock(&data->grid->cells[col][row + 1].lock);
            pthread_mutex_unlock(&data->grid->cells[col + 1 ][row].lock);
            pthread_mutex_unlock(&cell->lock);

            /* this part is for testing neighbour cells if they can be solve
             * first shold test tab lock, if the thread can not get the tab lock
             * means this cell have 2 tabs placed. so it is a slovable cell
             * 
             * queue lock is used to make sure the cell is not been queue twice
             */
            
            //test cell on the right
            if (col < data->grid->numcols - 1) {
                if (sem_trywait(&data->grid->cells[col + 1][row].tab_lock) != 0) {
                    if (sem_trywait(&data->grid->cells[col + 1][row].queue_lock) == 0) {
                        //if this thread can get the queue lock, means this cell is
                        //not in queue, then queue it.
                        temp = malloc(sizeof (task_t));
                        temp->col = col + 1;
                        temp->row = row;
                        //push the task into queue
                        queue_push(data->queue, (void *) temp);
                    }
                }
            }
            //test cell at the buttom
            if (row < data->grid->numrows - 1) {
                if (sem_trywait(&data->grid->cells[col][row + 1].tab_lock) != 0) {
                    if (sem_trywait(&data->grid->cells[col][row + 1].queue_lock) == 0) {
                        temp = malloc(sizeof (task_t));
                        temp->col = col;
                        temp->row = row + 1;
                        queue_push(data->queue, (void *) temp);
                    }
                } 
            }
            //test cell at the left
            if (col > 0) {
                if (sem_trywait(&data->grid->cells[col - 1][row].tab_lock) != 0) {
                    if (sem_trywait(&data->grid->cells[col - 1][row].queue_lock) == 0) {

                        temp = malloc(sizeof (task_t));
                        temp->col = col - 1;
                        temp->row = row;
                        queue_push(data->queue, (void *) temp);
                    }
                } 
            }
            //test cell above
            if (row > 0) {
                if (sem_trywait(&data->grid->cells[col][row - 1].tab_lock) != 0) {
                    if (sem_trywait(&data->grid->cells[col][row - 1].queue_lock) == 0) {

                        temp = malloc(sizeof (task_t));
                        temp->col = col;
                        temp->row = row - 1;
                        queue_push(data->queue, (void *) temp);

                    }
                }
            }


            //free this task memery
            free(task);
        } else {
            //if can not found a piece to fix, unlock and queue this task again
            pthread_mutex_unlock(&data->grid->cells[col][row + 1].lock);
            pthread_mutex_unlock(&data->grid->cells[col + 1 ][row].lock);
            pthread_mutex_unlock(&cell->lock);

            queue_push(data->queue, (void *) task);
        }

    }

    return NULL;
}

/*
 * main function, get input of number of threads
 * start threads and wait all thread to complete
 * free all memery and quit
 */
int
main(int argc, char ** argv) {
    int return_value = 0;
    piece_list_t piece_list;
    grid_t grid;
    search_t search;
    int i, t_num;
    pthread_t *thread = NULL;
    thread_data_t data;
    sem_t counter_lock;
    queue_t queue;
    task_t *temp = NULL;

    if (get_input(&grid, &piece_list, &search)) {

        //initialize the queue
        queue_init(&queue);

        /* the 4 corner is solvable. add them to the queue */
        temp = malloc(sizeof (task_t));
        temp->col = 0;
        temp->row = 0;
        sem_wait(&grid.cells[temp->col][temp->row].queue_lock);
        queue_push(&queue, (void*) temp);

        temp = malloc(sizeof (task_t));
        temp->col = 0;
        temp->row = grid.numrows - 1;
        sem_wait(&grid.cells[temp->col][temp->row].queue_lock);
        queue_push(&queue, (void*) temp);

        temp = malloc(sizeof (task_t));
        temp->col = grid.numcols - 1;
        temp->row = 0;
        sem_wait(&grid.cells[temp->col][temp->row].queue_lock);
        queue_push(&queue, (void*) temp);

        temp = malloc(sizeof (task_t));
        temp->col = grid.numcols - 1;
        temp->row = grid.numrows - 1;
        sem_wait(&grid.cells[temp->col][temp->row].queue_lock);
        queue_push(&queue, (void*) temp);

        /* initialize thread pool*/

        //get number of threads to start
        if (argc > 1) {
            t_num = atoi(argv[1]);
        } else {
            t_num = 1;
        }

        //init sem counter for all piece
        sem_init(&counter_lock, 0, piece_list.numpieces - 1);

        //malloc threads and data for start a thread
        thread = (pthread_t *) malloc(t_num * sizeof (pthread_t));

        //set up the data passing into threads
        data.grid = &grid;
        data.piece_list = &piece_list;
        data.queue = &queue;
        data.search = &search;
        data.counter_lock = &counter_lock;

        //start all threads
        for (i = 0; i < t_num; i++) {
            pthread_create(&thread[i], NULL, solve_puzzle, &data);
        } 

        /* wait all the thread to complete the job */
        for (i = 0; i < t_num; i++) {
            pthread_join(thread[i], NULL);
        }

        //release thread data memery
        free(thread);

        /* Show what the puzzle came out to be. */

        print_grid(&grid);

        release_memory(&grid, &piece_list, &search);
    }


    return return_value;
}
