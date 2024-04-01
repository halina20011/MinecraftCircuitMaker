#include "text.h"

uint8_t textBitmap[BITMAP_SIZE];

void readBitmap(const char fileName[]){
    FILE *file = fopen(fileName, "rb");
    if(!file){
        fprintf(stderr, "unable to open file: %s\n", fileName);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    fread(textBitmap, 1, size, file);
    fclose(file);
}

struct Text *textInit(){
    struct Text *text = malloc(sizeof(struct Text));

    readBitmap("src/bitmap.raw");

    text->shader = shaderInit(VERTEX_TEXT_SHADER, FRAGMENT_TEXT_SHADER);
    useShader(text->shader);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    text->colorUniform = getUniformLocation(text->shader, "textureColor");
    text->textureUniform = getUniformLocation(text->shader, "textureSampler");
    
    uint8_t tempTexture[TEXTURE_SIZE * TEXTURE_SIZE];

    int offset = 0;
    for(uint8_t i = 0; i < 127; i++){
        text->asciiMap[i] = 0;
        if(isprint(i)){

            // copy the texture
            float sum = 0;
            for(int y = 0; y < TEXTURE_SIZE; y++){
                for(int x = 0; x < TEXTURE_SIZE; x++){
                    int fromIndex = (y * TEXTURE_SIZE + x + offset);
                    int index = ((TEXTURE_SIZE - y - 1) * TEXTURE_SIZE + x);
                    sum += (float)textBitmap[fromIndex];
                    tempTexture[index] = textBitmap[fromIndex];
                }
            }

            offset += TEXTURE_SIZE * TEXTURE_SIZE;
            
            GLuint textureName;
            glGenTextures(1, &textureName);
            glBindTexture(GL_TEXTURE_2D, textureName);
            // printf("char %c => texure name: %i avg %f\n", i, textureName, sum / (float)(w * h));
            text->asciiMap[i] = textureName;
            // printf("t => %c %i %i\n", i, i, textureName);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                        
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, tempTexture);
        }
    }

    return text;
}

void textDraw(struct Text *text, char *str, float x, float y, float size){
    float width = size;
    float height = size * 1.9;
    useShader(text->shader);

    glDisable(GL_DEPTH_TEST);
    for(int i = 0; str[i]; i++){
        float vertices[] = {
            x               , y             ,  0.0f, 0.0f,
            x               , y + height    ,  0.0f, 1.0f,
            x + width       , y             ,  1.0f, 0.0f,
            x + width       , y + height    ,  1.0f, 1.0f,
        };

        GLint texture = text->asciiMap[(int)str[i]];
        // printf("texture %i %i\n", texture, str[i]);
        glActiveTexture(GL_TEXTURE0 + texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(text->textureUniform, texture);
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        x += width * 0.6;
    }
}

void textColor(struct Text *text, vec3 color){
    glUniform3fv(text->colorUniform, 1, (float*)color);
}
