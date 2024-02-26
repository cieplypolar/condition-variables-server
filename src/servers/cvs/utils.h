#ifndef _UTILS_H_
#define _UTILS_H_

#include <minix/type.h>

#define SIZE(a) (sizeof(a)/sizeof(a[0]))

void reply(endpoint_t, int);
int mutex_unlock(endpoint_t, int, int);
int mutex_enqueue(struct proc, position);
void mutex_remove_proc(endpoint_t, queue, int);
void condv_remove_proc(endpoint_t, queue, int);
void register_in_pm();
void unregister_in_pm();

/* OPERATIONS in mutex_unlock because of different errno values in unlock and wait */
#define WAIT 1882
#define UNLOCK 1929

#endif /* !_UTILS_H_ */
