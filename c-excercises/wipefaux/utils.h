#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>

char *FileRead(char *filename, u_long *lenght);

short GetShortLE(char *bytes, u_long *i);
short GetShortBE(char *bytes, u_long *i);
char GetByteBE(char *bytes, u_long * i);
#endif