#include "ship.h"
#include "utils.h"

#if DRAW_AXIS
#include <stddef.h>
#include "display.h"
#include "globals.h"
#include "libgpu.h"
# endif
#include "libgte.h"


void ShipInit(Ship *ship, Track *track, VECTOR *startpos, short mass, long thrustmax) {
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
    ship->thrustmax = thrustmax;

    ship->mass = mass;
}

void ShipUpdate(Ship *ship) {
	short sinx = rsin(ship->pitch);
	short cosx = rcos(ship->pitch);
	short siny = rsin(ship->yaw);
	short cosy = rcos(ship->yaw);
	short sinz = rsin(ship->roll);
	short cosz = rcos(ship->roll);

	// Compute the right, up, and forward vectors with the orientation of the ship
	ship->right.vx = ((cosy * cosz) >> 12) + ((((siny * sinx) >> 12) * sinz) >> 12);
	ship->right.vy = (cosx * sinz) >> 12;
	ship->right.vz = ((-siny * cosz) >> 12) + ((((cosy * sinx) >> 12) * sinz) >> 12);

	ship->up.vx = ((-sinz * cosy) >> 12) + ((((siny * sinx) >> 12) * cosz) >> 12);
	ship->up.vy = (cosx * cosz) >> 12;
	ship->up.vz = ((-siny * -sinz) >> 12) + ((((cosy * sinx) >> 12) * cosz) >> 12);

	ship->forward.vx = (siny * cosx) >> 12;
	ship->forward.vy = (-sinx);
	ship->forward.vz = (cosy * cosx) >> 12;

	// Compute the thrust force vector based on the thrust magnitude (that is being modified using the joypad)
    ship->thrust.vx = (ship->thrustmag * ship->forward.vx) >> 12;
    ship->thrust.vy = (ship->thrustmag * ship->forward.vy) >> 12;
    ship->thrust.vz = (ship->thrustmag * ship->forward.vz) >> 12;

    // Compute the length of the velocity   
    ship->speed = SquareRoot0(vectorSquared(&ship->vel));

    // Compute the forward velocity
    VECTOR nosevel = {
        (ship->speed * ship->forward.vx) >> 12, 
        (ship->speed * ship->forward.vy) >> 12, 
        (ship->speed * ship->forward.vz) >> 12
    };

    // The force is the sum of all forces (only thrust for now)
    VECTOR force = (VECTOR){0, 0, 0};
    force.vx += ship->thrust.vx;
    force.vy += ship->thrust.vy;
    force.vz += ship->thrust.vz;

     // Set the acceleration in the direction of the new nose/forward axis
	ship->acc.vx = (nosevel.vx - ship->vel.vx);
	ship->acc.vy = (nosevel.vy - ship->vel.vy);
	ship->acc.vz = (nosevel.vz - ship->vel.vz);

	// The acceleration of the ship comes from F=ma (a=F/m)
	ship->acc.vx += force.vx / ship->mass;
	ship->acc.vy += force.vy / ship->mass;
	ship->acc.vz += force.vz / ship->mass;

	// The velocity is computed based on the acceleration
	ship->vel.vx += ship->acc.vx;
	ship->vel.vy += ship->acc.vy;
	ship->vel.vz += ship->acc.vz;

	// The new position is computed based on the velocity
	ship->object->position.vx += ship->vel.vx >> 6;
	ship->object->position.vy += ship->vel.vy >> 6;
	ship->object->position.vz += ship->vel.vz >> 6;

	// Apply a roll movement based on the ship's angular yaw velocity
	ship->velroll = ship->velyaw >> 4;

	// Reduce/decay the roll velocity to restore it to the default orientation
	ship->velroll -= ship->velroll >> 1;

	// Update the yaw, pitch, and roll by their velocity
	ship->yaw += ship->velyaw >> 6;
	ship->pitch += ship->velpitch;
	ship->roll += ship->velroll >> 1;

	// Restores/reduces the roll so the ship's canopy always points up
	ship->roll -= ship->roll >> 3;

	// Set the ship->object orientation matrix (order of rotation: Yaw --> Pitch --> Roll)
	ship->object->rotmat.m[0][0] = ship->right.vx;
	ship->object->rotmat.m[1][0] = ship->right.vy;
	ship->object->rotmat.m[2][0] = ship->right.vz;

	ship->object->rotmat.m[0][1] = ship->up.vx;
	ship->object->rotmat.m[1][1] = ship->up.vy;
	ship->object->rotmat.m[2][1] = ship->up.vz;

	ship->object->rotmat.m[0][2] = ship->forward.vx;
	ship->object->rotmat.m[1][2] = ship->forward.vy;
	ship->object->rotmat.m[2][2] = ship->forward.vz;
}

