#include "structures.h"

/*===========================================================================*
 				            mutex_init 	                                     
 *===========================================================================*/
void mutex_init()
{
    struct mutex *m;
    
    for (m = mutex; m != mutex + MUTEX_NR; m++) {
        m->id = -1;
        m->owner = -1;
        m->flags = FREE;
        m->waiting_procs = CURSOR_NULL; 
    }
}

/*===========================================================================*
 				            mutex_find_free                                  
 *===========================================================================*/
int mutex_find_free()
{ 
    int pos = -1;
    for (int i = 0; i < MUTEX_NR; ++i) {
        if (mutex[i].flags == IN_USE) continue;
        pos = i; /* Remember to mark IN_USE */
        break;
    }
    return pos; 
}

/*===========================================================================*
 				            mutex_find_id                                    
 *===========================================================================*/
int mutex_find_id(int id)
{
    for (int i = 0; i < MUTEX_NR; ++i) {
        if (mutex[i].id == id && mutex[i].flags == IN_USE) return i;
    }
    return -1;
}

/*===========================================================================*
 				            mutex_is_clean                                    
 *===========================================================================*/
int mutex_is_clean()
{
    for (int i = 0; i < MUTEX_NR; ++i) {
        if (mutex[i].flags == FREE) continue;
        if (mutex[i].waiting_procs != CURSOR_NULL) return 0;
    }
    return 1;   
}

/*===========================================================================*
 				            cursor_init                                      
 *===========================================================================*/
void cursor_init()
{
    node_ptr prev = GUARD_OF_PROC_SPACE; 
    node_ptr cur =  GUARD_OF_PROC_SPACE + 1;
    
    for (int i = 0; i < SIZE_OF_CURSOR_SPACE; ++i) {
        proc_space[prev].next = cur; 
        proc_space[cur].prev = prev;
        prev = cur;
        cur = (cur + 1) % SIZE_OF_CURSOR_SPACE;
    }
}

/*===========================================================================*
 				            cursor_alloc                                     
 *===========================================================================*/
position cursor_alloc()
{
    position p, next;
    
    p = proc_space[GUARD_OF_PROC_SPACE].next;
    next = proc_space[p].next;
    proc_space[GUARD_OF_PROC_SPACE].next = next;
    proc_space[next].prev = GUARD_OF_PROC_SPACE;
    return p; /* Returns CURSOR_NULL (0) if no space */
}

/*===========================================================================*
 				            cursor_free                                      
 *===========================================================================*/
void cursor_free(position p) 
{
    position next = proc_space[GUARD_OF_PROC_SPACE].next;
    
    proc_space[p].next = next; 
    proc_space[p].prev = GUARD_OF_PROC_SPACE;
    proc_space[next].prev = p; 
    proc_space[GUARD_OF_PROC_SPACE].next = p;
}

/*===========================================================================*
 				            cursor_new_queue                                 
 *===========================================================================*/
queue cursor_new_queue()
{
    queue q = cursor_alloc();
    
    if (q != CURSOR_NULL) {
        proc_space[q].next = q;
        proc_space[q].prev = q;
    }
    return q;
}
    
/*===========================================================================*
 				            cursor_is_empty                                  
 *===========================================================================*/
int cursor_is_empty(queue q)
{
    return proc_space[q].next == q;
}

/*===========================================================================*
 				            cursor_find                                      
 *===========================================================================*/
position cursor_find(endpoint_t proc, queue q)
{
    if (cursor_is_empty(q)) return CURSOR_NULL;
    
    position guard = q;
    position cur = proc_space[q].next;
    
    while (cur != guard) {
        if (proc_space[cur].proc.proc_endp == proc) return cur;
        cur = proc_space[cur].next;
    }
    return CURSOR_NULL;
}

/*===========================================================================*
 				            cursor_dequeue                                   
 *===========================================================================*/
int cursor_dequeue(endpoint_t proc, queue q)
{
    position p = cursor_find(proc, q);
    
    if (p == CURSOR_NULL) return -1;

    position next = proc_space[p].next; 
    position prev = proc_space[p].prev;
    proc_space[next].prev = prev;
    proc_space[prev].next = next;
    cursor_free(p);
    return OK;
}

/*===========================================================================*
 				            cursor_enqueue                                   
 *===========================================================================*/
int cursor_enqueue(struct proc proc, queue q)
{
    position tmp_cell;
    
    tmp_cell = cursor_alloc();
    
    if (tmp_cell == CURSOR_NULL) return -1;
    
    proc_space[tmp_cell].proc = proc; /* Trivial copy is fine */
    
    position prev = proc_space[q].prev;
    proc_space[tmp_cell].next = q; 
    proc_space[tmp_cell].prev = prev;
    proc_space[q].prev = tmp_cell;
    proc_space[prev].next = tmp_cell;
    return OK;
}

/*===========================================================================*
 				            cursor_front                                     
 *===========================================================================*/
position cursor_front(queue q)
{
    return proc_space[q].next;
}

/*===========================================================================*
 				            condv_init                                       
 *===========================================================================*/
void condv_init()
{
    struct cond_var *c;
    
    for (c = cond_vars; c != cond_vars + COND_VAR_NR; c++) {
        c->id = 0;
        c->flags = FREE;
        c->waiting_procs = CURSOR_NULL; 
    } 
}

/*===========================================================================*
 				            condv_find_free                                  
 *===========================================================================*/
int condv_find_free()
{
    int pos = -1;
    
    for (int i = 0; i < COND_VAR_NR; ++i) {
        if (cond_vars[i].flags == IN_USE) continue;
        pos = i; /* Remember to mark IN_USE */
        break;
    }
    return pos; 
}

/*===========================================================================*
 				            condv_find_id                                    
 *===========================================================================*/
int condv_find_id(int id)
{
    for (int i = 0; i < COND_VAR_NR; ++i) {
        if (cond_vars[i].id == id) return i;
    }
    return -1;
}

/*===========================================================================*
 				            condv_is_clean                                  
 *===========================================================================*/
int condv_is_clean()
{
    for (int i = 0; i < COND_VAR_NR; ++i) {
        if (cond_vars[i].flags == FREE) continue;
        if (cond_vars[i].waiting_procs != CURSOR_NULL) return 0;
    }
    return 1; 
}




