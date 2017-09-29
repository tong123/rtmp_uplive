#ifndef POX_SYSTEM
#define POX_SYSTEM

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
int pox_system(const char *cmd_line);

#ifdef __cplusplus
}
#endif

#endif // POX_SYSTEM

