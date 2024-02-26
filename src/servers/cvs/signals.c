#include "inc.h"
#include "structures.h"
#include "utils.h"
#include "signals.h"

static int find_proc_in_any_mutex_queue(endpoint_t); /* Returns mutex pos */
static int find_proc_in_any_condv_queue(endpoint_t); /* Returns condv pos */

static int verbose = 0;

/*===========================================================================*
 		                     handle_exit                                    
 *===========================================================================*/
void handle_exit(message *m)
{
    endpoint_t proc = m->PM_PROC;

    if (verbose) printf("Proc: %d about to exit.[exit]\n", proc);
    /* Delete proc owners */
    for (int i = 0; i < MUTEX_NR; ++i) {
        if (mutex[i].flags == FREE) continue;
        if (mutex[i].owner == proc) {
            if (verbose) printf("Unlocking mutex: %d.[exit]\n", mutex[i].id);
            mutex_unlock(proc, mutex[i].id, UNLOCK); /* OPERATION doesn't really matter */
        }
    }
    
    /* Delete proc from queues */
    int pos = find_proc_in_any_mutex_queue(proc);
    if (pos != -1) {
        if (verbose) printf("Proc: %d in queue for mutex.[exit]\n", proc); 
        queue q = mutex[pos].waiting_procs;
        mutex_remove_proc(proc, q, pos);
        return;
    }

    pos = find_proc_in_any_condv_queue(proc);
    if (pos != -1) {
        if (verbose) printf("Proc: %d in queue for condv.[exit]\n", proc);
        queue q = cond_vars[pos].waiting_procs;
        condv_remove_proc(proc, q, pos);
    }
}

/*===========================================================================*
 		                     handle_sig                                   
 *===========================================================================*/
void handle_sig(message *m)
{
    endpoint_t proc = m->PM_PROC;
    
    /* Note that if proc is owner we don't care */
    
    /* Set flag is_handling_sig */
    int pos = find_proc_in_any_mutex_queue(proc);
    if (pos != -1) {
        queue q = mutex[pos].waiting_procs;
        mutex_remove_proc(proc, q, pos);
        reply(proc, EINTR);
        return;
    }
   

    /* Spurious wakeup */
    pos = find_proc_in_any_condv_queue(proc);
    if (pos != -1) {
        queue q = cond_vars[pos].waiting_procs;
        condv_remove_proc(proc, q, pos);
        reply(proc, EINTR);
    }
}

/*===========================================================================*
 		                     find_proc_in_any_mutex_queue                                    
 *===========================================================================*/
static int find_proc_in_any_mutex_queue(endpoint_t proc)
{
    position p = CURSOR_NULL;

    for (int i = 0; i < MUTEX_NR; ++i) {
        if (mutex[i].flags == FREE) continue;
        if (mutex[i].waiting_procs != CURSOR_NULL) {
            p = cursor_find(proc, mutex[i].waiting_procs);
            if (p != CURSOR_NULL) return i;
        }
    }
    return -1; 
}

/*===========================================================================*
 		                     find_proc_in_any_condv_queue                                    
 *===========================================================================*/
static int find_proc_in_any_condv_queue(endpoint_t proc)
{
    position p = CURSOR_NULL;

    for (int i = 0; i < COND_VAR_NR; ++i) {
        if (cond_vars[i].flags == FREE) continue;
        if (cond_vars[i].waiting_procs != CURSOR_NULL) {
            p = cursor_find(proc, cond_vars[i].waiting_procs);
            if (p != CURSOR_NULL) return i;
        }
    }
    return -1; 
}


