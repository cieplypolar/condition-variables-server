#include <lib.h>
#include <minix/rs.h>
#include <stdio.h>
#include <cv.h>

static int verbose = 0;

static int get_cvs_endpt(int *pt)
{
    /* errno changed by _syscall if fail */
    return minix_rs_lookup(CVS, pt);
}

/*===========================================================================*
 				            cs_lock			                                 
 *===========================================================================*/
int cs_lock(int mutex_id)
{
	message m;
	m.CVS_MUTEX_ID = mutex_id;
    endpoint_t pt;
    if (get_cvs_endpt(&pt) != 0){
        if (verbose)
            printf("CVS not found\n");
        return -1;
    }

    while(1) {
        if (_syscall(pt, CVS_LOCK, &m) == 0) return 0;
        if (errno != EINTR) return -1;
        m.CVS_MUTEX_ID = mutex_id;
    }
    /* no way to get here */
    return -1;
}


/*===========================================================================*
 				            cs_unlock		                                 
 *===========================================================================*/
int cs_unlock(int mutex_id)
{   
    message m;
    m.CVS_MUTEX_ID = mutex_id;
    endpoint_t pt;
    if (get_cvs_endpt(&pt) != 0){
        if (verbose)
            printf("CVS not found.\n");
        return -1;
    }

    if (_syscall(pt, CVS_UNLOCK, &m) < 0) return -1;    
    return 0;
}


/*===========================================================================*
 				            cs_wait			                                 
 *===========================================================================*/
int cs_wait(int cond_var_id, int mutex_id)
{   
    message m;
    m.CVS_MUTEX_ID = mutex_id;
    m.CVS_CV_ID = cond_var_id;
    endpoint_t pt;
    if (get_cvs_endpt(&pt) != 0){
        if (verbose)
            printf("CVS not found.\n");
        return -1;
    }

    if (_syscall(pt, CVS_WAIT, &m) == 0) return 0;
    if (errno != EINTR) return -1;
    return cs_lock(mutex_id);
}

/*===========================================================================*
 				            cs_broadcast	                                 
 *===========================================================================*/
int cs_broadcast(int cond_var_id)
{   
    message m;
    m.CVS_CV_ID = cond_var_id;
    endpoint_t pt;
    if (get_cvs_endpt(&pt) != 0){
        if (verbose)
            printf("CVS not found.\n");
        return -1;
    }

    if (_syscall(pt, CVS_BROADCAST, &m) < 0) return -1;    
    return 0;
}



