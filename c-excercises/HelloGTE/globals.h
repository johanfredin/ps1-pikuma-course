#ifndef GLOBALS_H
#define GLOBALS_H

#include <sys/types.h>

#define OT_LENGTH 0x2000
#define PRIMBUF_LEN 0x20000

void EmptyOT(u_short currbuff);
void SetOTAt(u_short currbuff, u_int i, u_long value);
u_long *GetOTAt(u_short currbuff, u_int i);

void IncrementNextPrim(u_int size);
void SetNextPrim(char *value);
void ResetNextPrim(u_short currbuff);
char *GetNextPrim(void);

#endif
