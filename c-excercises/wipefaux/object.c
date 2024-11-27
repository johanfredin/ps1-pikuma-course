#include "object.h"

#include <libgpu.h>
#include <stdio.h>
#include <stdlib.h>

#include "display.h"
#include "globals.h"
#include "inline_n.h"
#include "malloc.h"
#include "texture.h"
#include "utils.h"

#define LOG_OBJ 0

#define NAME_LEN 16

static inline void PadSkip(u_long *i, u_short skip) { *i += skip; }

static inline void LogObject(Object *o) {
	printf("------------------------\n");
	printf("Name: %s\n", o->name);
	printf("Normals: %d\n", o->numnormals);
	printf("Vertices: %d\n", o->numvertices);
	printf("Primitives: %d\n", o->numprimitives);
	printf("Flags: %d\n", o->flags);
	printf("Origin {x:%d, y:%d, z:%d}\n", (u_short)o->origin.vx >> 12, (u_short)o->origin.vy >> 12,
		   (u_short)o->origin.vz >> 12);
}

static void LoadPeripherals(Object *object, u_char *bytes, u_long *b) {
	object->numvertices = GetShortBE(bytes, b);
	PadSkip(b, 6);	// unused padding

	object->numnormals = GetShortBE(bytes, b);
	PadSkip(b, 6);	// unused padding

	object->numprimitives = GetShortBE(bytes, b);
	PadSkip(b, 22);	 // unused padding

	object->flags = GetShortBE(bytes, b);
	PadSkip(b, 26);	 // unused padding

	object->origin.vx = GetLongBE(bytes, b);
	object->origin.vy = GetLongBE(bytes, b);
	object->origin.vz = GetLongBE(bytes, b);

	// Skip unused bytes ("skeleton" information, rotation matrices, and extra flags)
	PadSkip(b, 48);

	object->vertices = (SVECTOR *)malloc3(object->numvertices * sizeof(SVECTOR));
	for (int i = 0; i < object->numvertices; i++) {
		object->vertices[i].vx = (GetShortBE(bytes, b));
		object->vertices[i].vy = (GetShortBE(bytes, b));
		object->vertices[i].vz = (GetShortBE(bytes, b));
		PadSkip(b, 2);	// padding
	}

	object->normals = (SVECTOR *)malloc3(object->numnormals * sizeof(SVECTOR));
	for (int i = 0; i < object->numnormals; i++) {
		object->normals[i].vx = GetShortBE(bytes, b);
		object->normals[i].vy = GetShortBE(bytes, b);
		object->normals[i].vz = GetShortBE(bytes, b);
		PadSkip(b, 2);	// padding
	}
}

