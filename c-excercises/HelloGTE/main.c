#include <sys/types.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include "inline_n.h"
#include "joypad.h"

#define VIDEO_MODE 0
#define SCREEN_RES_X 320
#define SCREEN_RES_Y 240
#define SCREEN_CENTER_X (SCREEN_RES_X >> 1)
#define SCREEN_CENTER_Y (SCREEN_RES_Y >> 1)
#define SCREEN_Z 320

#define OT_LENGTH 2048

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

typedef struct Ground {
  SVECTOR rotation;
  VECTOR position;
  VECTOR scale;
  SVECTOR vertices[4];
  short faces[6];
} Ground;

///////////////////////////////////////////////////////////////////////////////
// Declarations and global variables
///////////////////////////////////////////////////////////////////////////////
typedef struct {
  DRAWENV draw[2];
  DISPENV disp[2];
} DoubleBuff;

DoubleBuff screen;           // Struct to hold the display & draw buffers
u_short currbuff;            // Holds the current buffer number (0 or 1)

u_long ot[2][OT_LENGTH];     // Ordering table holding pointers to sorted primitives

char primbuff[2][2048];      // Primitive buffer that holds the actual data for each primitive
char *nextprim;              // Pointer to the next primitive in the primitive buffer

POLY_G4 *polyg4;
POLY_F3 *polyf3;

MATRIX world = {0};

Cube cube = {
  {0, 0, 0},
  {0, -400, 1800},
  {ONE, ONE, ONE},
  {0, 0, 0},
  {0, 1, 0},
  {
    { -128, -128, -128 },
    {  128, -128, -128 },
    {  128, -128,  128 },
    { -128, -128,  128 },
    { -128,  128, -128 },
    {  128,  128, -128 },
    {  128,  128,  128 },
    { -128,  128,  128 },
  },
  {
    3, 2, 0, 1,
    0, 1, 4, 5,
    4, 5, 7, 6,
    1, 2, 5, 6,
    2, 3, 6, 7,
    3, 0, 7, 4,
  }
};

Ground ground = {
  {0, 0, 0},
  {0, 450, 1800},
  {ONE, ONE, ONE},
  {
    {-900, 0, -900},
    {-900, 0,  900},
    { 900, 0, -900},
    { 900, 0,  900},
  },
  {
    0, 1, 2,
    1, 3, 2,
  }
};

///////////////////////////////////////////////////////////////////////////////
// Initialize the display mode and setup double buffering
///////////////////////////////////////////////////////////////////////////////
void ScreenInit(void) {
  // Reset GPU
  ResetGraph(0);

  // Set the display area of the first buffer
  SetDefDispEnv(&screen.disp[0], 0,   0, SCREEN_RES_X, SCREEN_RES_Y);
  SetDefDrawEnv(&screen.draw[0], 0, 240, SCREEN_RES_X, SCREEN_RES_Y);

  // Set the display area of the second buffer
  SetDefDispEnv(&screen.disp[1], 0, 240, SCREEN_RES_X, SCREEN_RES_Y);
  SetDefDrawEnv(&screen.draw[1], 0,   0, SCREEN_RES_X, SCREEN_RES_Y);

  // Set the back/drawing buffer
  screen.draw[0].isbg = 1;
  screen.draw[1].isbg = 1;

  // Set the background clear color
  setRGB0(&screen.draw[0], 63, 0, 127); // dark purple
  setRGB0(&screen.draw[1], 63, 0, 127); // dark purple

  // Set the current initial buffer
  currbuff = 0;
  PutDispEnv(&screen.disp[currbuff]);
  PutDrawEnv(&screen.draw[currbuff]);

  // Initialize and setup the GTE geometry offsets
  InitGeom();
  SetGeomOffset(SCREEN_CENTER_X, SCREEN_CENTER_Y);
  SetGeomScreen(SCREEN_Z);

  // Enable display
  SetDispMask(1);
}	

