#ifndef _SIGNALS_H_
#define _SIGNALS_H_

#include <minix/type.h>
#include <minix/ipc.h>
#include "structures.h"

void handle_sig(message *);
void handle_exit(message *);

#endif /* !_SIGNALS_H_ */
