#ifndef TEXTURE_H
#define TEXTURE_H

#include "stddef.h"
#define MAX_TEXTURES 800

#define CLUT4 0x00
#define CLUT8 0x01
#define TRUE15 0x10

#define CLUT_4BIT 0
#define CLUT_8BIT 1
#define CLUT_15BIT 2

#define TRANSLUCENT 1

#define CLUT(x, y) ((y << 6) | x)
#define TPAGE(c, a, x, y) ((c << 7) | (a << 5) | ((x >> 6) + ((y & 0x100) >> 4)))

#define TextureHOffset(x) (x & 0xFFC0)
#define TextureVOffset(y) (y & 0x100)

#define ClutType(t) ((t)->flags & 7)

typedef struct Texture {
	short type;
	short tpage;
	short clut;
	short clutX;
	short clutY;
	short clutW;
	short clutH;
	short textureX;
	short textureY;
	short textureW;
	short textureH;
	short u0;
	short v0;
	short u1;
	short v1;
	short u2;
	short v2;
	short u3;
	short v3;
} Texture;

typedef struct TimClut8 {
	long id;
	long flags;
	long clutBytes;
	short clutX;
	short clutY;
	short clutW;
	short clutH;
	short clut[256];
	long textureBytes;
	short textureX;
	short textureY;
	short textureW;
	short textureH;
} TimClut8;

typedef struct TimClut4 {
	long id;
	long flags;
	long clutBytes;
	short clutX;
	short clutY;
	short clutW;
	short clutH;
	short clut[16];
	long textureBytes;
	short textureX;
	short textureY;
	short textureW;
	short textureH;
} TimClut4;

typedef struct Tim {
	long id;
	long flags;
} Tim;

void LoadTexture(char *filename);

Texture *UploadTextureToVRAM(long timpointer);

Texture *GetFromTextureStore(u_short i);

u_short GetTextureCount();

#endif