static void LoadPrimitives(Object *object, u_char *bytes, u_long *b, u_short texturestart) {
	Texture *texture = NULL;
	u_short uoffset, voffset;

	object->primitives = (PrimitiveNode *)malloc3(object->numprimitives * sizeof(PrimitiveNode));
	for (int i = 0; i < object->numprimitives; i++) {
		object->primitives[i].type = GetShortBE(bytes, b);
		object->primitives[i].flag = GetShortBE(bytes, b);
		switch (object->primitives[i].type) {
			case TYPE_F3: {
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(F3));
				F3 *prm = (F3 *)object->primitives[i].primitive;
				prm->type = TYPE_F3;
				prm->coords[0] = GetShortBE(bytes, b);
				prm->coords[1] = GetShortBE(bytes, b);
				prm->coords[2] = GetShortBE(bytes, b);
				prm->pad1 = GetShortBE(bytes, b);
				prm->color = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
									   GetChar(bytes, b)};
				break;
			}
			case TYPE_FT3: {
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(FT3));
				FT3 *prm = (FT3 *)object->primitives[i].primitive;
				prm->type = TYPE_FT3;
				prm->coords[0] = GetShortBE(bytes, b);
				prm->coords[1] = GetShortBE(bytes, b);
				prm->coords[2] = GetShortBE(bytes, b);
				prm->texture = GetShortBE(bytes, b);
				prm->clut = GetShortBE(bytes, b);
				prm->tpage = GetShortBE(bytes, b);
				prm->u0 = GetChar(bytes, b);
				prm->v0 = GetChar(bytes, b);
				prm->u1 = GetChar(bytes, b);
				prm->v1 = GetChar(bytes, b);
				prm->u2 = GetChar(bytes, b);
				prm->v2 = GetChar(bytes, b);
				prm->pad1 = GetShortBE(bytes, b);
				prm->color = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
									   GetChar(bytes, b)};

				prm->texture += texturestart;  // offset by the start texture of this prm
				texture = GetFromTextureStore(prm->texture);
				prm->tpage = texture->tpage;
				prm->clut = texture->clut;
				uoffset = texture->u0;
				voffset = texture->v0;
				prm->u0 += uoffset;
				prm->v0 += voffset;
				prm->u1 += uoffset;
				prm->v1 += voffset;
				prm->u2 += uoffset;
				prm->v2 += voffset;
				break;
			}
			case TYPE_F4: {
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(F4));
				F4 *prm = (F4 *)object->primitives[i].primitive;
				prm->type = TYPE_F4;
				prm->coords[0] = GetShortBE(bytes, b);
				prm->coords[1] = GetShortBE(bytes, b);
				prm->coords[2] = GetShortBE(bytes, b);
				prm->coords[3] = GetShortBE(bytes, b);
				prm->color = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
									   GetChar(bytes, b)};
				break;
			}
			case TYPE_FT4: {
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(FT4));
				FT4 *prm = (FT4 *)object->primitives[i].primitive;
				prm->type = TYPE_FT4;
				prm->coords[0] = GetShortBE(bytes, b);
				prm->coords[1] = GetShortBE(bytes, b);
				prm->coords[2] = GetShortBE(bytes, b);
				prm->coords[3] = GetShortBE(bytes, b);
				prm->texture = GetShortBE(bytes, b);
				prm->clut = GetShortBE(bytes, b);
				prm->tpage = GetShortBE(bytes, b);
				prm->u0 = GetChar(bytes, b);
				prm->v0 = GetChar(bytes, b);
				prm->u1 = GetChar(bytes, b);
				prm->v1 = GetChar(bytes, b);
				prm->u2 = GetChar(bytes, b);
				prm->v2 = GetChar(bytes, b);
				prm->u3 = GetChar(bytes, b);
				prm->v3 = GetChar(bytes, b);
				prm->pad1 = GetShortBE(bytes, b);
				prm->color = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
									   GetChar(bytes, b)};

				prm->texture += texturestart;  // offset by the start texture of this prm
				texture = GetFromTextureStore(prm->texture);
				prm->tpage = texture->tpage;
				prm->clut = texture->clut;
				uoffset = texture->u0;
				voffset = texture->v0;
				prm->u0 += uoffset;
				prm->v0 += voffset;
				prm->u1 += uoffset;
				prm->v1 += voffset;
				prm->u2 += uoffset;
				prm->v2 += voffset;
				prm->u3 += uoffset;
				prm->v3 += voffset;
				break;
			}
			case TYPE_G3: {
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(G3));
				G3 *prm = (G3 *)object->primitives[i].primitive;
				prm->type = TYPE_G3;
				prm->coords[0] = GetShortBE(bytes, b);
				prm->coords[1] = GetShortBE(bytes, b);
				prm->coords[2] = GetShortBE(bytes, b);
				prm->pad1 = GetShortBE(bytes, b);
				prm->color[0] = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
										  GetChar(bytes, b)};
				prm->color[1] = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
										  GetChar(bytes, b)};
				prm->color[2] = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
										  GetChar(bytes, b)};
				break;
			}
			case TYPE_GT3: {
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(GT3));
				GT3 *prm = (GT3 *)object->primitives[i].primitive;
				prm->type = TYPE_GT3;
				prm->coords[0] = GetShortBE(bytes, b);
				prm->coords[1] = GetShortBE(bytes, b);
				prm->coords[2] = GetShortBE(bytes, b);
				prm->texture = GetShortBE(bytes, b);
				prm->clut = GetShortBE(bytes, b);
				prm->tpage = GetShortBE(bytes, b);
				prm->u0 = GetChar(bytes, b);
				prm->v0 = GetChar(bytes, b);
				prm->u1 = GetChar(bytes, b);
				prm->v1 = GetChar(bytes, b);
				prm->u2 = GetChar(bytes, b);
				prm->v2 = GetChar(bytes, b);
				prm->pad1 = GetShortBE(bytes, b);
				prm->color[0] = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
										  GetChar(bytes, b)};
				prm->color[1] = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
										  GetChar(bytes, b)};
				prm->color[2] = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
										  GetChar(bytes, b)};

				prm->texture += texturestart;  // offset by the start texture of this prm
				texture = GetFromTextureStore(prm->texture);
				prm->tpage = texture->tpage;
				prm->clut = texture->clut;
				uoffset = texture->u0;
				voffset = texture->v0;
				prm->u0 += uoffset;
				prm->v0 += voffset;
				prm->u1 += uoffset;
				prm->v1 += voffset;
				prm->u2 += uoffset;
				prm->v2 += voffset;
				break;
			}
			case TYPE_G4: {
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(G4));
				G4 *prm = (G4 *)object->primitives[i].primitive;
				prm->type = TYPE_G4;
				prm->coords[0] = GetShortBE(bytes, b);
				prm->coords[1] = GetShortBE(bytes, b);
				prm->coords[2] = GetShortBE(bytes, b);
				prm->coords[3] = GetShortBE(bytes, b);
				prm->color[0] = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
										  GetChar(bytes, b)};
				prm->color[1] = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
										  GetChar(bytes, b)};
				prm->color[2] = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
										  GetChar(bytes, b)};
				prm->color[3] = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
										  GetChar(bytes, b)};
				break;
			}
			case TYPE_GT4: {
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(GT4));
				GT4 *prm = (GT4 *)object->primitives[i].primitive;
				prm->type = TYPE_GT4;
				prm->coords[0] = GetShortBE(bytes, b);
				prm->coords[1] = GetShortBE(bytes, b);
				prm->coords[2] = GetShortBE(bytes, b);
				prm->coords[3] = GetShortBE(bytes, b);
				prm->texture = GetShortBE(bytes, b);
				prm->clut = GetShortBE(bytes, b);
				prm->tpage = GetShortBE(bytes, b);
				prm->u0 = GetChar(bytes, b);
				prm->v0 = GetChar(bytes, b);
				prm->u1 = GetChar(bytes, b);
				prm->v1 = GetChar(bytes, b);
				prm->u2 = GetChar(bytes, b);
				prm->v2 = GetChar(bytes, b);
				prm->u3 = GetChar(bytes, b);
				prm->v3 = GetChar(bytes, b);
				prm->pad1 = GetShortBE(bytes, b);
				prm->color[0] = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
										  GetChar(bytes, b)};
				prm->color[1] = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
										  GetChar(bytes, b)};
				prm->color[2] = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
										  GetChar(bytes, b)};
				prm->color[3] = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
										  GetChar(bytes, b)};

				prm->texture += texturestart;  // offset by the start texture of this prm
				texture = GetFromTextureStore(prm->texture);
				prm->tpage = texture->tpage;
				prm->clut = texture->clut;
				uoffset = texture->u0;
				voffset = texture->v0;
				prm->u0 += uoffset;
				prm->v0 += voffset;
				prm->u1 += uoffset;
				prm->v1 += voffset;
				prm->u2 += uoffset;
				prm->v2 += voffset;
				prm->u3 += uoffset;
				prm->v3 += voffset;
				break;
			}
			case TYPE_TSPR:
			case TYPE_BSPR: {
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(SPR));
				SPR *prm = (SPR *)object->primitives[i].primitive;
				prm->type = TYPE_TSPR;
				prm->coord = GetShortBE(bytes, b);
				prm->width = GetShortBE(bytes, b);
				prm->height = GetShortBE(bytes, b);
				prm->texture = GetShortBE(bytes, b);
				prm->color = (CVECTOR){GetChar(bytes, b), GetChar(bytes, b), GetChar(bytes, b),
									   GetChar(bytes, b)};
				break;
			}
			case TYPE_SPLINE: {
				PadSkip(b, 52);	 // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TYPE_POINTLIGHT: {
				PadSkip(b, 24);	 // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TYPE_SPOTLIGHT: {
				PadSkip(b, 36);	 // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TYPE_INFINITELIGHT: {
				PadSkip(b, 12);	 // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TYPE_LSF3: {
				PadSkip(b, 12);	 // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TYPE_LSFT3: {
				PadSkip(b, 24);	 // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TYPE_LSF4: {
				PadSkip(b, 16);	 // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TYPE_LSFT4: {
				PadSkip(b, 28);	 // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TYPE_LSG3: {
				PadSkip(b, 24);	 // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TYPE_LSGT3: {
				PadSkip(b, 36);	 // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TYPE_LSG4: {
				PadSkip(b, 32);	 // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TYPE_LSGT4: {
				PadSkip(b, 42);	 // --> skip this amount of bytes to bypass this primitive type
				break;
			}
		}
	}
}

