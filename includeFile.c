#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void makeHeader(const char fileName[], const char *name, char *content){
    // FILE *f = fopen(fileName, "w+");
    FILE *f = stdout;
    if(!f){
        fprintf(stderr, "failed to open destination file \"%s\"\n", fileName);
        exit(1);
    }

    fprintf(f, "#define %s = \"", name);
    char *c = content;
    while(*c){
        if(*c == '\n'){
            fprintf(f, "\\n");
        }
        else{
            fprintf(f, "%c", *c);
        }
        c++;
    }
    fprintf(f, "\"\n");
}

char *readFile(const char *fileName){
    FILE *file = fopen(fileName, "rb");
    if(!file){
        fprintf(stderr, "unable to open file: %s\n", fileName);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(sizeof(char) * (size + 1));
    fread(buffer, 1, size, file);
    fclose(file);

    buffer[size] = 0;
    // printf("%s\n", buffer);

    return buffer;
}

int main(){
    char *content = readFile("./src/blocks");
    makeHeader("./src/blocks.h", "BLOCKS", content);
    return 0;
}
