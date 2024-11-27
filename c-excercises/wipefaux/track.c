#include "track.h"
#include "libgte.h"
#include "malloc.h"
#include "utils.h"

#define BYTES_PER_VERTEX 16
#define BYTES_PER_FACE 20
#define BYTES_PER_SECTION 156

static inline void PadSkip(u_short skip, u_long *i) {
    *i += skip;
}

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
        track->vertices[i].pad = GetLongBE(bytes, &b); // load padding as well
    }

    free3(bytes);
}

void LoadTrackFaces(Track *track, char *filename) {
	u_long length;
	u_char *bytes = (u_char *)FileRead(filename, &length);
    
    u_long b = 0;

    track->numfaces = length / BYTES_PER_FACE;
    track->faces = calloc3(track->numfaces, sizeof(Face));

    for (int i = 0; i < track->numfaces; i++) {
        Face *face = &track->faces[i];
        face->indices[0] = GetShortBE(bytes, &b);
        face->indices[1] = GetShortBE(bytes, &b);
        face->indices[2] = GetShortBE(bytes, &b);
        face->indices[3] = GetShortBE(bytes, &b);

        face->normal.vx = GetShortBE(bytes, &b);
        face->normal.vy = GetShortBE(bytes, &b);
        face->normal.vz = GetShortBE(bytes, &b);
        // face->normal.pad = GetShortBE(bytes, &b);

        face->texture = GetChar(bytes, &b);

        face->flags = GetChar(bytes, &b);

        face->color.r = GetChar(bytes, &b);
        face->color.g = GetChar(bytes, &b);
        face->color.b = GetChar(bytes, &b);
        face->color.cd = GetChar(bytes, &b);
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
        PadSkip(4, &b);     // Information about junction index (skipped for now)

        Section *section = &track->sections[i];
        section->prev = track->sections + GetLongBE(bytes, &b);
        section->next = track->sections + GetLongBE(bytes, &b);

        section->center.vx = GetLongBE(bytes, &b);
        section->center.vy = GetLongBE(bytes, &b);
        section->center.vz = GetLongBE(bytes, &b);
        // section->center.pad = GetLongBE(bytes, &b);

        PadSkip(118, &b);

        section->facestart = GetShortBE(bytes, &b);
        section->numfaces = GetShortBE(bytes, &b);

        PadSkip(4, &b);

        section->flags = GetShortBE(bytes, &b);
        // section->id = GetShortBE(bytes, &b); // we dont care about this id so we simply skip and write over
        section->id = i;

        PadSkip(4, &b);
    }

    free3(bytes);
}

void RenderTrack(Track *track, Camera *camera) {
    Section *root = track->sections;
    
}
