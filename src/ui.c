#include "ui.h"

#define MAX(a, b) ((a < b) ? b : a)

VECTOR_TYPE_FUNCTIONS(struct UiElement*, UiElementPVector)

// first create and ui instance with uiInit();
// then define the ui from the top
//  the most top element is ui->root
//
// for better performance ui is baked and then in one vbo is send to gpu
// to bake all element use uiBake();
//
// in your loop use uiDraw();

struct Ui *uiInit(GLFWwindow *w){
    struct Ui *ui = malloc(sizeof(struct Ui));
    ui->window = w;
    ui->width = 0;
    ui->height = 0;

    ui->idCounter = 0;

    ui->uiElements = UiElementPVectorInit();

    struct UiElement *root = uiElementInit(ui);
    root->flags = ELEMENT_HIDDEN;

    ui->root = root;

    struct Shader *uiShader = shaderInit(VERTEX_UI_SHADER, FRAGMENT_UI_SHADER);
    ui->shader = uiShader;

    GLint posAttrib = glGetAttribLocation(uiShader->program, "uiPosition");
    GLint colorAttrib = glGetAttribLocation(uiShader->program, "uiColor");

    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
    glVertexAttribPointer(colorAttrib, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(sizeof(float) * 2));

    glEnableVertexAttribArray(posAttrib);
    glEnableVertexAttribArray(colorAttrib);

    return ui;
}

void uiElementCalc(struct Ui *ui, struct UiElement *element){
    // printf("el %p\n", element);
    // printf("par %p\n", element->parent);
    switch(element->posType){
        case RELATIVE_PX:   
            element->x = element->parent->x + element->iX;
            element->y = element->parent->y + element->iY;
            break;
        case RELATIVE_PERCENTAGE:
            element->x = element->parent->x + (element->iX / 100.0f) * element->parent->width;
            element->y = element->parent->y + (element->iY / 100.0f) * element->parent->height;
            break;
        case ABSOLUTE_PX:
            element->x = element->iX;
            element->y = element->iY;
            break;
        case ABSOLUTE_PERCENTAGE:
            element->x = (element->iX / 100.0f) * (float)ui->width;
            element->y = (element->iY / 100.0f) * (float)ui->height;
            break;
    }

    if(element->sizeType == PX){
        element->width = element->iWidth;
        element->height = element->iHeight;
    }
    else{
        // printf("i => %f %f\n", element->iWidth, element->iHeight);
        float pWidth = ui->width;
        float pHeight = ui->height;

        float scaleX = (element->iWidth / 100.0f);
        float scaleY = (element->iHeight / 100.f);
        if(element->sizeType == PERCENTAGE){
            pWidth = element->parent->width;
            pHeight = element->parent->height;
        }
        else if(element->sizeType == VW){
            scaleY = scaleX;
        }
        else if(element->sizeType == VH){
            scaleX = scaleY;
        }

        // printf("scale => %f %f %f %f\n", pWidth, scaleX, pHeight, scaleY);
        element->width = pWidth * scaleX;
        element->height = pHeight * scaleY;
    }

    ui->bakeSize++;
    // printf("calculated: %zu [%f %f] [%f %f]\n", element->id, element->x, element->y, element->width, element->height);
    // for(size_t i = 0; i < element->childrenSize; i++){
    //     uiElementCalc(ui, element->children[i]);
    // }
}

// to 0 -> size = -1 -> 1
float normalizeX(float val, float size){
    return (val / size * 2.0) - 1.0f;
}

float normalizeY(float val, float size){
    return ((val / size) * 2.0f - 1.0f) * -1.0f;
}

void printVertexData(float *data){
    printf(">>>\n");
    for(size_t v = 0; v < 4; v++){
        printf("    ");
        for(size_t i = 0; i < 6; i++){
            printf("%f ", data[v * 6 + i]);
        }
        printf("\n");
    }
}

