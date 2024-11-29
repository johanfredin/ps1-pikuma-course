#include "track.h"

#include "display.h"
#include "globals.h"
#include "libgpu.h"
#include "libgte.h"
#include "malloc.h"
#include "texture.h"
#include "types.h"
#include "utils.h"
#include <inline_n.h>

#define MAX_SIGNED_16_BIT 32767
#define MIN_SIGNED_16_BIT -32767
#define MAX_DISTANCE 1350000
#define DRAW_GRID 0

static inline void PadSkip(u_short skip, u_long *i) { *i += skip; }

void LoadTrackVertices(Track *track, char *filename) {
	u_long length;
	u_char *bytes = (u_char *)FileRead(filename, &length);

	u_long b = 0;

	track->numvertices = length / BYTES_PER_VERTEX;
	track->vertices = calloc3(track->numvertices, sizeof(VECTOR));

	for (int i = 0; i < track->numvertices; i++) {
		track->vertices[i].vx = GetLongBE(bytes, &b);
		track->vertices[i].vy = GetLongBE(bytes, &b);
		track->vertices[i].vz = GetLongBE(bytes, &b);
		track->vertices[i].pad = GetLongBE(bytes, &b);	// load padding as well
	}

	free3(bytes);
}

void LoadTrackFaces(Track *track, char *filename, u_short texturestart) {
	u_long length;
	u_char *bytes = (u_char *)FileRead(filename, &length);

	u_long b = 0;

	track->numfaces = length / BYTES_PER_FACE;
	track->faces = calloc3(track->numfaces, sizeof(Face));
	Texture *texture;
	for (int i = 0; i < track->numfaces; i++) {
		Face *face = &track->faces[i];
		face->indices[0] = GetShortBE(bytes, &b);
		face->indices[1] = GetShortBE(bytes, &b);
		face->indices[2] = GetShortBE(bytes, &b);
		face->indices[3] = GetShortBE(bytes, &b);

		face->normal.vx = GetShortBE(bytes, &b);
		face->normal.vy = GetShortBE(bytes, &b);
		face->normal.vz = GetShortBE(bytes, &b);

		face->texture = GetChar(bytes, &b);

		face->flags = GetChar(bytes, &b);

		face->color.r  = GetChar(bytes, &b);
		face->color.g  = GetChar(bytes, &b);
		face->color.b  = GetChar(bytes, &b);
		face->color.cd = GetChar(bytes, &b);

		face->texture += texturestart;
		texture = GetFromTextureStore(face->texture);
		face->tpage = texture->tpage;
		face->clut  = texture->clut;

		if(face->flags & FACE_FLIP_TEXTURE) {
			// Flit the texture uv coords if flag set on face
			face->u0 = texture->u1;
			face->v0 = texture->v1;
			face->u1 = texture->u0;
			face->v1 = texture->v0;
			face->u2 = texture->u3;
			face->v2 = texture->v3;
			face->u3 = texture->u2;
			face->v3 = texture->v2;
		} else {
			face->u0 = texture->u0;
			face->v0 = texture->v0;
			face->u1 = texture->u1;
			face->v1 = texture->v1;
			face->u2 = texture->u2;
			face->v2 = texture->v2;
			face->u3 = texture->u3;
			face->v3 = texture->v3;
		}

		
	}


	free3(bytes);
}

void LoadTrackSections(Track *track, char *filename) {
	u_long length;
	u_char *bytes = (u_char *)FileRead(filename, &length);

	u_long b = 0;

	track->numsections = length / BYTES_PER_SECTION;
	track->sections = calloc3(track->numsections, sizeof(Section));

	for (int i = 0; i < track->numsections; i++) {
		PadSkip(4, &b);	 // Information about junction index (skipped for now)

		Section *section = &track->sections[i];
		section->prev = track->sections + GetLongBE(bytes, &b);
		section->next = track->sections + GetLongBE(bytes, &b);

		section->center.vx = GetLongBE(bytes, &b);
		section->center.vy = GetLongBE(bytes, &b);
		section->center.vz = GetLongBE(bytes, &b);

		PadSkip(118, &b);

		section->facestart = GetShortBE(bytes, &b);
		section->numfaces = GetShortBE(bytes, &b);

		PadSkip(4, &b);

		section->flags = GetShortBE(bytes, &b);
		section->id = GetShortBE(bytes, &b); // we dont care about this id so we simply skip and
		// write over
		section->id = i;

		PadSkip(2, &b);
	}

	free3(bytes);
}

static inline short Clamp16Bit(long value) {
    if (value > MAX_SIGNED_16_BIT) {
        return MAX_SIGNED_16_BIT;
    } else if (value < MIN_SIGNED_16_BIT) {
        return MIN_SIGNED_16_BIT;
    }
    return (short) value;
}

