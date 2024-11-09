#ifndef OBJECT_H
#define OBJECT_H

#include "libgte.h"

typedef struct Object {
    SVECTOR *vertices;
    short numverts;
    short numfaces;
    short *faces;
    short numcolors;
    CVECTOR *colors;
} Object;

#endif