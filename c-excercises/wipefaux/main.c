#include <libcd.h>
#include <libetc.h>
#include <malloc.h>
#include <stdio.h>

#include "camera.h"
#include "display.h"
#include "globals.h"
#include "joypad.h"
#include "libgpu.h"
#include "libgte.h"
#include "object.h"
#include "sys/types.h"
#include "texture.h"
#include "ship.h"
#include "track.h"

extern char __heap_start, __sp;

#define SHIP_MASS_DEFAULT 150
#define SHIP_SPEED 200
#define THRUST_MAG 3000
#define THRUST_REDUCTION 15000
#define THUST_MAX 20000
#define SHIP_VEL_YAW 256
#define SHIP_PITCH 6
#define MAX_VEL_YAW 2048

Camera camera;
Object *ships;
Ship ship;
Object *sceneobjs;

Track track;

VECTOR up = {0, -ONE, 0};

static void Setup(void) {
	// Initialize the heap
	InitHeap3((unsigned long *)&__heap_start, (&__sp - 0x5000) - &__heap_start);

	u_short shipstarttexture;
	u_short scenestarttexture;
	u_short trackstarttexture;

	ScreenInit();
	CdInit();
	JoyPadInit();

	ResetNextPrim(GetCurrBuff());

	// Load all textures for the ships
	shipstarttexture = GetTextureCount();
	LoadTextureCMP("\\ALLSH.CMP;1", NULL);

	// Load all textures for the scene objects
	scenestarttexture = GetTextureCount();
	// LoadTextureCMP("\\SCENE.CMP;1", NULL);

	// Load all primitives for all the ships (and returns pointer to the first object)
	ships = LoadObjectPRMs("\\ALLSH.PRM;1", shipstarttexture);

	// Load all primitives for all the scene objects (and returns pointer to the first scene object)
	// sceneobjs = LoadObjectPRM("\\TRACK02\\SCENE.PRM;1", scenestarttexture);

	// Load textures and tile information from the CD-ROM
	trackstarttexture = GetTextureCount();
	LoadTextureCMP("\\LIBRARY.CMP;1", "\\LIBRARY.TTF;1");

	// Load track vertices, faces, and sections
	LoadTrackVertices(&track, "\\TRACK.TRV;1");
	LoadTrackFaces(&track, "\\TRACK.TRF;1", trackstarttexture);
	LoadTrackSections(&track, "\\TRACK.TRS;1");

	// Initialize ship
	VECTOR startpos = {32599, -347, -45310};
	ship.object = ships->next;
	ShipInit(&ship, &track, &startpos, SHIP_MASS_DEFAULT, THUST_MAX);

	// Initializes camera position
	setVector(&camera.position, ship.object->position.vx, ship.object->position.vy - 700, ship.object->position.vz - 1600);
	camera.lookat = (MATRIX){0};
	camera.rotmat = (MATRIX){0};
}

static void Update(void) {
	EmptyOT(GetCurrBuff());

	JoyPadUpdate();

	if (JoyPadCheck(PAD1_LEFT)) {
		if (ship.velyaw <= 0) {
			ship.velyaw -= (SHIP_VEL_YAW >> 1);
		} else {
			ship.velyaw -= SHIP_VEL_YAW;
		}
	} else if (JoyPadCheck(PAD1_RIGHT)) {
		if (ship.velyaw >= 0) {
			ship.velyaw += (SHIP_VEL_YAW >> 1);
		} else {
			ship.velyaw += SHIP_VEL_YAW;
		}	
	} else {
		// Decay the velocity of the yaw to restore it 
		if (ship.velyaw > 0) {
			ship.velyaw -= (SHIP_VEL_YAW >> 1);
		} else if (ship.velyaw < 0) {
			ship.velyaw += (SHIP_VEL_YAW >> 1);
		}
	}
	// Clamp max velyaw
	if (ship.velyaw < -MAX_VEL_YAW) {
		ship.velyaw = -MAX_VEL_YAW;
	} else if(ship.velyaw > MAX_VEL_YAW) {
		ship.velyaw = MAX_VEL_YAW;
	}
	
	// Handle pitch
	if (JoyPadCheck(PAD1_UP)) {
		ship.pitch += SHIP_PITCH;
	}
	if (JoyPadCheck(PAD1_DOWN)) {
		ship.pitch -= SHIP_PITCH;
	}
	if (JoyPadCheck(PAD1_CROSS)) {
		ship.thrustmag += THRUST_MAG;
	} else if(ship.thrustmag > 0) {
		ship.thrustmag -= THRUST_REDUCTION;
		if (ship.thrustmag < 0) {
			ship.thrustmag = 0;
		}
		
	}

	if (ship.thrustmag > ship.thrustmax) {
		ship.thrustmag = ship.thrustmax;
	}

	ShipUpdate(&ship);
	
	// Force camera to always be behind ship
	camera.position.vx = ship.object->position.vx - (ship.forward.vx >> 2);
	camera.position.vy = ship.object->position.vy - (ship.forward.vy >> 2) + (up.vy >> 3);
	camera.position.vz = ship.object->position.vz - (ship.forward.vz >> 2);
	CameraLookAt(&camera, &ship.object->position, &up);

	// RenderSceneObjects(sceneobjs, &camera);
	RenderObject(ship.object, &camera);
	RenderTrackAhead(&track, ship.section, &camera);
	ShipDrawXYZAxis(&ship, &camera);
}

static void Render(void) { DisplayFrame(); }

int main(void) {
	Setup();
	while (1) {
		Update();
		Render();
	}
	return 0;
}
