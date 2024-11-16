#include "globals.h"
#include <libgpu.h>

static u_long ot[2][OT_LENGTH];             // Ordering table holding pointers to sorted primitives
static char primbuff[2][PRIMBUF_LEN];       // Primitive buffer that holds the actual data for each primitive
static char *nextprim;                      // Pointer to the next primitive in the primitive buffer

void EmptyOT(u_short currbuff) {
    ClearOTagR(ot[currbuff], OT_LENGTH);
}

void SetOTAt(u_short currbuff, u_int i, u_long value) {
    ot[currbuff][i] = value;
}

u_long *GetOTAt(u_short currbuff, u_int i) {
    return &ot[currbuff][i];
}

void IncrementNextPrim(u_int size) {
    nextprim += size;
}

void SetNextPrim(char *value) {
    nextprim = value;
}

void ResetNextPrim(u_short currbuff) {
    nextprim = primbuff[currbuff];
}

char *GetNextPrim(void) {
    return nextprim;
}