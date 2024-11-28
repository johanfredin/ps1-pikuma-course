#include <libcd.h>
#include <libetc.h>
#include <malloc.h>
#include <stdio.h>

#include "camera.h"
#include "display.h"
#include "globals.h"
#include "joypad.h"
#include "libgpu.h"
#include "object.h"
#include "sys/types.h"
#include "texture.h"
#include "track.h"

extern char __heap_start, __sp;

Camera camera;
Object *ships, *ship;
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

	// Load the current ship object from the linked list
	ship = ships->next;

	// Initialize ship's position in the scene
	setVector(&ship->position, 32599, -347, -45310);

	// Initializes camera position
	setVector(&camera.position, ship->position.vx, ship->position.vy - 300, ship->position.vz - 1000);
	camera.lookat = (MATRIX){0};
	camera.rotmat = (MATRIX){0};
}

static void Update(void) {
	EmptyOT(GetCurrBuff());

	JoyPadUpdate();

	if (JoyPadCheck(PAD1_LEFT)) {
		camera.position.vx -= 10;
	}

	if (JoyPadCheck(PAD1_RIGHT)) {
		camera.position.vx += 10;
	}

	if (JoyPadCheck(PAD1_UP)) {
		camera.position.vz += 100;
		ship->position.vz += 100;
	}

	if (JoyPadCheck(PAD1_DOWN)) {
		camera.position.vz -= 100;
		ship->position.vz -= 100;
	}

	CameraLookAt(&camera, &ship->position, &(VECTOR){0, -ONE, 0});

	// RenderSceneObjects(sceneobjs, &camera);
	RenderObject(ship, &camera);
	RenderTrack(&track, &camera);
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
