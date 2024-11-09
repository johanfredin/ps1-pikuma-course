#include "utils.h"

#include <libcd.h>
#include <stdio.h>
#include <stdlib.h>
#include "sys/types.h"

#define SECTOR 2048

char *FileRead(char *filename, u_long *lenght) {
    CdlFILE filepos;
    
    // Search for the file in the cd
    if (CdSearchFile(&filepos, filename) == NULL) {
        printf("File=%s not found, terminating...\n", filename);
        exit(1);
    }

    int numsectors = (filepos.size + (SECTOR - 1)) / SECTOR;                    // Compute the number of sectors to read from the file
    char *buffer = (char *) malloc3(SECTOR * numsectors);                       // Allocate buffer for the file (must be a multiple of 2048)

    CdControl(CdlSetloc, (u_char*) &filepos.pos, 0);         // Set read target to the file position on the CD
    CdRead(numsectors, (u_long*) buffer, CdlModeSpeed);      // Start reading from the CD
    CdReadSync(0, 0);                                             // Wait until the read is complete
    
    *lenght = filepos.size;                                                     // Return a param with how many bytes we read from the file   
    printf("File read=%s, size=%lu bytes\n", filename, *lenght);
    return buffer;
}

short GetShortLE(char *bytes, u_long *i) {
    unsigned short value = 0;
    value |= bytes[(*i)++] << 0;
    value |= bytes[(*i)++] << 8;
    return (short) value;
}

short GetShortBE(char *bytes, u_long *i) {
    unsigned short value = 0;
    value |= bytes[(*i)++] << 8;
    value |= bytes[(*i)++] << 0;
    return (short) value;
}

char GetByteBE(char *bytes, u_long * i) {
    unsigned char value = 0;
    value = bytes[(*i)++];
    return (char) value;
}