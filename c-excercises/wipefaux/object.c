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

static inline void PadSkip(u_long *i, u_short skip) {
	*i += skip;
}

void LoadObjectPRM(Object *object, char *filename) {
	u_long length;
	u_char *bytes = (u_char *)FileRead(filename, &length);
	Texture *texture = NULL;
	u_short uoffset, voffset;

	if (bytes == NULL) {
		printf("Error reading %s from the CD.\n", filename);
		return;
	}

	u_long b = 0;

	for (int i = 0; i < 16; i++) {
		object->name[i] = GetChar(bytes, &b);
	}
	printf("Loading object: %s \n", object->name);
	

	object->numvertices = GetShortBE(bytes, &b);
	PadSkip(&b, 6);	 // unused padding
	printf("Num verts = %d \n", object->numvertices);

	object->numnormals = GetShortBE(bytes, &b);
	PadSkip(&b, 6);	 // unused padding
	printf("Num normals = %d \n", object->numnormals);

	object->numprimitives = GetShortBE(bytes, &b);
	// object->primitives = NULL;
	PadSkip(&b, 22);  // unused padding
	printf("Num primitives = %d \n", object->numprimitives);

	object->flags = GetShortBE(bytes, &b);
	PadSkip(&b, 26);  // unused padding

	object->origin.vx = GetLongBE(bytes, &b);
	object->origin.vy = GetLongBE(bytes, &b);
	object->origin.vz = GetLongBE(bytes, &b);
	printf("Origin (%ld, %ld, %ld) \n", object->origin.vx, object->origin.vy, object->origin.vz);

	// Skip unused bytes ("skeleton" information, rotation matrices, and extra flags)
	PadSkip(&b, 48);

	object->vertices = (SVECTOR *)malloc3(object->numvertices * sizeof(SVECTOR));
	for (int i = 0; i < object->numvertices; i++) {
		object->vertices[i].vx = (GetShortBE(bytes, &b));
		object->vertices[i].vy = (GetShortBE(bytes, &b));
		object->vertices[i].vz = (GetShortBE(bytes, &b));
		PadSkip(&b, 2);	 // padding
		printf("Vertex[%d] = (%d, %d, %d) \n", i, object->vertices[i].vx, object->vertices[i].vx, object->vertices[i].vx);
	}

	object->normals = (SVECTOR *)malloc3(object->numnormals * sizeof(SVECTOR));
	for (int i = 0; i < object->numnormals; i++) {
		object->normals[i].vx = GetShortBE(bytes, &b);
		object->normals[i].vy = GetShortBE(bytes, &b);
		object->normals[i].vz = GetShortBE(bytes, &b);
		PadSkip(&b, 2);	 // padding
		printf("Normal[%d] = (%d, %d, %d) \n", i, object->normals[i].vx, object->normals[i].vx, object->normals[i].vx);
	}

	object->primitives = (PrimitiveNode *)malloc3(object->numprimitives * sizeof(PrimitiveNode));
	for (int i = 0; i < object->numprimitives; i++) {
		object->primitives[i].type = GetShortBE(bytes, &b);
		object->primitives[i].flag = GetShortBE(bytes, &b);
		switch (object->primitives[i].type) {
			case TypeF3: {
				F3 *prm;
				printf("Loading primitive type F3 \n");
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(F3));
				prm = (F3 *)object->primitives[i].primitive;
				prm->type = TypeF3;
				prm->coords[0] = GetShortBE(bytes, &b);
				prm->coords[1] = GetShortBE(bytes, &b);
				prm->coords[2] = GetShortBE(bytes, &b);
				prm->pad1 = GetShortBE(bytes, &b);
				prm->color.r = GetChar(bytes, &b);
				prm->color.g = GetChar(bytes, &b);
				prm->color.b = GetChar(bytes, &b);
				prm->color.cd = GetChar(bytes, &b);
				break;
			}
			case TypeFT3: {
				printf("Loading primitive type FT3 \n");
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(FT3));
				FT3 *prm = (FT3 *)object->primitives[i].primitive;
				prm->type = TypeFT3;
				prm->coords[0] = GetShortBE(bytes, &b);
				prm->coords[1] = GetShortBE(bytes, &b);
				prm->coords[2] = GetShortBE(bytes, &b);
				prm->texture = GetShortBE(bytes, &b);
				prm->clut = GetShortBE(bytes, &b);
				prm->tpage = GetShortBE(bytes, &b);
				prm->u0 = GetChar(bytes, &b);
				prm->v0 = GetChar(bytes, &b);
				prm->u1 = GetChar(bytes, &b);
				prm->v1 = GetChar(bytes, &b);
				prm->u2 = GetChar(bytes, &b);
				prm->v2 = GetChar(bytes, &b);
				prm->pad1 = GetShortBE(bytes, &b);
				prm->color = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b), GetChar(bytes, &b), GetChar(bytes, &b)};

				texture = GetFromTextureStore(prm->tpage);
				uoffset = texture->u0;
				voffset = texture->v0;

				prm->u0 += uoffset;
				prm->v0 += voffset;
				prm->u1 += uoffset;
				prm->v1 += voffset;
				prm->u2 += uoffset;
				prm->v2 += voffset;

				prm->tpage = texture->tpage;
				prm->clut = texture->clut;

				break;
			}
			case TypeF4: {
				printf("Loading primitive type F4 \n");
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(F4));
				F4 *prm = (F4 *)object->primitives[i].primitive;
				prm->type = TypeF4;
				prm->coords[0] = GetShortBE(bytes, &b);
				prm->coords[1] = GetShortBE(bytes, &b);
				prm->coords[2] = GetShortBE(bytes, &b);
				prm->coords[3] = GetShortBE(bytes, &b);
				prm->color = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b), GetChar(bytes, &b), GetChar(bytes, &b)};
				break;
			}
			case TypeFT4: {
				printf("Loading primitive type FT4 \n");
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(FT4));
				FT4 *prm = (FT4 *)object->primitives[i].primitive;
				prm->type = TypeFT4;
				prm->coords[0] = GetShortBE(bytes, &b);
				prm->coords[1] = GetShortBE(bytes, &b);
				prm->coords[2] = GetShortBE(bytes, &b);
				prm->coords[3] = GetShortBE(bytes, &b);
				prm->texture = GetShortBE(bytes, &b);
				prm->clut = GetShortBE(bytes, &b);
				prm->tpage = GetShortBE(bytes, &b);
				prm->u0 = GetChar(bytes, &b);
				prm->v0 = GetChar(bytes, &b);
				prm->u1 = GetChar(bytes, &b);
				prm->v1 = GetChar(bytes, &b);
				prm->u2 = GetChar(bytes, &b);
				prm->v2 = GetChar(bytes, &b);
				prm->u3 = GetChar(bytes, &b);
				prm->v3 = GetChar(bytes, &b);
				prm->pad1 = GetShortBE(bytes, &b);
				prm->color = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b), GetChar(bytes, &b), GetChar(bytes, &b)};

				texture = GetFromTextureStore(prm->tpage);
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

				prm->tpage = texture->tpage;
				prm->clut = texture->clut;

				break;
			}
			case TypeG3: {
				printf("Loading primitive type G3 \n");
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(G3));
				G3 *prm = (G3 *)object->primitives[i].primitive;
				prm->type = TypeG3;
				prm->coords[0] = GetShortBE(bytes, &b);
				prm->coords[1] = GetShortBE(bytes, &b);
				prm->coords[2] = GetShortBE(bytes, &b);
				prm->pad1 = GetShortBE(bytes, &b);
				prm->color[0] = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b),
										  GetChar(bytes, &b), GetChar(bytes, &b)};
				prm->color[1] = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b),
										  GetChar(bytes, &b), GetChar(bytes, &b)};
				prm->color[2] = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b),
										  GetChar(bytes, &b), GetChar(bytes, &b)};
				break;
			}
			case TypeGT3: {
				printf("Loading primitive type GT3 \n");
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(GT3));
				GT3 *prm = (GT3 *)object->primitives[i].primitive;
				prm->type = TypeGT3;
				prm->coords[0] = GetShortBE(bytes, &b);
				prm->coords[1] = GetShortBE(bytes, &b);
				prm->coords[2] = GetShortBE(bytes, &b);
				prm->texture = GetShortBE(bytes, &b);
				prm->clut = GetShortBE(bytes, &b);
				prm->tpage = GetShortBE(bytes, &b);
				prm->u0 = GetChar(bytes, &b);
				prm->v0 = GetChar(bytes, &b);
				prm->u1 = GetChar(bytes, &b);
				prm->v1 = GetChar(bytes, &b);
				prm->u2 = GetChar(bytes, &b);
				prm->v2 = GetChar(bytes, &b);
				prm->pad1 = GetShortBE(bytes, &b);
				prm->color[0] = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b),
										  GetChar(bytes, &b), GetChar(bytes, &b)};
				prm->color[1] = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b),
										  GetChar(bytes, &b), GetChar(bytes, &b)};
				prm->color[2] = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b),
										  GetChar(bytes, &b), GetChar(bytes, &b)};


				texture = GetFromTextureStore(prm->tpage);
				uoffset = texture->u0;
				voffset = texture->v0;

				prm->u0 += uoffset;
				prm->v0 += voffset;
				prm->u1 += uoffset;
				prm->v1 += voffset;
				prm->u2 += uoffset;
				prm->v2 += voffset;
				
				prm->tpage = texture->tpage;
				prm->clut = texture->clut;
				break;
			}
			case TypeG4: {
				printf("Loading primitive type G4 \n");
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(G4));
				G4 *prm = (G4 *)object->primitives[i].primitive;
				prm->type = TypeG4;
				prm->coords[0] = GetShortBE(bytes, &b);
				prm->coords[1] = GetShortBE(bytes, &b);
				prm->coords[2] = GetShortBE(bytes, &b);
				prm->coords[3] = GetShortBE(bytes, &b);
				prm->color[0] = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b),
										  GetChar(bytes, &b), GetChar(bytes, &b)};
				prm->color[1] = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b),
										  GetChar(bytes, &b), GetChar(bytes, &b)};
				prm->color[2] = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b),
										  GetChar(bytes, &b), GetChar(bytes, &b)};
				prm->color[3] = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b),
										  GetChar(bytes, &b), GetChar(bytes, &b)};
				break;
			}
			case TypeGT4: {
				printf("Loading primitive type GT4 \n");
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(GT4));
				GT4 *prm = (GT4 *)object->primitives[i].primitive;
				prm->type = TypeGT4;
				prm->coords[0] = GetShortBE(bytes, &b);
				prm->coords[1] = GetShortBE(bytes, &b);
				prm->coords[2] = GetShortBE(bytes, &b);
				prm->coords[3] = GetShortBE(bytes, &b);
				prm->texture = GetShortBE(bytes, &b);
				prm->clut = GetShortBE(bytes, &b);
				prm->tpage = GetShortBE(bytes, &b);
				prm->u0 = GetChar(bytes, &b);
				prm->v0 = GetChar(bytes, &b);
				prm->u1 = GetChar(bytes, &b);
				prm->v1 = GetChar(bytes, &b);
				prm->u2 = GetChar(bytes, &b);
				prm->v2 = GetChar(bytes, &b);
				prm->u3 = GetChar(bytes, &b);
				prm->v3 = GetChar(bytes, &b);
				prm->pad1 = GetShortBE(bytes, &b);
				prm->color[0] = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b),
										  GetChar(bytes, &b), GetChar(bytes, &b)};
				prm->color[1] = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b),
										  GetChar(bytes, &b), GetChar(bytes, &b)};
				prm->color[2] = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b),
										  GetChar(bytes, &b), GetChar(bytes, &b)};
				prm->color[3] = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b),
										  GetChar(bytes, &b), GetChar(bytes, &b)};


				texture = GetFromTextureStore(prm->tpage);
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

				prm->tpage = texture->tpage;
				prm->clut = texture->clut;				
				break;	
			}
			case TypeTSPR:
			case TypeBSPR: {
				printf("Loading primitive type SPR \n");
				object->primitives[i].primitive = (Prm *)malloc3(sizeof(SPR));
				SPR *prm = (SPR *)object->primitives[i].primitive;
				prm->type = TypeTSPR;
				prm->coord = GetShortBE(bytes, &b);
				prm->width = GetShortBE(bytes, &b);
				prm->height = GetShortBE(bytes, &b);
				prm->texture = GetShortBE(bytes, &b);
				prm->color = (CVECTOR){GetChar(bytes, &b), GetChar(bytes, &b), GetChar(bytes, &b), GetChar(bytes, &b)};
				break;
			}
			case TypeSpline: {
				printf("Loading primitive type Spline \n");
				PadSkip(&b, 52);  // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TypePointLight: {
				printf("Loading primitive type PointLight\n");
				PadSkip(&b, 24);  // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TypeSpotLight: {
				printf("Loading primitive type SpotLight\n");
				PadSkip(&b, 36);  // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TypeInfiniteLight: {
				printf("Loading primitive type InfiniteLight\n");
				PadSkip(&b, 12);  // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TypeLSF3: {
				printf("Loading primitive type LSF3\n");
				PadSkip(&b, 12);  // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TypeLSFT3: {
				printf("Loading primitive type LSFT3\n");
				PadSkip(&b, 24);  // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TypeLSF4: {
				printf("Loading primitive type LSF4\n");
				PadSkip(&b, 16);  // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TypeLSFT4: {
				printf("Loading primitive type LSFT4\n");
				PadSkip(&b, 28);  // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TypeLSG3: {
				printf("Loading primitive type LSG3\n");
				PadSkip(&b, 24);  // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TypeLSGT3: {
				printf("Loading primitive type LSGT3\n");
				PadSkip(&b, 36);  // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TypeLSG4: {
				printf("Loading primitive type LSG4\n");
				PadSkip(&b, 32);  // --> skip this amount of bytes to bypass this primitive type
				break;
			}
			case TypeLSGT4: {
				printf("Loading primitive type LSGT4\n");
				PadSkip(&b, 42);  // --> skip this amount of bytes to bypass this primitive type
				break;
			}
		}
	}
	// Populate object's initial transform values
	object->position = (VECTOR){object->origin.vx, object->origin.vy, object->origin.vz};
	object->scale = (VECTOR){ONE, ONE, ONE};
	object->rotation = (SVECTOR){0, 0, 0};

	// Free the bytes
	free3(bytes);
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
			case TypeF3:
			case TypeFT3:
			case TypeG3:
			case TypeGT3: {
				POLY_F3 *poly;
				F3 *prm;
				prm = (F3 *)object->primitives[i].primitive;
				poly = (POLY_F3 *)GetNextPrim();
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
				if (otz > 0 && otz < OT_LENGTH) {
					SetPolyF3(poly);
					poly->r0 = 255;	 // prm->color.r;
					poly->g0 = 255;	 // prm->color.g;
					poly->b0 = 0;	 // prm->color.b;
					addPrim(GetOTAt(GetCurrentBuff(), otz), poly);
					IncrementNextPrim(sizeof(POLY_F3));
				}
				break;
			}
			case TypeF4:
			case TypeFT4:
			case TypeG4:
			case TypeGT4: {
				POLY_F4 *poly;
				F4 *prm;
				prm = (F4 *)object->primitives[i].primitive;
				poly = (POLY_F4 *)GetNextPrim();
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
				if (otz > 0 && otz < OT_LENGTH) {
					SetPolyF4(poly);
					poly->r0 = 255;	 // prm->color.r;
					poly->g0 = 0;	 // prm->color.g;
					poly->b0 = 255;	 // prm->color.b;
					addPrim(GetOTAt(GetCurrentBuff(), otz), poly);
					IncrementNextPrim(sizeof(POLY_F4));
				}
				break;
			}
		}
	}
}