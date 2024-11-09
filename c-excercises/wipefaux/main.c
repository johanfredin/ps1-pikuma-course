#include <libetc.h>
#include <stdio.h>

#include "globals.h"
#include "camera.h"
#include "inline_n.h"
#include "joypad.h"
#include "display.h"
#include "libcd.h"
#include "libgte.h"
#include "malloc.h"
#include "object.h"
#include "sys/types.h"
#include "utils.h"

extern char __heap_start, __sp;

///////////////////////////////////////////////////////////////////////////////
// Vertices and face indices
///////////////////////////////////////////////////////////////////////////////
typedef struct Cube {
	SVECTOR rotation;
	VECTOR position;
	VECTOR scale;
	VECTOR vel;
	VECTOR acc;
	SVECTOR vertices[8];
	short faces[24];
} Cube;

///////////////////////////////////////////////////////////////////////////////
// Declarations and global variables
///////////////////////////////////////////////////////////////////////////////

POLY_G4 *polyg4;
POLY_F3 *polyf3;

MATRIX worldmat = {0};
MATRIX viewmat = {0};
Camera camera;

Object object;

VECTOR UP = {0, -ONE, 0};

// Cube cube = {{0, 0, 0},
// 			 {0, -400, 1800},
// 			 {ONE, ONE, ONE},
// 			 {0, 0, 0},
// 			 {0, 1, 0},
// 			 {
// 				 {-128, -128, -128},
// 				 {128, -128, -128},
// 				 {128, -128, 128},
// 				 {-128, -128, 128},
// 				 {-128, 128, -128},
// 				 {128, 128, -128},
// 				 {128, 128, 128},
// 				 {-128, 128, 128},
// 			 },
// 			 {
// 				 3, 2, 0, 1, 0, 1, 4, 5, 4, 5, 7, 6, 1, 2, 5, 6, 2, 3, 6, 7, 3, 0, 7, 4,
// 			 }};


Cube cube = {{0, 0, 0},
			 {0, -400, 1800},
			 {ONE, ONE, ONE},
			 {0, 0, 0},
			 {0, 1, 0},
			 };


