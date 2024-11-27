#include <libcd.h>
#include <libetc.h>
#include <malloc.h>
#include <stdio.h>

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

	ScreenInit();
	CdInit();
	JoyPadInit();

	ResetNextPrim(GetCurrBuff());

	u_short shipstarttexture = GetTextureCount();
	LoadTextureCMP("\\ALLSH.CMP;1");
	ships = LoadObjectPRMs("\\ALLSH.PRM;1", shipstarttexture);
	ship = ships;

	// Load track vertices, faces and sections
	LoadTrackVertices(&track, "\\TRACK.TRV;1");
	LoadTrackFaces(&track, "\\TRACK.TRF;1");
	LoadTrackSections(&track, "\\TRACK.TRS;1");


	u_short scenestarttexture = GetTextureCount();
	LoadTextureCMP("\\SCENE.CMP;1");
	// sceneobjs = LoadObjectPRMs("\\SCENE.PRM;1", scenestarttexture);

	// Initialize ships position in the scene
	setVector(&ship->position, 32599, -347, -45310);

	// Initializes the cameras position
	setVector(&camera.position, ship->position.vx, ship->position.vy - 100, ship->position.vz - 100);
	camera.lookat = (MATRIX){0};
}

static void Update(void) {
	EmptyOT(GetCurrBuff());

	JoyPadUpdate();

	if (JoyPadCheck(PAD1_LEFT)) {
		camera.position.vx -= 100;
	}
	if (JoyPadCheck(PAD1_RIGHT)) {
		camera.position.vx += 100;
	}
	if (JoyPadCheck(PAD1_UP)) {
		camera.position.vz += 100;
	}
	if (JoyPadCheck(PAD1_DOWN)) {
		camera.position.vz += 100;
	}

	CameraLookAt(&camera, &ship->position, &up);

	RenderTrack(&track, &camera);
	RenderObject(ship, &camera);
	// for (Object *sceneobj = sceneobjs; sceneobj != NULL; sceneobj = sceneobj->next) {
	// 	RenderObject(sceneobj, &camera);
	// }
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
