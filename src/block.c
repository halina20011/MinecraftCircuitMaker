#include "block.h"

#ifndef BLOCKS_DIR_PATH
#define BLOCKS_DIR_PATH __BASE_FILE__
#endif

VECTOR_TYPE_FUNCTIONS(struct BlockType, BlockTypeVector, "")

// block file format
// line 1       =>  BLOCK
// line 2       =>  blockId
// line 3       =>  blockEnum
// line 3       =>  block states size (uint8_t)
// line 4 -     =>  blockStateEnum:[allowed values seperated with ","] 
//                  facing[east,north,south,west]
// line 5       =>  vertices size
// line 6       =>  vertices
// line 7       =>  indices size
// line 8       =>  indices
// line 9       =>  texture cords size
// line 10      =>  texture cords
// line 11      =>  texture size

// TODO: block hash map
///         block hash map size (uint16_t)
//          blockId:id(uint16_t)

// building file format
// line 0       =>  BUILDING
// line 1       =>  name
// line 2-3     =>  size x,y,x in uint64_t
// blocks in x y z order
// id:blockStateName:blockStateValue
// ...

#define MAX(a, b) ((a < b) ? b : a)
#define MIN(a, b) ((a < b) ? a : b)

void boundingBox(struct BoundingBox *bb, float *data, size_t size){
    for(size_t i = 0; i < size; i += 5){
        bb->minX = MIN(data[i + 0], bb->minX);
        bb->minY = MIN(data[i + 1], bb->minY);
        bb->minZ = MIN(data[i + 2], bb->minZ);
        
        bb->maxX = MAX(data[i + 0], bb->maxX);
        bb->maxY = MAX(data[i + 1], bb->maxY);
        bb->maxZ = MAX(data[i + 2], bb->maxZ);
    }
}


GLuint loadAllBlocks(const char textureFile[]){
    size_t size;
    uint8_t *data = readFile(textureFile, &size);
    printf("texture size %zu\n", size);
    uint16_t w, h;

    memcpy(&w, data, sizeof(uint16_t));
    memcpy(&h, data + sizeof(uint16_t), sizeof(uint16_t));
    
    printf("texture %ux%u\n", w, h);

    return loadTexture(data + sizeof(uint16_t) * 2, w, h);
}

struct BlockType loadBlock(const char fileName[]){
    FILE *file = fopen(fileName, "rb");
    if(file == NULL){
        fprintf(stderr, "failed to open file %s\n", fileName);
        exit(1);
    }
    
    int type;
    fread(&type, sizeof(int), 1, file);
    int size = 0;
    fread(&size, sizeof(int), 1, file);

    printf("block size: %i\n", size);
    float *data = malloc(sizeof(float) * size);

    fread(data, sizeof(float), size, file);
    if(feof(file)){
        fprintf(stderr, "file is shorter then the size specified %s\n", fileName);
        exit(1);
    }

    struct BlockType block;
    block.type = type;
    block.data = data;
    block.dataSize = size;

    boundingBox(&block.boundingBox, data, size);

    return block;
}

void drawBlock(struct BlockType block){
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * block.dataSize, block.data, GL_STATIC_DRAW);

    // float angle = glm_rad(20 * i);
    // float angle = glm_rad(20 * i);
    // glm_rotate(model, angle, (vec3){1, 0.3, 0.5});

    // glm_vec3_rotate(color, angle, (vec3){0.4, -1.4, 2});
    // glUniform3fv(colorUniform, 1, (float*)color);
    // glUniform3f(colorUniform, 0.5, 0.5, 0.5);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    size_t trigCount =  block.dataSize / 5;

    // float *d = block.data;
    // for(int i = 0; i < block.dataSize; i += 5){
    //     printf("%f %f %f | %f %f\n", d[i], d[i + 1], d[i + 2], d[i + 3], d[i + 4]);
    // }
    glDrawArrays(GL_TRIANGLES, 0, trigCount);
}

struct BlockType *loadBlocks(size_t *size){
    DIR *d;
    struct dirent *dir;

    struct BlockTypeVector *blockTypeVector = BlockTypeVectorInit();

    d = opendir(BLOCKS_DIR_PATH);
    FILE *stream = stdout;
    if(d){
        while((dir = readdir(d)) != NULL){
            if(dir->d_type != DT_DIR){
                char filePath[255];
                strcpy(filePath, BLOCKS_DIR_PATH);
                strcat(filePath, "/");
                strcat(filePath, dir->d_name);

                printf("%s %i\n", filePath, dir->d_type);
                struct BlockType blockType = loadBlock(filePath);
                BlockTypeVectorPush(blockTypeVector, blockType);
                // char *fileBuffer = readFile(filePath);
            }
        }
        closedir(d);
    }

    return BlockTypeVectorDuplicate(blockTypeVector,size);
}

void loadBuilding(const char fileName[]){

}