#if DRAW_GRID
static inline void DrawGrid(POLY_FT4 *poly) {
	for (int i = 0; i < 4; i++) {
		LINE_F2 *line = (LINE_F2*) GetNextPrim();
		setLineF2(line);
		setRGB0(line, 255, 255, 0);
		switch(i) {
			case 0: setXY2(line, poly->x0, poly->y0, poly->x1, poly->y1); break;
			case 1: setXY2(line, poly->x1, poly->y1, poly->x3, poly->y3); break;
			case 2: setXY2(line, poly->x3, poly->y3, poly->x2, poly->y2);  break;
			case 3: setXY2(line, poly->x2, poly->y2, poly->x0, poly->y0); break;
		}
		addPrim(GetOTAt(GetCurrBuff(), 0), line);
		IncrementNextPrim(sizeof(LINE_F2));
	}
}
#endif

static void RenderQuadRecursive(Face* face, SVECTOR *v0, SVECTOR *v1, SVECTOR *v2, SVECTOR *v3, u_char tu0, u_char tv0, u_char tu1, u_char tv1, u_char tu2, u_char tv2, u_char tu3, u_char tv3, u_char level, u_char depth) {
	if (level >= depth) {
		short nclip;

		gte_ldv0(v0);
		gte_ldv1(v1);
		gte_ldv2(v2);
		gte_rtpt();
		gte_nclip();
		gte_stopz(&nclip);
		if (nclip < 0) {
			return;
		}
		
		POLY_FT4 *poly = (POLY_FT4 *) GetNextPrim();
		long otz;
		gte_stsxy0(&poly->x0);
		gte_ldv0(v3);
		gte_rtps();
		gte_stsxy3(&poly->x1, &poly->x2, &poly->x3);
		gte_avsz4();
		gte_stotz(&otz);
		if (otz > 0 && otz < OT_LEN) {
			setPolyFT4(poly);
			setRGB0(poly, face->color.r, face->color.g, face->color.b);
			poly->tpage = face->tpage;
			poly->clut = face->clut;
	      	setUV4(poly, tu0, tv0, tu1, tv1, tu2, tv2, tu3, tv3);
			addPrim(GetOTAt(GetCurrBuff(), otz), poly);
			IncrementNextPrim(sizeof(POLY_FT4));

#if DRAW_GRID
			DrawGrid(poly);
#endif
		}
	} else {
		/*
		* 		Create 4 sub-quads
		*		==================
		*
		*	   (v0)		   (vm01)      (v1)
		*		*-----------*----------*
		*		|			|		   |
		*		|			|		   |
		*		|			|		   |	
		* (vm02)*-----------*----------*(vm13)
		*		|		  (vm03)	   |
		*		|			|		   |
		*		|			|		   |
		*		*-----------*----------*	
		*	   (v2)		  (vm32)	   (v3)
		
		*/

		// If lefel is < depth we keed sub-dividing out quad
		SVECTOR vm01 = (SVECTOR){(v0->vx + v1->vx) >> 1, (v0->vy + v1->vy) >> 1, (v0->vz + v1->vz) >> 1};
		SVECTOR vm02 = (SVECTOR){(v0->vx + v2->vx) >> 1, (v0->vy + v2->vy) >> 1, (v0->vz + v2->vz) >> 1};
		SVECTOR vm03 = (SVECTOR){(v0->vx + v3->vx) >> 1, (v0->vy + v3->vy) >> 1, (v0->vz + v3->vz) >> 1};
		SVECTOR vm12 = (SVECTOR){(v1->vx + v2->vx) >> 1, (v1->vy + v2->vy) >> 1, (v1->vz + v2->vz) >> 1};
		SVECTOR vm13 = (SVECTOR){(v1->vx + v3->vx) >> 1, (v1->vy + v3->vy) >> 1, (v1->vz + v3->vz) >> 1};
		SVECTOR vm32 = (SVECTOR){(v3->vx + v2->vx) >> 1, (v3->vy + v2->vy) >> 1, (v3->vz + v2->vz) >> 1};

 		u_short tum01 = (tu0 + tu1) >> 1; u_short tvm01 = (tv0 + tv1) >> 1;
    	u_short tum02 = (tu0 + tu2) >> 1; u_short tvm02 = (tv0 + tv2) >> 1;
    	u_short tum03 = (tu0 + tu3) >> 1; u_short tvm03 = (tv0 + tv3) >> 1;
    	u_short tum12 = (tu1 + tu2) >> 1; u_short tvm12 = (tv1 + tv2) >> 1;
    	u_short tum13 = (tu1 + tu3) >> 1; u_short tvm13 = (tv1 + tv3) >> 1;
    	u_short tum32 = (tu3 + tu2) >> 1; u_short tvm32 = (tv3 + tv2) >> 1;

    	RenderQuadRecursive(face,    v0, &vm01, &vm02, &vm03,   tu0,   tv0, tum01, tvm01, tum02, tvm02, tum03, tvm03, level + 1, depth);  // top-left subquad
    	RenderQuadRecursive(face, &vm01,    v1, &vm03, &vm13, tum01, tvm01,   tu1,   tv1, tum03, tvm03, tum13, tvm13, level + 1, depth);  // top-right subquad
    	RenderQuadRecursive(face, &vm02, &vm03,    v2, &vm32, tum02, tvm02, tum03, tvm03,   tu2,   tv2, tum32, tvm32, level + 1, depth);  // bottom-left subquad
    	RenderQuadRecursive(face, &vm03, &vm13, &vm32,    v3, tum03, tvm03, tum13, tvm13, tum32, tvm32,   tu3,   tv3, level + 1, depth);  // bottom-right subquad
	}
}

