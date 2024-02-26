#include "glo.h"
#include "inc.h"
#include "structures.h"
#include "utils.h"
#include "signals.h"

static int verbose = 0;

/*===========================================================================*
 		                       do_lock                                       
 *===========================================================================*/
int do_lock(message *m) 
{
    int mutex_id = m->CVS_MUTEX_ID;
    endpoint_t proc = who_e;

    int pos = mutex_find_id(mutex_id);
    if (pos == -1) {
        if (verbose) printf("Proc: %d, gets mutex: %d, case no such mutex.\n", proc, mutex_id);
        pos = mutex_find_free();
        if (pos == -1) return ENOMEM;
        mutex[pos].id = mutex_id;
        mutex[pos].owner = proc;
        mutex[pos].flags = IN_USE;
        return OK;
    }

    if (mutex[pos].owner == proc) {
        if (verbose) printf("Proc: %d, requested owned mutex, id: %d.\n", proc, mutex_id);
        return EPERM;
    }
    struct proc proc_to_enqueue = {proc, mutex_id};
    int r = mutex_enqueue(proc_to_enqueue, pos);
    if (r != OK) return r;
    
    return EDONTREPLY;
}

/*===========================================================================*
 		                       do_unlock                                     
 *===========================================================================*/
int do_unlock(message *m)
{
    int mutex_id = m->CVS_MUTEX_ID;
    endpoint_t proc = who_e;
    return mutex_unlock(proc, mutex_id, UNLOCK); 
}

/*===========================================================================*
 		                       do_wait                                       
 *===========================================================================*/
int do_wait(message *m)
{
    int mutex_id = m->CVS_MUTEX_ID;
    int cond_var_id = m->CVS_CV_ID;
    endpoint_t proc = who_e;

    int r = mutex_unlock(proc, mutex_id, WAIT);
    if (r != OK) return r;
    
    struct proc proc_to_enqueue = {proc, mutex_id};

    /* Note that errors with memory (not expected) cause process to not wait and lose mutex */
    int pos = condv_find_id(cond_var_id);
    if (pos == -1) {
        if (verbose) printf("Proc: %d, first to wait for cv: %d.\n", proc, cond_var_id);
        pos = condv_find_free();
        if (pos == -1) return ENOMEM;
        cond_vars[pos].id = cond_var_id;
        cond_vars[pos].flags = IN_USE;
        
        queue q = cursor_new_queue(); 
        if (q == CURSOR_NULL) {
            cond_vars[pos].flags = FREE;
            return ENOMEM;
        }
        int r = cursor_enqueue(proc_to_enqueue, q);
        if (r != OK) {
            cursor_free(q);
            cond_vars[pos].flags = FREE;
            return ENOMEM;
        }
        cond_vars[pos].waiting_procs = q;
        return EDONTREPLY;
    } 

    if (verbose) printf("Proc: %d, not first in queue for cv: %d.\n", proc, cond_var_id);
    r = cursor_enqueue(proc_to_enqueue, cond_vars[pos].waiting_procs);
    if (r != OK) return r;
    
    return EDONTREPLY;
}

/*===========================================================================*
		                       do_broadcast                                  
 *===========================================================================*/
int do_broadcast(message *m)
{
    int cond_var_id = m->CVS_CV_ID;
    endpoint_t proc = who_e;
    
    /* Inform broadcaster */
    reply(proc, OK);

    int pos_cv = condv_find_id(cond_var_id);
    if (pos_cv != -1) {
        if (verbose) printf("Some procs are waiting for cv: %d.\n", cond_var_id);
        
        position p;
        endpoint_t proc_to_enqueue_endp;
        int mutex_id, pos_m;
        queue q = cond_vars[pos_cv].waiting_procs;
        
        while (!cursor_is_empty(q)) {
            p = cursor_front(q);

            proc_to_enqueue_endp = proc_space[p].proc.proc_endp;
            mutex_id = proc_space[p].proc.mutex_id;
            
            cursor_dequeue(proc_to_enqueue_endp, q);

            pos_m = mutex_find_id(mutex_id);
            if (pos_m == -1) {
                if (verbose) 
                    printf("Proc: %d, releasing from wait cv: %d.\n", proc_to_enqueue_endp, cond_var_id); 
                
                pos_m = mutex_find_free();
                if (pos_m == -1) reply(proc_to_enqueue_endp, ENOMEM); /* Not expected */
                mutex[pos_m].id = mutex_id;
                mutex[pos_m].owner = proc_to_enqueue_endp;
                mutex[pos_m].flags = IN_USE;
                reply(proc_to_enqueue_endp, OK);
            } else {
                if (verbose) 
                    printf("Proc: %d, in queue for its mutex: %d. [cv]\n", proc_to_enqueue_endp, mutex_id);
                
                struct proc proc_to_enqueue = {proc_to_enqueue_endp, mutex_id};
                int r = mutex_enqueue(proc_to_enqueue, pos_m);
                if (r != OK) reply(proc_to_enqueue_endp, r);
            }
        }
    }

    return EDONTREPLY; /* Already informed */
}


