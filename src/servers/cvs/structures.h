#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

#include "inc.h"

typedef unsigned node_ptr;
typedef node_ptr queue; 
typedef node_ptr position;

/*===========================================================================*
 				            mutex tab 	                                     
 *===========================================================================*/
struct mutex {
    int id;
    endpoint_t owner;
    unsigned flags;
    queue waiting_procs;
} mutex[MUTEX_NR];

void mutex_init();
int mutex_find_free();
int mutex_find_id(int);
int mutex_is_clean();

/*===========================================================================*
 				            slot  flags		                                 
 *===========================================================================*/
#define FREE        0x0    /* Set when slot free   */
#define IN_USE      0x1    /* Set when slot in use */

/*===========================================================================*
 				            proc waiting tab                                 
 *===========================================================================*/

/* 
Note from author: 
I never had to implement cursor list.
I know I could use array cyclic queue here, but this situation, where I coudn't use malloc
seemed perfect to try implementing memory arena :)  
*/

struct proc {
    endpoint_t proc_endp;
    int mutex_id;
};

#define SIZE_OF_CURSOR_SPACE (2 * MUTEX_NR + NR_PROCS + 1) 
/* 
+ 2 * MUTEX_NR because of guard impl of cursor list 
+ 1 because of memory arena guard
and this is clearly enough if we assume process cannot make threads
*/

struct node {
    struct proc proc;
    node_ptr next;
    node_ptr prev;
} proc_space[SIZE_OF_CURSOR_SPACE]; 

#define GUARD_OF_PROC_SPACE 0 /* Do not change this */
#define CURSOR_NULL GUARD_OF_PROC_SPACE /* It can be all queue, node_ptr and position */

void cursor_init();
position cursor_alloc();
void cursor_free(position);
queue cursor_new_queue();
int cursor_is_empty(queue);
position cursor_find(endpoint_t, queue);
int cursor_dequeue(endpoint_t, queue); /* Not pop */
int cursor_enqueue(struct proc, queue);
position cursor_front(queue);

/*===========================================================================*
 				            cv tab                                           
 *===========================================================================*/
struct cond_var {
    int id;
    queue waiting_procs;
    unsigned flags; /* Same flags as in mutex */
} cond_vars[COND_VAR_NR];

void condv_init();
int condv_find_free();
int condv_find_id(int);
int condv_is_clean();

#endif /* !_STRUCTURES_H_ */