// calculate
void uiBake(struct Ui *ui){
    glfwGetFramebufferSize(ui->window, &ui->width, &ui->height);
    struct UiElement *root = ui->root;
    root->posType = ABSOLUTE_PX;
    root->iX = 0;
    root->iY = 0;
    root->sizeType = PX;
    root->iWidth = ui->width;
    root->iHeight = ui->height;

    ui->bakeSize = 0;
    struct UiElementPVector *elements = ui->uiElements;
    printf("size %zu\n", ui->uiElements->size);
    printf("size %zu\n", elements->size);
    for(size_t i = 0; i < elements->size; i++){
        // printf("%zu %p\n", i, elements->data[i]->parent);
        uiElementCalc(ui, elements->data[i]);
    }

    free(ui->data);
    size_t dataSize = sizeof(float) * (ui->bakeSize * ELEMENT_DATE_SIZE);
    ui->data = malloc(dataSize);
    size_t dataIndex = 0;
    for(size_t j = 0; j < elements->size; j++){
        struct UiElement *e = ui->uiElements->data[j];
        if(e->flags != ELEMENT_HIDDEN){
            float data[ELEMENT_DATE_SIZE];
            float color[4] = {N_COLOR(e->color.r), N_COLOR(e->color.g), N_COLOR(e->color.b), 1.0f};
            data[0] = e->x;
            data[1] = e->y;

            data[6] = e->x + e->width;
            data[7] = e->y;

            data[12] = e->x;
            data[13] = e->y + e->height;

            data[18] = e->x + e->width;
            data[19] = e->y + e->height;

            for(size_t i = 0; i < 4; i++){
                // printf("%i %f %f\n", i, data[i * 6], data[i * 6 + 1]);
                data[i * 6 + 0] = normalizeX(data[i * 6 + 0], ui->width);
                data[i * 6 + 1] = normalizeY(data[i * 6 + 1], ui->height);
                memcpy(&data[i * 6 + 2], color, sizeof(float) * 4);
            }
            // printVertexData(data);
            memcpy(&ui->data[dataIndex], data, sizeof(float) * ELEMENT_DATE_SIZE);
            dataIndex += ELEMENT_DATE_SIZE;
        }
    }

    // float testData[24] = {
    //     //     p
    //     -1, 1, 1, 0, 0, 1,
    //     1, 1, 0, 1, 0, 1,
    //     -1, -1, 0, 0, 1, 1,
    //     1, -1, 0.5, 0, 0.5, 1,
    // };
    
    // printVertexData(ui->data);

    useShader(ui->shader);
    glBufferData(GL_ARRAY_BUFFER, dataSize, ui->data, GL_STATIC_DRAW);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ELEMENT_DATE_SIZE, ui->data, GL_STATIC_DRAW);
    // glBufferData(GL_ARRAY_BUFFER, dataSize, ui->data, GL_STATIC_DRAW);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(testData), testData, GL_STATIC_DRAW);
    // printf("baked %zu\n", dataSize);
}

void uiDraw(struct Ui *ui){
    useShader(ui->shader);
    // size_t i = 0;
    // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    for(size_t i = 0; i < ui->uiElements->size; i++){
        // printf("%i\n", i);
        // struct UiElement *e = ui->uiElements->data[i];
        glDrawArrays(GL_TRIANGLE_STRIP, i * 4, 4);
    }
}

struct UiElement *uiElementInit(struct Ui *ui){
    struct UiElement *element = malloc(sizeof(struct UiElement));
    
    element->id = ui->idCounter++;
    element->level = 0;
    // printf("new element %zu\n", element->id);
    element->parent = NULL;
    element->childrenSize = 0;
    element->color = (struct ElementColor){255, 255, 255};
    
    UiElementPVectorPush(ui->uiElements, element);
    // printf("new size %zu\n", ui->uiElements->size);

    return element;
}

struct UiElement *uiAddElement(struct UiElement *element, struct UiElement *parent, uint8_t posType, uint8_t sizeType, float x, float y, float width, float height){
    if(parent){
        // printf("adding parent %p %p\n", element, parent);
        parent->childrenSize++;
        element->parent = parent;
        element->level = parent->level + 1;
    }

    element->iX = x;
    element->iY = y;
    element->iWidth = width;
    element->iHeight = height;

    element->posType = posType;
    element->sizeType = sizeType;

    return element;
}
