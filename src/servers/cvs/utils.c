#include "inc.h"
#include "structures.h"
#include "utils.h"

static int check_operation_unlock(int);

static int verbose = 0;

/*===========================================================================*
 				            reply			                                 
 *===========================================================================*/
void reply(endpoint_t to_whom, int result)
{
    message m;
    m.m_type = result;
    int r;
    if ((r = sendnb(to_whom, &m)) != OK) {
        if (verbose) printf("CVS send error %d.\n", r);
    }
}

/*===========================================================================*
 		                     mutex_unlock                                    
 *===========================================================================*/
int mutex_unlock(endpoint_t proc, int mutex_id, int operation)
{   
    int pos = mutex_find_id(mutex_id);
    if (pos == -1) {
        if (verbose) printf("Proc: %d, tries to unlock mutex: %d, case no such mutex.\n", proc, mutex_id);
        return check_operation_unlock(operation);
    }

    if (mutex[pos].owner != proc) {
        if (verbose) printf("Proc: %d, requested unlocking not owned mutex, id: %d.\n", proc, mutex_id);
        return check_operation_unlock(operation);
    }

    if (mutex[pos].waiting_procs == CURSOR_NULL) {
        if (verbose) printf("Proc: %d, no queue in mutex: %d.\n", proc, mutex_id);
        mutex[pos].flags = FREE;
        return OK;
    }

    if (verbose) printf("Proc: %d, popping queue in mutex: %d.\n", proc, mutex_id);
    
    queue q = mutex[pos].waiting_procs;
    position p = cursor_front(q);
    
    endpoint_t new_owner = proc_space[p].proc.proc_endp;
    mutex[pos].owner = new_owner;
    mutex_remove_proc(new_owner, q, pos);

    reply(new_owner, OK); /* Inform new owner */
    if (verbose) printf("Proc: %d new owner of mutex: %d.\n", new_owner, mutex_id);
   
    return OK;
}

static int check_operation_unlock(int operation)
{
    if (operation == UNLOCK) return EPERM;
    else if (operation == WAIT) return EINVAL;
    else panic("Illegal operation passed to mutex_unlock().\n");
}

/*===========================================================================*
 		                     mutex_enqueue                                    
 *===========================================================================*/
int mutex_enqueue(struct proc proc_to_enqueue, position mutex_pos)
{
    endpoint_t proc = proc_to_enqueue.proc_endp;
    int mutex_id = proc_to_enqueue.mutex_id;

    if (mutex[mutex_pos].waiting_procs == CURSOR_NULL) {
        if (verbose) printf("Proc: %d, first in queue for mutex: %d.\n", proc, mutex_id);
        queue q = cursor_new_queue(); 
        if (q == CURSOR_NULL) return ENOMEM;
        int r = cursor_enqueue(proc_to_enqueue, q);
        if (r != OK) {
            cursor_free(q);
            return ENOMEM;
        }
        mutex[mutex_pos].waiting_procs = q;
        return OK;
    }

    if (verbose) printf("Proc: %d, not first in queue for mutex: %d.\n", proc, mutex_id);
    int r = cursor_enqueue(proc_to_enqueue, mutex[mutex_pos].waiting_procs);
    if (r != OK) return ENOMEM;
    
    return OK;
}

/*===========================================================================*
 		                     mutex_remove_proc                                    
 *===========================================================================*/
void mutex_remove_proc(endpoint_t proc, queue q, int pos)
{
    cursor_dequeue(proc, q);
    if (cursor_is_empty(q)) {
        cursor_free(q);
        mutex[pos].waiting_procs = CURSOR_NULL;
    }
}

/*===========================================================================*
 		                     condv_remove_proc                                    
 *===========================================================================*/
void condv_remove_proc(endpoint_t proc, queue q, int pos)
{
    cursor_dequeue(proc, q);
    if (cursor_is_empty(q)) {
        cond_vars[pos].flags = FREE;
        cursor_free(q);
        cond_vars[pos].waiting_procs = CURSOR_NULL;
    }
}

/*===========================================================================*
 		                     register_in_pm                                   
 *===========================================================================*/
void register_in_pm()
{
    message m; 
    m.m_type = I_AM_CVS_BETTER_REMEMBER_ME_PM;
    int r = sendrec(PM_PROC_NR, &m);
    if (r != OK) 
        panic("CVS unable to register in pm: %d\n", r);
}

/*===========================================================================*
 		                     unregister_in_pm                                    
 *===========================================================================*/
void unregister_in_pm()
{
    message m;
    m.m_type = BYE_PM;
    int r = sendrec(PM_PROC_NR, &m);
    if (r != OK)
        panic("CVS unable to unregister in pm: %d\n", r);
}
