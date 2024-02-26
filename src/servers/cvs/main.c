#include "inc.h"
#include "structures.h" 
#include "utils.h"
#include "signals.h"

static struct {
    int type;
    int (*func)(message *);
} vec_calls[] = {
    { CVS_LOCK,     do_lock,	 },
    { CVS_UNLOCK,   do_unlock,	 },
    { CVS_WAIT,     do_wait,	 },
    { CVS_BROADCAST,do_broadcast },
};

/* SEF functions and variables */
static void sef_local_startup(void);
static int sef_cb_init_fresh(int type, sef_init_info_t *info);
static void sef_cb_signal_handler(int signo);
static void struct_init();

static int verbose = 0;

/* globals alloc */
endpoint_t who_e;
int call_type;
endpoint_t SELF_E;

/*===========================================================================*
 		                       main                                          
 *===========================================================================*/
int main(int argc, char *argv[])
{
    message m;

    /* SEF local startup */
    env_setargs(argc, argv);
    sef_local_startup();
    struct_init();
    register_in_pm();

    while (TRUE) {

        int r;
        int call_number;

        if ((r = sef_receive(ANY, &m)) != OK)
            if (verbose) printf("sef_receive failed %d.\n", r);
        
        who_e = m.m_source;
        call_type = m.m_type;

        if (verbose) printf("CVS: get %d from %d\n", call_type, who_e);

        if (who_e == PM_PROC_NR) {
            switch (call_type) { /* Using the same types as VFS */
                case PM_UNPAUSE: 
                    handle_sig(&m);                    
                break;
                case PM_EXIT:
                case PM_DUMPCORE:
                    handle_exit(&m);
                break;
                default:
                if (verbose) printf("Unknown message from PM.\n");
            }
            continue;
        }
       

        /* See minix/com.h */
        call_number = call_type - CVS_BASE;

        /* Dispatch message */
        if (call_number >= 0 && call_number < SIZE(vec_calls)) {
            int result;

            if (vec_calls[call_number].type != call_type)
                panic("CVS: call table order mismatch");

            result = vec_calls[call_number].func(&m);

            if(verbose && result != OK && result != EDONTREPLY)
                printf("CVS: error for %d: %d\n", call_type, result);

            if (result != EDONTREPLY) reply(who_e, result);
        } 
        else {
            if (verbose) printf("CVS unknown call type: %d from %d.\n", call_type, who_e);
            reply(who_e, ENOSYS);
        }
    }

    /* No way to get here */
    return -1;
}

/*===========================================================================*
 			       sef_local_startup			                             
 *===========================================================================*/
static void sef_local_startup()
{
    /* Register init callbacks */
    sef_setcb_init_fresh(sef_cb_init_fresh);
    sef_setcb_init_restart(sef_cb_init_fresh);

    /* Register signal callbacks */
    sef_setcb_signal_handler(sef_cb_signal_handler);

    /* Let SEF perform startup */
    sef_startup();
}

static int sef_cb_init_fresh(int UNUSED(type), sef_init_info_t *UNUSED(info))
{
    /* Initialize the CVS */
    SELF_E = getprocnr();

    if(verbose)
        printf("CVS: self: %d\n", SELF_E);

    return(OK);
}

static void sef_cb_signal_handler(int signo)
{
    /* Only check for termination signal, ignore anything else */
    if (signo != SIGTERM) return;

    /* Checkout if there are still queues. Inform the user in that case */ 
    if (!mutex_is_clean() || !condv_is_clean())
        if (verbose) printf("CVS: exit with un-clean states.\n");
    unregister_in_pm();
}

/*===========================================================================*
 		                       struct_init                                   
 *===========================================================================*/
static void struct_init()
{
    mutex_init();
    condv_init();
    cursor_init();
}   


