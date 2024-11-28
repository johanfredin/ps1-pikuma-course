#include "track.h"

#include "display.h"
#include "globals.h"
#include "libgpu.h"
#include "libgte.h"
#include "malloc.h"
#include "texture.h"
#include "utils.h"
#include <inline_n.h>

#define BYTES_PER_VERTEX 16
#define BYTES_PER_FACE 20
#define BYTES_PER_SECTION 156

#define MAX_SIGNED_16_BIT 32767
#define MIN_SIGNED_16_BIT -32767

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

		texture += texturestart;
		texture = GetFromTextureStore(face->texture);
		face->tpage = texture->tpage;
		face->clut  = texture->clut;
		face->u0 = texture->u0;
		face->v0 = texture->v0;
		face->u1 = texture->u1;
		face->v1 = texture->v1;
		face->u2 = texture->u2;
		face->v2 = texture->v2;
		face->u3 = texture->u3;
		face->v3 = texture->v3;
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

static inline void RenderTrackSection(Track *track, Section *section, Camera *camera) {
	short nclip;
    long otz;

    SVECTOR v0, v1, v2, v3;
	LINE_F2 *line0, *line1, *line2, *line3;

    for (u_long i = 0; i < section->numfaces; i++) {
		// Face *face = track->faces + section->facestart + i;
		/*
		 * Acquire the correct face by offsetting the track faces with the facestart index of the
		 * section + the current face index
		 */
		Face *face = &track->faces[section->facestart + i];
		POLY_FT4 *poly = (POLY_FT4 *) GetNextPrim();
		
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

        
        gte_ldv0(&v0); 
		gte_ldv1(&v1);
		gte_ldv2(&v2);
		gte_rtpt();
		gte_nclip();
		gte_stopz(&nclip);
		if (nclip < 0) {
			continue;
		}
		gte_stsxy0(&poly->x0);
		gte_ldv0(&v3);
		gte_rtps();
		gte_stsxy3(&poly->x1, &poly->x2, &poly->x3);
		gte_avsz4();
		gte_stotz(&otz);
		if (otz > 0 && otz < OT_LEN) {
			setPolyFT4(poly);
			setRGB0(poly, face->color.r, face->color.g, face->color.b);
			poly->tpage = face->tpage;
			poly->clut  = face->clut;
			setUV4(poly, face->u0, face->v0, face->u1, face->v1, face->u2, face->v2, face->u3, face->v3);
			addPrim(GetOTAt(GetCurrBuff(), otz), poly);
			IncrementNextPrim(sizeof(POLY_FT4));
		}
	}
}

static inline long MagnitudeSquared(VECTOR *v) {
	return (v->vx * v->vx) + (v->vy * v->vy) + (v->vz * v->vz);
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

		distmagsq = MagnitudeSquared(&dist);
		distmag = SquareRoot12(distmagsq);  // 12 = fixed point
		if (distmag < 1350000) {
			RenderTrackSection(track, currsection, camera);
		}

		currsection = currsection->next;
	} while (currsection != root);
}
