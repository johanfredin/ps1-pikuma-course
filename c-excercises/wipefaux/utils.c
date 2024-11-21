#include <libcd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define SECTOR 2048

char *FileRead(char *filename, u_long *length) {
	CdlFILE filepos;

	if (CdSearchFile(&filepos, filename) == NULL) {
		printf("%s file not found in the CD.", filename);
		exit(1);
	} 

	printf("Found %s in the CD.\n", filename);
	int numsectors = (filepos.size + 2047) / SECTOR;  		// compute the number of sectors to read from the file
	char *buffer = (char *)malloc3(SECTOR * numsectors);  	// allocate buffer for the file
	if (!buffer) {
		printf("Error allocating %d sectors, terminating...", numsectors);
		exit(1);
	}

	CdControl(CdlSetloc, (u_char *)&filepos.pos, 0);	 // set read target to the file
	CdRead(numsectors, (u_long *)buffer, CdlModeSpeed);	 // start reading from the CD
	CdReadSync(0, 0);									 		 // wait until the read is complete

	*length = filepos.size;
	return buffer;
}

char GetChar(u_char *bytes, u_long *b) { return bytes[(*b)++]; }

short GetShortLE(u_char *bytes, u_long *b) {
	u_short value = 0;
	value |= bytes[(*b)++] << 0;
	value |= bytes[(*b)++] << 8;
	return (short)value;
}

short GetShortBE(u_char *bytes, u_long *b) {
	u_short value = 0;
	value |= bytes[(*b)++] << 8;
	value |= bytes[(*b)++] << 0;
	return (short)value;
}

long GetLongLE(u_char *bytes, u_long *b) {
	u_long value = 0;
	value |= bytes[(*b)++] << 0;
	value |= bytes[(*b)++] << 8;
	value |= bytes[(*b)++] << 16;
	value |= bytes[(*b)++] << 24;
	return (long)value;
}

long GetLongBE(u_char *bytes, u_long *b) {
	u_long value = 0;
	value |= bytes[(*b)++] << 24;
	value |= bytes[(*b)++] << 16;
	value |= bytes[(*b)++] << 8;
	value |= bytes[(*b)++] << 0;
	return (long)value;
}