static inline void RenderTrackSection(Track *track, Section *section, Camera *camera, u_char depth) {
    SVECTOR v0, v1, v2, v3;

	for (u_long i = 0; i < section->numfaces; i++) {
		// Face *face = track->faces + section->facestart + i;
		/*
		 * Acquire the correct face by offsetting the track faces with the facestart index of the
		 * section + the current face index
		 */
		Face *face = &track->faces[section->facestart + i];
        v0.vx = Clamp16Bit(track->vertices[face->indices[1]].vx - camera->position.vx); // --> the indices order from the TRF file has the first index at 1 and the second at 0 hence the weird order
        v0.vy = Clamp16Bit(track->vertices[face->indices[1]].vy - camera->position.vy);
        v0.vz = Clamp16Bit(track->vertices[face->indices[1]].vz - camera->position.vz);
        v1.vx = Clamp16Bit(track->vertices[face->indices[0]].vx - camera->position.vx);
        v1.vy = Clamp16Bit(track->vertices[face->indices[0]].vy - camera->position.vy);
        v1.vz = Clamp16Bit(track->vertices[face->indices[0]].vz - camera->position.vz);
        v2.vx = Clamp16Bit(track->vertices[face->indices[2]].vx - camera->position.vx);
        v2.vy = Clamp16Bit(track->vertices[face->indices[2]].vy - camera->position.vy);
        v2.vz = Clamp16Bit(track->vertices[face->indices[2]].vz - camera->position.vz);
        v3.vx = Clamp16Bit(track->vertices[face->indices[3]].vx - camera->position.vx);
        v3.vy = Clamp16Bit(track->vertices[face->indices[3]].vy - camera->position.vy);
        v3.vz = Clamp16Bit(track->vertices[face->indices[3]].vz - camera->position.vz);


		RenderQuadRecursive(face, &v0, &v1, &v2, &v3, face->u0, face->v0, face->u1, face->v1,face->u2, face->v2, face->u3, face->v3, 0, depth);
	}
}

void RenderTrack(Track *track, Camera *camera) {
	Section *root = track->sections;
	Section *currsection = root;

	VECTOR dist;
	u_long distmagsq;  // square root of distance magnitude
	u_long distmag;
	const long cameraposvx = camera->position.vx;
	const long cameraposvy = camera->position.vy;
	const long cameraposvz = camera->position.vz;

    // Set matrices
    MATRIX worldmat, viewmat;
    VECTOR pos = {0}; 
    VECTOR scale = {ONE, ONE, ONE};
    SVECTOR rot = {0};

    RotMatrix(&rot, &worldmat);
    TransMatrix(&worldmat, &pos);
    ScaleMatrix(&worldmat, &scale);
    CompMatrixLV(&camera->rotmat, &worldmat, &viewmat);
    SetRotMatrix(&viewmat);
    SetTransMatrix(&viewmat);

	/*
	 * The linked list for the sections does not end with last entries next pointer being null.
	 * The last entries next element points back to the start of the list, so we use a do-while to
	 * handle this behaviour
	 */
	do {
		// Calculate distance between curr section center and camera
		dist.vx = Clamp16Bit(currsection->center.vx - cameraposvx);
		dist.vy = Clamp16Bit(currsection->center.vy - cameraposvy);
		dist.vz = Clamp16Bit(currsection->center.vz - cameraposvz);

		distmagsq = vectorSquared(&dist);
		distmag = SquareRoot12(distmagsq);  // 12 = fixed point
		if (distmag < MAX_DISTANCE) {
			u_char depth = 0;
			if (distmag < 200000) {
				depth = 2;
			} else if (distmag < 600000) {
				depth = 1;
			}
			RenderTrackSection(track, currsection, camera, depth);
		}

		currsection = currsection->next;
	} while (currsection != root);
}