///////////////////////////////////////////////////////////////////////////////
// Draw the current frame primitives in the ordering table
///////////////////////////////////////////////////////////////////////////////
void DisplayFrame(void) {
  // Sync and wait for vertical blank
  DrawSync(0);
  VSync(0);

  // Set the current display & draw buffers
  PutDispEnv(&screen.disp[currbuff]);
  PutDrawEnv(&screen.draw[currbuff]);

  // Draw the ordering table for the current buffer
  DrawOTag(ot[currbuff] + OT_LENGTH - 1);

  // Swap current buffer
  currbuff = !currbuff;

  // Reset next primitive pointer to the start of the primitive buffer
  nextprim = primbuff[currbuff];
}

///////////////////////////////////////////////////////////////////////////////
// Setup function that is called once at the beginning of the execution
///////////////////////////////////////////////////////////////////////////////
void Setup(void) {
  // Setup the display environment
  ScreenInit();

  // Initialize the joypad
  JoyPadInit();

  // Reset next primitive pointer to the start of the primitive buffer
  nextprim = primbuff[currbuff];
}

///////////////////////////////////////////////////////////////////////////////
// Setup function that is called once at the beginning of the execution
///////////////////////////////////////////////////////////////////////////////
void Update(void) {
  int i, nclip;
  long otz, p, flg;

  // Empty the Ordering Table
  ClearOTagR(ot[currbuff], OT_LENGTH);

  // Update the state of the controller
  JoyPadUpdate();
  if (JoyPadCheck(PAD1_LEFT)) {
	cube.rotation.vy += 20;
  }
  if (JoyPadCheck(PAD1_RIGHT)) {
	cube.rotation.vy -= 20;
  }

  // Update the cube velocity based on its acceleration
  cube.vel.vx += cube.acc.vx;
  cube.vel.vy += cube.acc.vy;
  cube.vel.vz += cube.acc.vz;

  // Update the cube position based on its velocity
  cube.position.vx += (cube.vel.vx) >> 1;
  cube.position.vy += (cube.vel.vy) >> 1;
  cube.position.vz += (cube.vel.vz) >> 1;

  // Check "collision" with the ground
  if (cube.position.vy + 150 > ground.position.vy) {
    cube.vel.vy *= -1;
  }

  //--------------------------
  // Draw the Cube Quads
  //--------------------------
  RotMatrix(&cube.rotation, &world);
  TransMatrix(&world, &cube.position);
  ScaleMatrix(&world, &cube.scale);
  SetRotMatrix(&world);
  SetTransMatrix(&world);

  // Loop all face indices of our cube faces
  for (i = 0; i < 24; i += 4) {
    polyg4 = (POLY_G4*) nextprim;
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
			addPrim(ot[currbuff][otz], polyg4);
			nextprim += sizeof(POLY_G4);
		}
	}
  }

  //--------------------------
  // Draw the Floor Triangles
  //--------------------------
  RotMatrix(&ground.rotation, &world);
  TransMatrix(&world, &ground.position);
  ScaleMatrix(&world, &ground.scale);
  SetRotMatrix(&world);
  SetTransMatrix(&world);

  for (i = 0; i < 6; i += 3) {
    polyf3 = (POLY_F3*) nextprim;
    setPolyF3(polyf3);
    setRGB0(polyf3, 255, 255, 0);	
	
	// Load vertices
	gte_ldv0(&ground.vertices[ground.faces[i + 0]]);
	gte_ldv1(&ground.vertices[ground.faces[i + 1]]);
	gte_ldv2(&ground.vertices[ground.faces[i + 2]]);

	// Perform rotation, translation and projection
	gte_rtpt();

	// Perform normal clipping and store result in nclip
	gte_nclip();
	gte_stotz(&nclip);

	if (nclip >= 0) {
		// Store transformed vertices into poly position
		gte_stsxy3(&polyf3->x0, &polyf3->x1, &polyf3->x2);
		
		// Calculate otz index and store in otz
		gte_avsz3();
		gte_stotz(&otz);

		// Sort the triangle in the OT if within limit
    	if ((otz > 0) && (otz < OT_LENGTH)) {
      		addPrim(ot[currbuff][otz], polyf3);
      		nextprim += sizeof(POLY_F3);
		}
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Render function that invokes the draw of the current frame
///////////////////////////////////////////////////////////////////////////////
void Render(void) {
  DisplayFrame();
}

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