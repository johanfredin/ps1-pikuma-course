#include <libetc.h>
#include <libcd.h>
#include <malloc.h>

#include "globals.h"
#include "joypad.h"
#include "display.h"
#include "object.h"
#include "texture.h"

extern char __heap_start, __sp;
Camera camera;
Object object;

VECTOR up = {0, -ONE, 0};

static void Setup(void) {
	// Initialize the heap
	InitHeap3((unsigned long *) &__heap_start, (&__sp - 0x5000) - &__heap_start);

	ScreenInit();
	CdInit();
	JoyPadInit();

	ResetNextPrim(GetCurrentBuff());

	setVector(&camera.position, 0, -600, -900);
	camera.lookat = (MATRIX){0};

	LoadTexture("\\ALLSH.CMP;1");	
	// LoadObjectPRM(&object, "\\ALLSH.PRM;1");
	
}

static void Update(void) {
  EmptyOT(GetCurrentBuff());

  JoyPadUpdate();

  if (JoyPadCheck(PAD1_LEFT)) {
    object.rotation.vy -= 15;
  }
  if (JoyPadCheck(PAD1_RIGHT)) {
    object.rotation.vy += 15;
  }

  CameraLookAt(&camera, &object.position, &(VECTOR){0, -ONE, 0});

  RenderObject(&object, &camera);
}


static void Render(void) {
	DisplayFrame();
}

int main(void) {
	Setup();
	while(1) {
		Update();
		Render();
	}
	return 0;
}
