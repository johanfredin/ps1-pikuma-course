#include "camera.h"
#include "libgte.h"

static inline void VectorCross(VECTOR *a, VECTOR *b, VECTOR *out) {
    OuterProduct12(a, b, out);  // OuterProduct12 = fixed point
}

void CameraLookAt(Camera *camera, VECTOR *target, VECTOR *up) {

    VECTOR eye = camera->position;
    VECTOR xright;              // right (up) vector
    VECTOR yup;                 // up (y) vector
    VECTOR zforward;            // forward (z) vector

    VECTOR nx, ny, nz;          // normalized right/up/forward vectors

    VECTOR pos;                 // position (temporary) vector
    VECTOR t;                   // translation (temporary) vector

    // Compute the forward vector and normalize it
    zforward.vx = target->vx - eye.vx;
    zforward.vy = target->vy - eye.vy;
    zforward.vz = target->vz - eye.vz;
    VectorNormal(&zforward, &nz);

    // Compute the right vector and normalize it
    VectorCross(&nz, up, &xright);
    VectorNormal(&xright, &nx);

    // Compute the up vector and normalize it
    VectorCross(&nz, &nx, &yup);
    VectorNormal(&yup, &ny);

    // Populate the lookat matrix with the rotation values (forward, up and right values per row)
    camera->lookat.m[0][0] = nx.vx;     camera->lookat.m[0][1] = nx.vy;     camera->lookat.m[0][2] = nx.vz;
    camera->lookat.m[1][0] = ny.vx;     camera->lookat.m[1][1] = ny.vy;     camera->lookat.m[1][2] = ny.vz;
    camera->lookat.m[2][0] = nz.vx;     camera->lookat.m[2][1] = nz.vy;     camera->lookat.m[2][2] = nz.vz;

    pos.vx = -eye.vx;
    pos.vy = -eye.vy;
    pos.vz = -eye.vz;

    ApplyMatrixLV(&camera->lookat, &pos, &t);    // Multiply the vector by the matrix
    TransMatrix(&camera->lookat, &t);                // Populate the translation terms
}