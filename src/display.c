#include "display.h"

// void arrowInit(vec3 end){
//     // const float angle = 45;
//     // float distance = sin(glm_rad(angle)) * 0.1;
//     // UNUSED(distance);
//     float line[] = {
//         0, 0, 0, 
//         end[0], end[1], end[2]
//     };
//
//     glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_DYNAMIC_DRAW);
//     glDrawArrays(GL_LINES, 0, 2);
// }

void drawGrid(int X, int Y, int Z, int width, int depth, int height){
    for(int z = 0; z < height; z++){
        for(int y = 0; y < depth; y++){
            drawLine(X, y + Y, z, X + width, y + Y, z);
        }
        //
        // for(int x = 0; x < width; x++){
        //     drawLine();
        // }
    }
}
