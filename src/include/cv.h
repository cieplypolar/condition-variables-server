#ifndef _MUTEX_H_
#define _MUTEX_H_

int cs_lock(int);
int cs_unlock(int);
int cs_wait(int, int);
int cs_broadcast(int);

#endif /* !_MUTEX_H_ */
