#include "model.h"

struct Mesh *meshInit(struct Vertex *vertices, size_t verticesSize, unsigned int *indices, size_t indicesSize, struct Texture *textures){
    struct Mesh *mesh = malloc(sizeof(struct Mesh));
    
    mesh->vertices = vertices;
    mesh->verticesSize = verticesSize;

    mesh->indices = indices;
    mesh->indicesSize = indicesSize;

    mesh->textures = textures;

    meshSetup(mesh);

    return mesh;
}

void meshSetup(struct Mesh *mesh){
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(struct Vertex) * mesh->verticesSize, mesh->vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh->indicesSize, mesh->indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLAT, GL_FALSE, sizeof(struct Vertex), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, normal));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, texCoords));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}
