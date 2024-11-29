#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

#define vectorSquared(v) ((v)->vx * (v)->vx) + ((v)->vy * (v)->vy) + ((v)->vz * (v)->vz)

char *FileRead(char *filename, u_long *length);

char GetChar(u_char *bytes, u_long *b);

short GetShortBE(u_char *bytes, u_long *b);
short GetShortLE(u_char *bytes, u_long *b);

long GetLongBE(u_char *bytes, u_long *b);
long GetLongLE(u_char *bytes, u_long *b);



#endif