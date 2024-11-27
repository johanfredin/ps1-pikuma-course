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

	u_short shipstarttexture;
	u_short scenestarttexture;
  	u_short trackstarttexture;


	ResetNextPrim(GetCurrBuff());

	scenestarttexture = GetTextureCount();
	LoadTextureCMP("\\SCENE.CMP;1", NULL);
	// sceneobjs = LoadObjectPRMs("\\SCENE.PRM;1", scenestarttexture);
	
	shipstarttexture = GetTextureCount();
	LoadTextureCMP("\\ALLSH.CMP;1", NULL);
	ships = LoadObjectPRMs("\\ALLSH.PRM;1", shipstarttexture);
	ship = ships;
	
	
	trackstarttexture = GetTextureCount();
	LoadTextureCMP("\\LIBRARY.CMP;1", "\\LIBRARY.TTF;1");
	LoadTrackVertices(&track, "\\TRACK.TRV;1");
	LoadTrackFaces(&track, "\\TRACK.TRF;1", trackstarttexture);
	LoadTrackSections(&track, "\\TRACK.TRS;1");


	

	// Initialize ships position in the scene
	setVector(&ship->position, 32599, -347, -45310);

	// Initializes the cameras position
	setVector(&camera.position, ship->position.vx, ship->position.vy - 200, ship->position.vz - 2000);
	camera.lookat = (MATRIX){0};
	
}

static void Update(void) {
	EmptyOT(GetCurrBuff());

	JoyPadUpdate();

	if (JoyPadCheck(PAD1_LEFT)) {
		camera.position.vx -= 300;
	}
	if (JoyPadCheck(PAD1_RIGHT)) {
		camera.position.vx += 300;
	}
	if (JoyPadCheck(PAD1_UP)) {
		camera.position.vz += 300;
		ship->position.vz += 300;
	}
	if (JoyPadCheck(PAD1_DOWN)) {
		camera.position.vz -= 300;
		ship->position.vz -= 300;
	}

	CameraLookAt(&camera, &ship->position, &up);

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
