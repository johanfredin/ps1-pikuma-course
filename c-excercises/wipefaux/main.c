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
Object ship;
Object rescu;

VECTOR up = {0, -ONE, 0};

static void Setup(void) {
	// Initialize the heap
	InitHeap3((unsigned long *) &__heap_start, (&__sp - 0x5000) - &__heap_start);

	ScreenInit();
	CdInit();
	JoyPadInit();

	ResetNextPrim(GetCurrBuff());

	setVector(&camera.position, 0, -600, -900);
	camera.lookat = (MATRIX){0};

	u_short shipstarttexture = GetTextureCount();
	LoadTextureCMP("\\ALLSH.CMP;1");	
	LoadObjectPRM(&ship, "\\ALLSH.PRM;1", shipstarttexture);

	u_short rescustarttexture = GetTextureCount();
	LoadTextureCMP("\\RESCU.CMP;1");	
	LoadObjectPRM(&rescu, "\\RESCU.PRM;1", rescustarttexture);
	
}

static void Update(void) {
  EmptyOT(GetCurrBuff());

  JoyPadUpdate();

  if (JoyPadCheck(PAD1_LEFT)) {
    rescu.rotation.vy -= 15;
  }
  if (JoyPadCheck(PAD1_RIGHT)) {
    rescu.rotation.vy += 15;
  }

  CameraLookAt(&camera, &rescu.position, &(VECTOR){0, -ONE, 0});

  RenderObject(&rescu, &camera);
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
