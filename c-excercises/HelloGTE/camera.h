#ifndef CAMERA_H
#define CAMERA_H

#include <libgte.h>

typedef struct Camera {
    VECTOR position;
    SVECTOR rotation;
    MATRIX lookat;
} Camera;

void CameraLookAt(Camera *camera, VECTOR *target, VECTOR *up);

#endif