#include "blocks.h"

#define COMPARE(str, ID)do{\
    if(strcmp(id, str) == 0){\
        return ID;\
    }\
}while(0)

uint16_t blockId(const char *id){
    COMPARE("redstone_wire", REDSTONE_WIRE);
    COMPARE("redstone_torch", REDSTONE_TORCH);
    COMPARE("piston", PISTON);
    COMPARE("sticky_piston", STICKY_PISTON);
    COMPARE("target", TARGET);

    fprintf(stderr, "block type was not found %s\n", id);
    return BLOCK_NOT_FOUND;
}