static void LoadObjectPRM(Object *object, u_short texturestart, u_char *bytes, u_long *b) {
	for (int i = 0; i < NAME_LEN; i++) {
		object->name[i] = GetChar(bytes, b);
	}

	object->numvertices = GetShortBE(bytes, b);
	PadSkip(b, 6);	 // unused padding

	object->numnormals = GetShortBE(bytes, b);
	PadSkip(b, 6);	 // unused padding

	object->numprimitives = GetShortBE(bytes, b);
	PadSkip(b, 22);  // unused padding

	object->flags = GetShortBE(bytes, b);
	PadSkip(b, 26);  // unused padding

	object->origin.vx = GetLongBE(bytes, b);
	object->origin.vy = GetLongBE(bytes, b);
	object->origin.vz = GetLongBE(bytes, b);

	// Skip unused bytes ("skeleton" information, rotation matrices, and extra flags)
	PadSkip(b, 48);

	object->vertices = (SVECTOR *)malloc3(object->numvertices * sizeof(SVECTOR));
	for (int i = 0; i < object->numvertices; i++) {
		object->vertices[i].vx = (GetShortBE(bytes, b));
		object->vertices[i].vy = (GetShortBE(bytes, b));
		object->vertices[i].vz = (GetShortBE(bytes, b));
		PadSkip(b, 2);	 // padding
	}

	object->normals = (SVECTOR *)malloc3(object->numnormals * sizeof(SVECTOR));
	for (int i = 0; i < object->numnormals; i++) {
		object->normals[i].vx = GetShortBE(bytes, b);
		object->normals[i].vy = GetShortBE(bytes, b);
		object->normals[i].vz = GetShortBE(bytes, b);
		PadSkip(b, 2);	 // padding
	}

	// Load all the primitives
	LoadPrimitives(object, bytes, b, texturestart);

	// Populate object's initial transform values
	object->position = (VECTOR){object->origin.vx, object->origin.vy, object->origin.vz};
	object->scale = (VECTOR){ONE, ONE, ONE};
	object->rotation = (SVECTOR){0, 0, 0};
}

