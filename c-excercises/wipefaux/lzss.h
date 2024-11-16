#ifndef LZSS_H
#define LZSS_H

#define INDEX_BIT_COUNT		13
#define LENGTH_BIT_COUNT  4
#define WINDOW_SIZE       (1 << INDEX_BIT_COUNT)
#define BREAK_EVEN			  ((1 + INDEX_BIT_COUNT + LENGTH_BIT_COUNT ) /9 )
#define END_OF_STREAM		  0
#define MOD_WINDOW(a)		  ((a) & (WINDOW_SIZE - 1))

void ExpandLZSSData(unsigned char *indata, unsigned char *outdata);

#endif