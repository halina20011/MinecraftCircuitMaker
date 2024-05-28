#ifndef TYPES
#define TYPES

#include "stdint.h"

#define MAX_BLOCK_ID_SIZE UINT8_MAX

typedef char PathType[255];
typedef int32_t BuildingPosition[3];
#define blockPosVec3(a) (vec3){a[0], a[1], a[2]}

typedef uint8_t ChunkPosition;


// Buildings FILE FORMAT
// - Blocks

// SCENE FILE FORMAT 
// - Blocks
// - Buildings

                                    // # Blocks
typedef uint16_t BlockTypesSize;    // size of blockTypes (uint16_t)
                                    // ----- BLOCKTYPE ENTRY -----
typedef uint8_t BlockTypeIdStrSize; //  size of idStr (uint8_t)]
typedef char BlockTypeIdStr;        //  idStr (char*)
typedef uint16_t BlockTypeId;       //  blockId (uint16_t)
                                    //
typedef uint32_t BlocksSize;        // number of blocks (uint32_t)
                                    // ----- BLOCK ENTRY -----
                                    //  id (BlockTypeId) 
typedef int32_t BlockPosition[3];   //  abs pos (x, y, z BlockPosition)
typedef uint8_t BlockRotation;      //  facing rotation of block (uint8_t)

                                    // # Building
                                    // path and pathSize are used for 
                                    //  reloading and when "exporting a scene"
typedef uint8_t BuildingPathSize;   // building path size (uint8_t)
typedef char* BuildingPath;         // building path (char*)
typedef char BuildingName;      //
typedef char BuildingNameSize;      //
                                    // BlockSize

                                    // # Buildings
typedef uint16_t BuildingTypesSize; // size of buildingTypes (uint16_t)
                                    // ----- BUILDING TYPE ENTRY -----
typedef uint16_t BuildingTypeId;    // building id
typedef uint8_t BuildingPathSize;   //  building size path (uint8_t)
typedef char* BuildingPath;         //  building path (char*)
                                    //
typedef uint32_t BuildingTypeSize;  // number of buildings (uint32_t)
                                    // ----- BUILDING ENTRY -----
                                    //  id (uint16_t)
                                    //  rotation (uint8_t)
                                    //  abs pos of origin (x, y, z)

// # BUILDING FILE FORMAT
typedef uint32_t BuildingSize;      // size of blockTypes (uint16_t)
// ----- Building ENTRY -----
//  BuildingId
//
// number of blocks (uint32_t)
// ----- BLOCK ENTRY -----
//  id (uint16_t) 
//  abs pos (x, y, z int16_t)
//  rotation (uint8_t)


#endif
