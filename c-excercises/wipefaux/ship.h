#ifndef SHIP_H
#define SHIP_H

#include "libgte.h"
#include "object.h"
#include "track.h"

#define DRAW_AXIS 1

typedef struct Ship {
    Object *object;

    VECTOR vel;
    VECTOR acc;
    VECTOR right, up, forward;
    VECTOR thrust;
    VECTOR drag;

    short yaw, pitch, roll;

    short velyaw, velpitch, velroll;
    short accyaw, accpitch, accroll;

    short mass;
    long speed;
    long thrustmag;
    long thrustmax;

    Section *section; // The section we are closest to
} Ship;

void ShipInit(Ship *ship, Track *track, VECTOR *startpos, short mass, long thrustmax);
void ShipUpdate(Ship *ship);
void ShipDrawXYZAxis(Ship *ship, Camera *camera);
#endif