#include "ship.h"
#include "libgte.h"

#define SHIP_MASS_DEFAULT 150

void ShipInit(Ship *ship, Track *track, VECTOR *startpos) {
    ship->vel = (VECTOR){0, 0, 0};
    ship->acc = (VECTOR){0, 0, 0};

    ship->object->position.vx = startpos->vx;
    ship->object->position.vy = startpos->vy;
    ship->object->position.vz = startpos->vz;

    ship->thrust = (VECTOR) {0, 0, 0};
    ship->drag = (VECTOR) {0, 0, 0};

    ship->yaw = 0;
    ship->pitch = 0;
    ship->roll = 0;
    ship->velyaw = 0;
    ship->velpitch = 0;
    ship->velroll = 0;
    ship->accyaw = 0;
    ship->accpitch = 0;
    ship->accroll = 0;

    ship->speed = 0;
    ship->thrustmag = 0;
    ship->thrustmax = 700;

    ship->mass = SHIP_MASS_DEFAULT;
}

void ShipUpdate(Ship *ship) {
    // Lets pretend ship forward is always pointing towards +Z
    ship->forward = (VECTOR) {0, 0, ONE};

    // Compute the thrust force vector based on the thrust magnitude (that is being modified using the joypad)
    ship->thrust.vx = (ship->thrustmag * ship->forward.vx) >> 12;
    ship->thrust.vy = (ship->thrustmag * ship->forward.vy) >> 12;
    ship->thrust.vz = (ship->thrustmag * ship->forward.vz) >> 12;

    // The force is the sum of all forces (only thrust for now)
    VECTOR force = (VECTOR){0, 0, 0};
    force.vx += ship->thrust.vx;
    force.vy += ship->thrust.vy;
    force.vz += ship->thrust.vz;

    // The acceleration of the ship comes from F=ma (a=F/m)
    ship->acc.vx += (force.vx / ship->mass);
    ship->acc.vy += (force.vy / ship->mass);
    ship->acc.vz += (force.vz / ship->mass);

    // The velocity is based on the acceleration
    ship->vel.vx += ship->acc.vx;
    ship->vel.vy += ship->acc.vy;
    ship->vel.vz += ship->acc.vz;

    // The position is computed based on the velocity
    ship->object->position.vx += ship->vel.vx >> 6;
    ship->object->position.vy += ship->vel.vy >> 6;
    ship->object->position.vz += ship->vel.vz >> 6;
}