void ShipDrawXYZAxis(Ship *ship, Camera *camera) {
#if DRAW_AXIS
  SVECTOR v0, v1, v2;
  int i;
  long otz;
  POLY_F3 *polya, *polyb, *polyc, *polyd;
  LINE_F2 lineA, lineB, lineC;

  VECTOR vec;
  SVECTOR rot;
  MATRIX worldmat;
  MATRIX viewmat;

  rot.vx = 0;
  rot.vy = 0;
  rot.vz = 0;
  RotMatrix(&rot, &worldmat);
  TransMatrix(&worldmat, &ship->object->position);
  ScaleMatrix(&worldmat, &ship->object->scale);
  CompMatrixLV(&camera->lookat, &worldmat, &viewmat);
  SetRotMatrix(&viewmat);
  SetTransMatrix(&viewmat);

  polya = (POLY_F3*) GetNextPrim();
  SetPolyF3(polya);
  v0.vx = (short) (0);
  v0.vy = (short) (0);
  v0.vz = (short) (0);
  v1 = v2 = v0;
  otz  = RotTransPers(&v0, (long*)&polya->x0, NULL, NULL);
  otz += RotTransPers(&v1, (long*)&polya->x1, NULL, NULL);
  otz += RotTransPers(&v2, (long*)&polya->x2, NULL, NULL);
  otz /= 3;
  setRGB0(polya, 255, 255, 255);
  addPrim(GetOTAt(GetCurrBuff(), otz), polya);
  IncrementNextPrim(sizeof(POLY_FT3));

  polyb = (POLY_F3*) GetNextPrim();
  SetPolyF3(polyb);
  v0.vx = (short) (ship->forward.vx >> 3);
  v0.vy = (short) (ship->forward.vy >> 3);
  v0.vz = (short) (ship->forward.vz >> 3);
  v1 = v2 = v0;
  otz  = RotTransPers(&v0, (long*)&polyb->x0, NULL, NULL);
  otz += RotTransPers(&v1, (long*)&polyb->x1, NULL, NULL);
  otz += RotTransPers(&v2, (long*)&polyb->x2, NULL, NULL);
  otz /= 3;
  setRGB0(polyb, 0, 0, 0);
  addPrim(GetOTAt(GetCurrBuff(), otz), polyb);
  IncrementNextPrim(sizeof(POLY_FT3));

  polyc = (POLY_F3*) GetNextPrim();
  SetPolyF3(polyc);
  v0.vx = (short) (ship->right.vx >> 4);
  v0.vy = (short) (ship->right.vy >> 4);
  v0.vz = (short) (ship->right.vz >> 4);
  v1 = v2 = v0;
  otz  = RotTransPers(&v0, (long*)&polyc->x0, NULL, NULL);
  otz += RotTransPers(&v1, (long*)&polyc->x1, NULL, NULL);
  otz += RotTransPers(&v2, (long*)&polyc->x2, NULL, NULL);
  otz /= 3;
  setRGB0(polyc, 0, 0, 0);
  addPrim(GetOTAt(GetCurrBuff(), otz), polyc);
  IncrementNextPrim(sizeof(POLY_FT3));

  polyd = (POLY_F3*) GetNextPrim();
  SetPolyF3(polyd);
  v0.vx = (short) -(ship->up.vx >> 4);
  v0.vy = (short) -(ship->up.vy >> 4);
  v0.vz = (short) -(ship->up.vz >> 4);
  v1 = v2 = v0;
  otz  = RotTransPers(&v0, (long*)&polyd->x0, NULL, NULL);
  otz += RotTransPers(&v1, (long*)&polyd->x1, NULL, NULL);
  otz += RotTransPers(&v2, (long*)&polyd->x2, NULL, NULL);
  otz /= 3;
  setRGB0(polyd, 0, 0, 0);
  addPrim(GetOTAt(GetCurrBuff(), otz), polyd);
  IncrementNextPrim(sizeof(POLY_FT3));

  // Draw blue line from origin to forward
  SetLineF2(&lineA);
  setXY2(&lineA, polya->x0, polya->y0, polyb->x0, polyb->y0);
  setRGB0(&lineA, 55, 150, 255);
  DrawPrim(&lineA);

  // Draw red line from origin to right
  SetLineF2(&lineB);
  setXY2(&lineB, polya->x0, polya->y0, polyc->x0, polyc->y0);
  setRGB0(&lineB, 255, 55, 87);
  DrawPrim(&lineB);

  // Draw green line from origin to up
  SetLineF2(&lineC);
  setXY2(&lineC, polya->x0, polya->y0, polyd->x0, polyd->y0);
  setRGB0(&lineC, 10, 255, 110);
  DrawPrim(&lineC);
#endif
}
