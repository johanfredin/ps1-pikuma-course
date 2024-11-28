#include "track.h"
#include "display.h"
#include "globals.h"
#include "inline_n.h"
#include "libgpu.h"
#include "texture.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>


void LoadTrackVertices(Track *track, char *filename) {
  u_long i, b, length;
  u_char *bytes;

  bytes = (u_char*) FileRead(filename, &length);

  if (bytes == NULL) {
    printf("Error reading %s from the CD.\n", filename);
    return;
  }

  b = 0;

  track->numvertices = length / BYTES_PER_VERTEX;
  track->vertices = (VECTOR*) malloc3(track->numvertices * sizeof(VECTOR));

  for (i = 0; i < track->numvertices; i++) {
    track->vertices[i].vx  = GetLongBE(bytes, &b);
    track->vertices[i].vy  = GetLongBE(bytes, &b);
    track->vertices[i].vz  = GetLongBE(bytes, &b);
    track->vertices[i].pad = GetLongBE(bytes, &b);
  }

  free3(bytes);
}

void LoadTrackFaces(Track *track, char *filename, u_short texturestart) {
  u_long i, b, length;
  u_char *bytes;
  Texture *texture;

  bytes = (u_char*) FileRead(filename, &length);

  if (bytes == NULL) {
    printf("Error reading %s from the CD.\n", filename);
    return;
  }

  b = 0;

  track->numfaces = length / BYTES_PER_FACE;
  track->faces = (Face*) malloc3(track->numfaces * sizeof(Face));

  for (i = 0; i < track->numfaces; i++) {
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
  u_long i, b, length;
  u_char *bytes;

  bytes = (u_char*) FileRead(filename, &length);

  if (bytes == NULL) {
    printf("Error reading %s from the CD.\n", filename);
    return;
  }

  b = 0;

  track->numsections = length / BYTES_PER_SECTION;
  track->sections = (Section*) malloc3(track->numsections * sizeof(Section));

  for (i = 0; i < track->numsections; i++) {
    b += 4;

    track->sections[i].prev = track->sections + GetLongBE(bytes, &b);
    track->sections[i].next = track->sections + GetLongBE(bytes, &b);

    track->sections[i].center.vx = GetLongBE(bytes, &b);
    track->sections[i].center.vy = GetLongBE(bytes, &b);
    track->sections[i].center.vz = GetLongBE(bytes, &b);

    b += 118;

    track->sections[i].facestart = GetShortBE(bytes, &b);
    track->sections[i].numfaces  = GetShortBE(bytes, &b);

    b += 4;

    track->sections[i].flags = GetShortBE(bytes, &b);
    track->sections[i].id = GetShortBE(bytes, &b);
    track->sections[i].id = i;

    b += 2;
  }

  free3(bytes);
}

void RenderTrackSection(Track *track, Section *section, Camera *camera) {
  int i;
  short nclip;
  long otz, p, flg;
  LINE_F2 *line0, *line1, *line2, *line3;
  POLY_FT4 *poly;

  MATRIX worldmat;
  MATRIX viewmat;

  VECTOR pos;
  VECTOR scale;
  SVECTOR rot;

  CVECTOR facecolor;

  SVECTOR v0, v1, v2, v3;

  setVector(&pos, 0, 0, 0);
  setVector(&rot, 0, 0, 0);
  setVector(&scale, ONE, ONE, ONE);

  worldmat = (MATRIX){0};
  RotMatrix(&rot, &worldmat);
  TransMatrix(&worldmat, &pos);
  ScaleMatrix(&worldmat, &scale);
  CompMatrixLV(&camera->rotmat, &worldmat, &viewmat);
  SetRotMatrix(&viewmat);
  SetTransMatrix(&viewmat);

  for (i = 0; i < section->numfaces; i++) {
    Face *face = track->faces + section->facestart + i;
    poly = (POLY_FT4*) GetNextPrim();

    v0.vx = (short) (track->vertices[face->indices[1]].vx - camera->position.vx);
    v0.vy = (short) (track->vertices[face->indices[1]].vy - camera->position.vy);
    v0.vz = (short) (track->vertices[face->indices[1]].vz - camera->position.vz);

    v1.vx = (short) (track->vertices[face->indices[0]].vx - camera->position.vx);
    v1.vy = (short) (track->vertices[face->indices[0]].vy - camera->position.vy);
    v1.vz = (short) (track->vertices[face->indices[0]].vz - camera->position.vz);

    v2.vx = (short) (track->vertices[face->indices[2]].vx - camera->position.vx);
    v2.vy = (short) (track->vertices[face->indices[2]].vy - camera->position.vy);
    v2.vz = (short) (track->vertices[face->indices[2]].vz - camera->position.vz);

    v3.vx = (short) (track->vertices[face->indices[3]].vx - camera->position.vx);
    v3.vy = (short) (track->vertices[face->indices[3]].vy - camera->position.vy);
    v3.vz = (short) (track->vertices[face->indices[3]].vz - camera->position.vz);

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

      // Draw lines for the quads
      line0 = (LINE_F2*) GetNextPrim();
      setLineF2(line0);
      setXY2(line0, poly->x0, poly->y0, poly->x1, poly->y1);
      setRGB0(line0, 255, 255, 0);
      addPrim(GetOTAt(GetCurrBuff(), 0), line0);
      IncrementNextPrim(sizeof(LINE_F2));

      line1 = (LINE_F2*) GetNextPrim();
      setLineF2(line1);
      setXY2(line1, poly->x1, poly->y1, poly->x3, poly->y3);
      setRGB0(line1, 255, 255, 0);
      addPrim(GetOTAt(GetCurrBuff(), 0), line1);
      IncrementNextPrim(sizeof(LINE_F2));

      line2 = (LINE_F2*) GetNextPrim();
      setLineF2(line2);
      setXY2(line2, poly->x3, poly->y3, poly->x2, poly->y2);
      setRGB0(line2, 255, 255, 0);
      addPrim(GetOTAt(GetCurrBuff(), 0), line2);
      IncrementNextPrim(sizeof(LINE_F2));

      line3 = (LINE_F2*) GetNextPrim();
      setLineF2(line3);
      setXY2(line3, poly->x2, poly->y2, poly->x0, poly->y0);
      setRGB0(line3, 255, 255, 0);
      addPrim(GetOTAt(GetCurrBuff(), 0), line3);
      IncrementNextPrim(sizeof(LINE_F2));
    }
  }
}

long Clamp16Bits(long value) {
  if (value > +32767) return +32767;
  if (value < -32767) return -32767;
  return value;
}

void RenderTrack(Track *track, Camera *camera) {
  Section *currsection;
  VECTOR d;
  u_long distmagsq;
  u_long distmag;

  currsection = track->sections;

  do {
    d.vx = Clamp16Bits(currsection->center.vx - camera->position.vx);
    d.vy = Clamp16Bits(currsection->center.vy - camera->position.vy);
    d.vz = Clamp16Bits(currsection->center.vz - camera->position.vz);
    distmagsq = (d.vx * d.vx) + (d.vy * d.vy) + (d.vz * d.vz);
    distmag = SquareRoot12(distmagsq);

    if (distmag < 1350000) {
      RenderTrackSection(track, currsection, camera);
    }

    currsection = currsection->next;
  } while (currsection != track->sections);
}