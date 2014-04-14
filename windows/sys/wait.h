#ifndef WAIT_H
#define WAIT_H 1
#include <io.h>

#define WUNTRACED        0x00000002
int __pipe(int*);              /* Prototype */
#define pipe __pipe
#include <unistd.h>
pid_t waitpid(pid_t, int*, int);

#endif