Object *LoadObjectPRMs(char *filename, u_short texturestart) {
	u_long length;
	u_char *bytes = (u_char *)FileRead(filename, &length);
	u_long b = 0;

	Object *root = (Object *)malloc3(sizeof(Object));
	Object *curr = root;
	while (b < length) {
		LoadObjectPRM(curr, texturestart, bytes, &b);
		printf("Bytes read: %lu out of %lu\n", b, length);
		if (b >= length) {
			printf("End of the line...\n");
			curr->next = NULL;
			break;
		}
		curr->next = (Object *)malloc3(sizeof(Object));
		curr = curr->next;
	}

	// Free the bytes
	free3(bytes);
	
	#if LOG_OBJ == 1
	for(Object *o = root; o != NULL; o = o->next) {
		LogObject(o);
	}
	#endif

	return root;
}

void RenderObject(Object *object, Camera *camera) {
	short nclip;
	long otz, p, flg;

	MATRIX worldmat;
	MATRIX viewmat;

	RotMatrix(&object->rotation, &worldmat);
	TransMatrix(&worldmat, &object->position);
	ScaleMatrix(&worldmat, &object->scale);

	CompMatrixLV(&camera->lookat, &worldmat, &viewmat);	 // combine word and lookat transform

	SetRotMatrix(&viewmat);
	SetTransMatrix(&viewmat);

	// Loop all object primitives
	for (int i = 0; i < object->numprimitives; i++) {
		switch (object->primitives[i].type) {
			case TYPE_F3: {
				F3 *prm = (F3 *)object->primitives[i].primitive;
				POLY_F3 *poly = (POLY_F3 *)GetNextPrim();
				gte_ldv0(&object->vertices[prm->coords[0]]);
				gte_ldv1(&object->vertices[prm->coords[1]]);
				gte_ldv2(&object->vertices[prm->coords[2]]);
				gte_rtpt();
				gte_nclip();
				gte_stopz(&nclip);
				if (nclip < 0) {
					continue;
				}
				gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);
				gte_avsz3();
				gte_stotz(&otz);
				if (otz > 0 && otz < OT_LEN) {
					SetPolyF3(poly);
					setRGB0(poly, prm->color.r, prm->color.g, prm->color.b);
					addPrim(GetOTAt(GetCurrBuff(), otz), poly);
					IncrementNextPrim(sizeof(POLY_F3));
				}
				break;
			}
			case TYPE_G3: {
				G3 *prm = (G3 *)object->primitives[i].primitive;
				POLY_G3 *poly = (POLY_G3 *)GetNextPrim();
				gte_ldv0(&object->vertices[prm->coords[0]]);
				gte_ldv1(&object->vertices[prm->coords[1]]);
				gte_ldv2(&object->vertices[prm->coords[2]]);
				gte_rtpt();
				gte_nclip();
				gte_stopz(&nclip);
				if (nclip < 0) {
					continue;
				}
				gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);
				gte_avsz3();
				gte_stotz(&otz);
				if (otz > 0 && otz < OT_LEN) {
					SetPolyG3(poly);
					setRGB0(poly, prm->color[0].r, prm->color[0].g, prm->color[0].b);
					setRGB1(poly, prm->color[1].r, prm->color[1].g, prm->color[1].b);
					setRGB2(poly, prm->color[2].r, prm->color[2].g, prm->color[2].b);
					addPrim(GetOTAt(GetCurrBuff(), otz), poly);
					IncrementNextPrim(sizeof(POLY_G3));
				}
				break;
			}
			case TYPE_FT3: {
				FT3 *prm = (FT3 *)object->primitives[i].primitive;
				POLY_FT3 *poly = (POLY_FT3 *)GetNextPrim();
				gte_ldv0(&object->vertices[prm->coords[0]]);
				gte_ldv1(&object->vertices[prm->coords[1]]);
				gte_ldv2(&object->vertices[prm->coords[2]]);
				gte_rtpt();
				gte_nclip();
				gte_stopz(&nclip);
				if (nclip < 0) {
					continue;
				}
				gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);
				gte_avsz3();
				gte_stotz(&otz);
				if (otz > 0 && otz < OT_LEN) {
					SetPolyFT3(poly);
					setRGB0(poly, prm->color.r, prm->color.g, prm->color.b);
					poly->tpage = prm->tpage;
					poly->clut = prm->clut;
					setUV3(poly, prm->u0, prm->v0, prm->u1, prm->v1, prm->u2, prm->v2);
					addPrim(GetOTAt(GetCurrBuff(), otz), poly);
					IncrementNextPrim(sizeof(POLY_FT3));
				}
				break;
			}
			case TYPE_GT3: {
				GT3 *prm = (GT3 *)object->primitives[i].primitive;
				POLY_GT3 *poly = (POLY_GT3 *)GetNextPrim();
				gte_ldv0(&object->vertices[prm->coords[0]]);
				gte_ldv1(&object->vertices[prm->coords[1]]);
				gte_ldv2(&object->vertices[prm->coords[2]]);
				gte_rtpt();
				gte_nclip();
				gte_stopz(&nclip);
				if (nclip < 0) {
					continue;
				}
				gte_stsxy3(&poly->x0, &poly->x1, &poly->x2);
				gte_avsz3();
				gte_stotz(&otz);
				if (otz > 0 && otz < OT_LEN) {
					SetPolyGT3(poly);
					setRGB0(poly, prm->color[0].r, prm->color[0].g, prm->color[0].b);
					setRGB1(poly, prm->color[1].r, prm->color[1].g, prm->color[1].b);
					setRGB2(poly, prm->color[2].r, prm->color[2].g, prm->color[2].b);
					poly->tpage = prm->tpage;
					poly->clut = prm->clut;
					setUV3(poly, prm->u0, prm->v0, prm->u1, prm->v1, prm->u2, prm->v2);
					addPrim(GetOTAt(GetCurrBuff(), otz), poly);
					IncrementNextPrim(sizeof(POLY_GT3));
				}
				break;
			}
			case TYPE_F4: {
				F4 *prm = (F4 *)object->primitives[i].primitive;
				POLY_F4 *poly = (POLY_F4 *)GetNextPrim();
				gte_ldv0(&object->vertices[prm->coords[0]]);
				gte_ldv1(&object->vertices[prm->coords[1]]);
				gte_ldv2(&object->vertices[prm->coords[2]]);
				gte_rtpt();
				gte_nclip();
				gte_stopz(&nclip);
				if (nclip < 0) {
					continue;
				}
				gte_stsxy0(&poly->x0);
				gte_ldv0(&object->vertices[prm->coords[3]]);
				gte_rtps();
				gte_stsxy3(&poly->x1, &poly->x2, &poly->x3);
				gte_avsz4();
				gte_stotz(&otz);
				if (otz > 0 && otz < OT_LEN) {
					SetPolyF4(poly);
					poly->r0 = prm->color.r;
					poly->g0 = prm->color.g;
					poly->b0 = prm->color.b;
					addPrim(GetOTAt(GetCurrBuff(), otz), poly);
					IncrementNextPrim(sizeof(POLY_F4));
				}
				break;
			}
			case TYPE_G4: {
				G4 *prm = (G4 *)object->primitives[i].primitive;
				POLY_G4 *poly = (POLY_G4 *)GetNextPrim();
				gte_ldv0(&object->vertices[prm->coords[0]]);
				gte_ldv1(&object->vertices[prm->coords[1]]);
				gte_ldv2(&object->vertices[prm->coords[2]]);
				gte_rtpt();
				gte_nclip();
				gte_stopz(&nclip);
				if (nclip < 0) {
					continue;
				}
				gte_stsxy0(&poly->x0);
				gte_ldv0(&object->vertices[prm->coords[3]]);
				gte_rtps();
				gte_stsxy3(&poly->x1, &poly->x2, &poly->x3);
				gte_avsz4();
				gte_stotz(&otz);
				if (otz > 0 && otz < OT_LEN) {
					SetPolyG4(poly);
					setRGB0(poly, prm->color[0].r, prm->color[0].g, prm->color[0].b);
					setRGB1(poly, prm->color[1].r, prm->color[1].g, prm->color[1].b);
					setRGB2(poly, prm->color[2].r, prm->color[2].g, prm->color[2].b);
					setRGB3(poly, prm->color[3].r, prm->color[3].g, prm->color[3].b);

					addPrim(GetOTAt(GetCurrBuff(), otz), poly);
					IncrementNextPrim(sizeof(POLY_G4));
				}
				break;
			}
			case TYPE_FT4: {
				FT4 *prm = (FT4 *)object->primitives[i].primitive;
				POLY_FT4 *poly = (POLY_FT4 *)GetNextPrim();
				gte_ldv0(&object->vertices[prm->coords[0]]);
				gte_ldv1(&object->vertices[prm->coords[1]]);
				gte_ldv2(&object->vertices[prm->coords[2]]);
				gte_rtpt();
				gte_nclip();
				gte_stopz(&nclip);
				if (nclip < 0) {
					continue;
				}
				gte_stsxy0(&poly->x0);
				gte_ldv0(&object->vertices[prm->coords[3]]);
				gte_rtps();
				gte_stsxy3(&poly->x1, &poly->x2, &poly->x3);
				gte_avsz4();
				gte_stotz(&otz);
				if (otz > 0 && otz < OT_LEN) {
					SetPolyFT4(poly);
					setRGB0(poly, prm->color.r, prm->color.g, prm->color.b);
					poly->tpage = prm->tpage;
					poly->clut = prm->clut;
					setUV4(poly, prm->u0, prm->v0, prm->u1, prm->v1, prm->u2, prm->v2, prm->u3,
						   prm->v3);

					addPrim(GetOTAt(GetCurrBuff(), otz), poly);
					IncrementNextPrim(sizeof(POLY_FT4));
				}
				break;
			}
			case TYPE_GT4: {
				GT4 *prm = (GT4 *)object->primitives[i].primitive;
				POLY_GT4 *poly = (POLY_GT4 *)GetNextPrim();
				gte_ldv0(&object->vertices[prm->coords[0]]);
				gte_ldv1(&object->vertices[prm->coords[1]]);
				gte_ldv2(&object->vertices[prm->coords[2]]);
				gte_rtpt();
				gte_nclip();
				gte_stopz(&nclip);
				if (nclip < 0) {
					continue;
				}
				gte_stsxy0(&poly->x0);
				gte_ldv0(&object->vertices[prm->coords[3]]);
				gte_rtps();
				gte_stsxy3(&poly->x1, &poly->x2, &poly->x3);
				gte_avsz4();
				gte_stotz(&otz);
				if (otz > 0 && otz < OT_LEN) {
					SetPolyGT4(poly);
					setRGB0(poly, prm->color[0].r, prm->color[0].g, prm->color[0].b);
					setRGB1(poly, prm->color[1].r, prm->color[1].g, prm->color[1].b);
					setRGB2(poly, prm->color[2].r, prm->color[2].g, prm->color[2].b);
					setRGB3(poly, prm->color[3].r, prm->color[3].g, prm->color[3].b);
					poly->tpage = prm->tpage;
					poly->clut = prm->clut;
					setUV4(poly, prm->u0, prm->v0, prm->u1, prm->v1, prm->u2, prm->v2, prm->u3,
						   prm->v3);

					addPrim(GetOTAt(GetCurrBuff(), otz), poly);
					IncrementNextPrim(sizeof(POLY_GT4));
				}
				break;
			}
		}
	}
}