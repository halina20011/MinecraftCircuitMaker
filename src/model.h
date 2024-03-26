#include <stdio.h>
#include <stdlib.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "graphics.h"

struct Vertex{
    vec3 position;
    vec3 normal;
    vec2 texCoords;
};

#define TEXTURE_DIFFUSE 0
#define TEXTURE_SPECULAR 1

struct Texture{
    unsigned int id;
    uint8_t type;
};

struct Mesh{
    struct Vertex *vertices;
    unsigned int *indices;
    struct Texture *textures;

    size_t verticesSize;
    size_t indicesSize;

    float position[3];
    float rotation[3];

    unsigned int VAO, VBO, EBO;
};

struct Mesh *meshInit(struct Vertex *vertices, size_t verticesSize, unsigned int *indices, size_t indicesSize, struct Texture *textures);
void meshSetup(struct Mesh *mesh);
