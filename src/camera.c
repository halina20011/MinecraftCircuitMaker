#include "camera.h"

void cameraInit(struct Camera *camera){
    SET_VEC3(0, 1, 0, camera->up);
    SET_VEC3(0, 0, 3, camera->cameraPos);

    SET_VEC3(0, 0, -1, camera->cameraFront);
    SET_VEC3(0, 1, 0, camera->cameraUp);

    camera->yaw = -90;
    camera->pitch = 0;
    camera->fov = 45;

    camera->prevX = -1;
    camera->prevY = 0;
}
