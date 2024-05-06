#ifndef CAMERA
#define CAMERA


#define CGLM_DEFINE_PRINTS 1
#define DEBUG 1

#include <cglm/cglm.h>
#include <cglm/types.h>
#include <cglm/io.h>

#define SET_VEC3(v1, v2, v3, vec){\
    vec[0] = v1;\
    vec[1] = v2;\
    vec[2] = v3;\
}

struct Camera{
    mat4 viewMatrix;

    vec3 up;
    vec3 cameraPos;

    vec3 cameraFront;
    vec3 cameraUp;

    float prevX, prevY;
    float yaw, pitch;
    float fov;
};

void cameraInit(struct Camera *camera);

#endif