///////////////////////////////////////////////////////////////////////////////
// Setup function that is called once at the beginning of the execution
///////////////////////////////////////////////////////////////////////////////
void Setup(void) {
	// Initialize the heap
	InitHeap3((unsigned long *) &__heap_start, (&__sp - 0x5000) - &__heap_start);

	// Setup the display environment
	ScreenInit();

	// Initialize the joypad
	JoyPadInit();

	CdInit();

	// Reset next primitive pointer to the start of the primitive buffer
	ResetNextPrim(GetCurrentBuff());

	// Initialize the camera object
	camera.position.vx = 500;
	camera.position.vy = -1000;
	camera.position.vz = -1500;
	camera.lookat = (MATRIX){0};

	u_long lenght;
	char *bytes = FileRead("\\MODEL.BIN;1",  &lenght);

	u_long b = 0; // counter of bytes
	object.numverts = GetShortBE(bytes, &b);
	object.vertices = calloc3(object.numverts, sizeof(SVECTOR));
	for (int i = 0; i < object.numverts; i++) {
		object.vertices[i].vx = GetShortBE(bytes, &b);
		object.vertices[i].vy = GetShortBE(bytes, &b);
		object.vertices[i].vz = GetShortBE(bytes, &b);
		printf("VERTEX %d x:%d, y:%d, z:%d\n", i, object.vertices[i].vx, object.vertices[i].vy, object.vertices[i].vz);
	}
	object.numfaces = GetShortBE(bytes, &b) * 4; // 4 indices per face;
	object.faces = calloc3(object.numfaces, sizeof(short));
	for(int i = 0; i < object.numfaces; i++) {
		object.faces[i] = GetShortBE(bytes, &b);
		printf("Face: %d\n", object.faces[i]);
	}
	object.numcolors = GetByteBE(bytes, &b);
	printf("Num colors: %d\n", object.numcolors);
	object.colors = calloc3(object.numcolors, sizeof(CVECTOR));
	for (int i = 0; i < object.numcolors; i++) {
		object.colors[i].r = (u_char) GetByteBE(bytes, &b);
		object.colors[i].g = (u_char) GetByteBE(bytes, &b);
		object.colors[i].b = (u_char) GetByteBE(bytes, &b); 
		object.colors[i].cd = (u_char) GetByteBE(bytes, &b);
		printf("COLOR %d, r:%d, g:%d, b:%d\n", i, object.colors[i].r, object.colors[i].g, object.colors[i].b);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Setup function that is called once at the beginning of the execution
///////////////////////////////////////////////////////////////////////////////
void Update(void) {
	int i, nclip;
	long otz, p, flg;

	// Empty the Ordering Table
	EmptyOT(GetCurrentBuff());

	// Update the state of the controller
	JoyPadUpdate();
	if (JoyPadCheck(PAD1_LEFT)) {
		camera.position.vx -= 50;
	}
	if (JoyPadCheck(PAD1_RIGHT)) {
		camera.position.vx += 50;
	}
	if (JoyPadCheck(PAD1_UP)) {
		camera.position.vy -= 50;
	}
	if (JoyPadCheck(PAD1_DOWN)) {
		camera.position.vy += 50;
	}
	if (JoyPadCheck(PAD1_CROSS)) {
		camera.position.vz += 50;
	}
	if (JoyPadCheck(PAD1_CIRCLE)) {
		camera.position.vz -= 50;
	}

	// Compute the lookat matrix for this frame
	// CameraLookAt(&camera, &cube.position, &UP);

	//---------------
	// Draw the Cube 
	//---------------

	// Apply rotation, translation and scaling of the cube in the world space
	RotMatrix(&cube.rotation, &worldmat);
	TransMatrix(&worldmat, &cube.position);
	ScaleMatrix(&worldmat, &cube.scale);

	// // Create the view matrix combining the world matrix and the lookat matrix
	CompMatrixLV(&camera.lookat, &worldmat, &viewmat);

	SetRotMatrix(&viewmat);
	SetTransMatrix(&viewmat);

	// Loop all face indices of our cube faces
	for (i = 0; i < object.numfaces * 4; i += 4) {
		polyg4 = (POLY_G4 *)GetNextPrim();
		CVECTOR color = object.colors[i];
		setPolyG4(polyg4);
		setRGB0(polyg4, 255, 0, 255);
		setRGB1(polyg4, 255, 255, 0);
		setRGB2(polyg4, 0, 255, 255);
		setRGB3(polyg4, 0, 255, 0);

		// This is how the gte handles quads

		// Load the first 3 vertices (the GTE can only perform max 3 vertices at a time)
		gte_ldv0(&cube.vertices[cube.faces[i + 0]]);
		gte_ldv1(&cube.vertices[cube.faces[i + 1]]);
		gte_ldv2(&cube.vertices[cube.faces[i + 2]]);

		// Perform rotation, translation and projection
		gte_rtpt();

		// Perform normal clipping and store result in nclip
		gte_nclip();
		gte_stotz(&nclip);

		if (nclip >= 0) {
			// Store the transformed projected coord of the first vertex
			gte_stsxy0(&polyg4->x0);

			// Load the last 4th vertex
			gte_ldv0(&cube.vertices[cube.faces[i + 3]]);

			// Project & transform the remaining 4th vertex
			gte_rtps();

			// Store the transformed last vertices
			gte_stsxy3(&polyg4->x1, &polyg4->x2, &polyg4->x3);

			gte_avsz4();
			gte_stotz(&otz);

			// Sort the quad in the OT
			if ((otz > 0) && (otz < OT_LENGTH)) {
				addPrim(GetOTAt(GetCurrentBuff(), otz), polyg4);
				IncrementNextPrim(sizeof(POLY_G4));
			}
		}
	}

}

///////////////////////////////////////////////////////////////////////////////
// Render function that invokes the draw of the current frame
///////////////////////////////////////////////////////////////////////////////
void Render(void) { DisplayFrame(); }

///////////////////////////////////////////////////////////////////////////////
// Main function
///////////////////////////////////////////////////////////////////////////////
int main(void) {
	Setup();
	while (1) {
		Update();
		Render();
	}
	return 0;
}