#ifndef _INC_
#define _INC_

/*===========================================================================*
 				            config			                                 
 *===========================================================================*/
#define _POSIX_SOURCE      1	/* Tell headers to include POSIX stuff */
#define _MINIX             1	/* Tell headers to include MINIX stuff */
#define _SYSTEM            1    /* Get OK and negative error codes */
#define MUTEX_NR           1024
#define COND_VAR_NR        MUTEX_NR

/*===========================================================================*
 				            includes		                                 
 *===========================================================================*/
#include <minix/com.h>
#include <minix/config.h>
#include <minix/ipc.h>
#include <minix/endpoint.h>
#include <minix/type.h>
#include <minix/sef.h>
#include <minix/sysutil.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

/*===========================================================================*
 				            handled requests                                 
 *===========================================================================*/
int do_lock(message *);
int do_unlock(message *);
int do_wait(message *);
int do_broadcast(message *);

#endif /* !_INC_ */
