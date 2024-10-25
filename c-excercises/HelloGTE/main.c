#include <libetc.h>
#include <libgpu.h>
#include <libgte.h>
#include <sys/types.h>

#include "inline_n.h"

#define VIDEO_MODE 0
#define SCREEN_RES_X 320
#define SCREEN_RES_Y 240
#define SCREEN_CENTER_X (SCREEN_RES_X >> 1)
#define SCREEN_CENTER_Y (SCREEN_RES_Y >> 1)
#define SCREEN_Z 320

#define OT_LENGTH 2048

#define NUM_FACES 6
#define NUM_VERTICES 8

typedef struct {
	DRAWENV draw[2];
	DISPENV disp[2];
} DoubleBuff;


///////////////////////////////////////////////////////////////////////////////
// Declarations and global variables
///////////////////////////////////////////////////////////////////////////////

DoubleBuff screen;		  // Struct to hold the display & draw buffers
u_short currbuff;		  // Holds the current buffer number (0 or 1)
u_long ot[2][OT_LENGTH];  // Ordering table holding pointers to sorted primitives
char primbuff[2][2048];	  // Primitive buffer that holds the actual data for each
char *nextprim;			  // Pointer to the next primitive in the primitive buffer

POLY_G4 *polyg4;

SVECTOR vertices[] = {
	{ -128, -128, -128 }, // Front top-left
	{128, -128, -128}, // Front top-right
	{128, -128, 128}, // Back top-right
	{-128, -128, 128}, // Back top-left
	{-128, 128, -128}, // Front bottom-left
	{128, 128, -128}, // Front bottom-right
	{128, 128, 128}, // Back bottom-right
	{-128, 128, 128} // Back bottom-left
};

// Faces that are looking at us are in clockwise (->) order, faces looking away in counter-clockwise (<-) order
short faces[] = {
	// TOP
	3, 2, 0, 1,   // btl -> btr -> ftl -> ftr
	// FRONT
	0, 1, 4, 5,	 // ftl -> ftr -> fbl -> fbr
	// BOTTOM
	7, 4, 6, 5,  // bbl <- fbl <- bbr <- fbr
	// RIGHT
	5, 1, 6, 2,  // fbr -> ftr -> bbr -> btr
	// BACK
	2, 3, 6, 7,	 // btr <- btl <- bbr <- bbl
	// LEFT
	0, 4, 3, 7 // ftl <- fbl <- btl <- bbl
};


SVECTOR rotation = {0, 0, 0};
VECTOR translation = {0, 0, 900};
VECTOR scale = {ONE, ONE, ONE};

MATRIX world = {0};

VECTOR vel = {0, 0, 0};
VECTOR acc = {0, 0, 0};
VECTOR pos = {0, 0, 0};

/**
 * Initialize the display mode and setup double buffering
*/
void ScreenInit(void) {
	// Reset GPU
	ResetGraph(0);

	// Set the display area of the first buffer
	SetDefDispEnv(&screen.disp[0], 0, 0, SCREEN_RES_X, SCREEN_RES_Y);
	SetDefDrawEnv(&screen.draw[0], 0, 240, SCREEN_RES_X, SCREEN_RES_Y);

	// Set the display area of the second buffer
	SetDefDispEnv(&screen.disp[1], 0, 240, SCREEN_RES_X, SCREEN_RES_Y);
	SetDefDrawEnv(&screen.draw[1], 0, 0, SCREEN_RES_X, SCREEN_RES_Y);

	// Set the back/drawing buffer
	screen.draw[0].isbg = 1;
	screen.draw[1].isbg = 1;

	// Set the background clear color
	setRGB0(&screen.draw[0], 63, 0, 127);  // dark purple
	setRGB0(&screen.draw[1], 63, 0, 127);  // dark purple

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

	// Reset next primitive pointer to the start of the primitive buffer
	nextprim = primbuff[currbuff];

	acc.vx = 0;
	acc.vy = 1;
	acc.vz = 0;

	vel.vx = 0;
	vel.vy = 0;
	vel.vz = 0;

	pos.vx = 0;
	pos.vy = -400;
	pos.vz = 1800;
}

///////////////////////////////////////////////////////////////////////////////
// Setup function that is called once at the beginning of the execution
///////////////////////////////////////////////////////////////////////////////
void Update(void) {
	long p, flg;

	rotation.vx += 16;
	rotation.vy += 8;

	// Empty the Ordering Table
	ClearOTagR(ot[currbuff], OT_LENGTH);
	
	// Update the position based on acceleration and velocity
	vel.vx += acc.vx;
	vel.vy += acc.vy;
	vel.vz += acc.vz;

	pos.vx += vel.vx >> 1;
	pos.vy += vel.vy >> 1;
	pos.vz += vel.vz >> 1;

	if (pos.vy > 400) {
		vel.vy *= -1;
	}
 
	RotMatrix(&rotation, &world);
	TransMatrix(&world, &pos);
	ScaleMatrix(&world, &scale);

	SetRotMatrix(&world);
	SetTransMatrix(&world);

	for (int i = 0; i < NUM_FACES * 4; i += 4) {
		polyg4 = (POLY_G4*) nextprim;
		setPolyG4(polyg4);
		setRGB0(polyg4, 255, 0, 255);
		setRGB1(polyg4, 255, 255, 0);
		setRGB2(polyg4, 0, 255, 255);
		setRGB3(polyg4, 0, 255, 0);
		
		int f1 = faces[i + 0];
		int f2 = faces[i + 1];
		int f3 = faces[i + 2];
		int f4 = faces[i + 3];	
		// printf("%d, %d, %d, %d\n", f1, f2, f3, f4);

		long otz = 0;
		long nclip = RotAverageNclip4(
			&vertices[f1], 
			&vertices[f2], 
			&vertices[f3],
			&vertices[f4], 
			(long*) &polyg4->x0,
			(long*) &polyg4->x1,
			(long*) &polyg4->x2,
			(long*) &polyg4->x3,
			&p, &otz, &flg
		);
		
		if(nclip <= 0) {
			// Dont render faces that are looking away
			continue;
		}

		if (otz >0 && otz < OT_LENGTH) {
			addPrim(ot[currbuff][otz], polyg4);
			nextprim += sizeof(POLY_G4);
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