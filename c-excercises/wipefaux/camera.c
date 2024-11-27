#include "camera.h"

static inline void VectorCross(VECTOR *a, VECTOR *b, VECTOR *out) {
	OuterProduct12(a, b, out);	// OuterProduct12 = fixed point
}

void CameraLookAt(Camera *camera, VECTOR *target, VECTOR *up) {
	VECTOR eye = camera->position;
    VECTOR xright;
	VECTOR yup;
	VECTOR zforward;
	VECTOR x, y, z;
	VECTOR pos;
	VECTOR t;

	zforward.vx = target->vx - eye.vx;
	zforward.vy = target->vy - eye.vy;
	zforward.vz = target->vz - eye.vz;
	VectorNormal(&zforward, &z);

	VectorCross(&z, up, &xright);
	VectorNormal(&xright, &x);

	VectorCross(&z, &x, &yup);
	VectorNormal(&yup, &y);

	camera->lookat.m[0][0] = x.vx;
	camera->lookat.m[0][1] = x.vy;
	camera->lookat.m[0][2] = x.vz;
	camera->lookat.m[1][0] = y.vx;
	camera->lookat.m[1][1] = y.vy;
	camera->lookat.m[1][2] = y.vz;
	camera->lookat.m[2][0] = z.vx;
	camera->lookat.m[2][1] = z.vy;
	camera->lookat.m[2][2] = z.vz;

	// Translation part
	pos.vx = -eye.vx;
	pos.vy = -eye.vy;
	pos.vz = -eye.vz;

	// Save the rotation part into camera->rotmat
	camera->rotmat.m[0][0] = x.vx;
	camera->rotmat.m[0][1] = x.vy;
	camera->rotmat.m[0][2] = x.vz;
	camera->rotmat.m[1][0] = y.vx;
	camera->rotmat.m[1][1] = y.vy;
	camera->rotmat.m[1][2] = y.vz;
	camera->rotmat.m[2][0] = z.vx;
	camera->rotmat.m[2][1] = z.vy;
	camera->rotmat.m[2][2] = z.vz;

	// Proceed to combine both rotation and translation into camera->lookat
	ApplyMatrixLV(&camera->lookat, &pos, &t);
	TransMatrix(&camera->lookat, &t);
}