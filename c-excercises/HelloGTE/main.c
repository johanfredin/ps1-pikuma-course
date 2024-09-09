#include <sys/types.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>

#define VIDEO_MODE 0 // NTSC = 0, PAL = 1
#define SCREEN_RES_X 320
#define SCREEN_RES_Y 240
#define SCREEN_CENTER_X (SCREEN_RES_X >> 1)
#define SCREEN_CENTER_Y (SCREEN_RES_Y >> 1)
#define SCREEN_Z 320

#define OT_LENGTH 256

#define NUM_VERTICES 8
#define NUM_FACES 12

///////////////////////////////////////////////////////////////////////////////
// Cube vertices and face indices
///////////////////////////////////////////////////////////////////////////////
SVECTOR vertices[] = {
  { -128, -128, -128 },
  {  128, -128, -128 },
  {  128, -128,  128 },
  { -128, -128,  128 },
  { -128,  128, -128 },
  {  128,  128, -128 },
  {  128,  128,  128 },
  { -128,  128,  128 }
};

short faces[] = {
  0, 3, 2,      // top
  0, 2, 1,      // top
  4, 0, 1,      // front
  4, 1, 5,    // front
  7, 4, 5,   // bottom
  7, 5, 6,   // bottom
  5, 1, 2,   // right
  5, 2, 6,   // right
  2, 3, 7,   // back
  2, 7, 6,   // back
  0, 4, 7,   // left
  0, 7, 3    // left
};

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

POLY_G3 *poly;

SVECTOR rotation = {0, 0, 0};
VECTOR translation = {0, 0, 900};
VECTOR scale = {ONE, ONE, ONE};

MATRIX world = {0};

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

  RotMatrix(&rotation, &world);        // Populate the world matrix with the current rotation values
  TransMatrix(&world, &translation);   // Populate the world matrix with the current translation values
  ScaleMatrix(&world, &scale);         // Populate the world matrix with the current scale values

  SetRotMatrix(&world);                // Sets the rotation matrix to be used by the GTE (RotTransPers)
  SetTransMatrix(&world);              // Sets the translation matrix to be used by the GTE (RotTransPers)

  // Loop all triangle faces
  for (i = 0; i < NUM_FACES * 3; i += 3) {
    poly = (POLY_G3*) nextprim;
    setPolyG3(poly);
    setRGB0(poly, 255, 0, 255);
    setRGB1(poly, 255, 255, 0);
    setRGB2(poly, 0, 255, 255);

    nclip = RotAverageNclip3(
        &vertices[faces[i + 0]], 
        &vertices[faces[i + 1]], 
        &vertices[faces[i + 2]], 
        (long*)&poly->x0, 
        (long*)&poly->x1, 
        (long*)&poly->x2, 
        &p, 
        &otz, 
        &flg
    );
    
    if (nclip <= 0) {
      continue;
    }

    if ((otz > 0) && (otz < OT_LENGTH)) {
      addPrim(ot[currbuff][otz], poly);
      nextprim += sizeof(POLY_G3);
    }
  }
  rotation.vx += 6;
  rotation.vy += 8;
  rotation.vz += 12;
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