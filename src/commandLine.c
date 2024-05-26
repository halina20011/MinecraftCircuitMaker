#include "commandLine.h"

extern struct Text *text;

size_t commandLineDepth(struct Option *option){
    if(option){
        return 0;
    }

    size_t max = 0;
    for(size_t i = 0; i < option->optionsSize; i++){
        size_t curr = commandLineDepth(option->options[i]);
        if(max < curr){
            max = curr;
        }
    }

    if(option->type == OPTION_MULTY){
        max++;
    }

    return max + 1;
}

struct CommandLine *commandLineInit(struct Option *rootOption){
    struct CommandLine *cmd = malloc(sizeof(struct CommandLine));
    size_t maxDepth = commandLineDepth(rootOption);
    
    cmd->optionsIndicies = malloc(sizeof(size_t) * maxDepth);
    cmd->matchSize = 0;

    cmd->rootOption = rootOption;
    cmd->state = 0;
    cmd->command = malloc(sizeof(char) * (COMMAND_MAX_SIZE + 1));
    cmd->commandSize = 0;
    cmd->active = false;

    return cmd;
}

struct Option *optionNew(const char name[], void *function, size_t size, ...){
    struct Option *option = malloc(sizeof(struct Option));
    option->name = strdup(name);
    option->names = NULL;
    option->type = OPTION_SINGLE;
    option->options = malloc(sizeof(struct Option) * size);
    option->optionsSize = size;

    va_list va;
    va_start(va, size);
    for(size_t i = 0; i < size; i++){
        option->options[i] = va_arg(va, struct Option*);
    }
    va_end(va);
    option->function = function;

    return option;
}

struct Option *optionList(char **list, size_t listSize, void *function, size_t size, ...){
    struct Option *option = malloc(sizeof(struct Option));
    option->name = NULL;
    option->names = list;
    option->namesSize = listSize;
    option->type = OPTION_MULTY;
    option->options = malloc(sizeof(struct Option) * size);
    option->optionsSize = size;

    va_list va;
    va_start(va, size);
    for(size_t i = 0; i < size; i++){
        option->options[i] = va_arg(va, struct Option*);
    }
    va_end(va);

    option->function = function;

    return option;
}

void optionPrint(struct Option *option, uint8_t level){
    if(!option){
        return;
    }

    for(size_t i = 0; i < option->optionsSize; i++){
        struct Option *curr = option->options[i];
        for(uint8_t i = 0; i < level; i++){
            printf("│   ");
        }
        if(curr->type == OPTION_SINGLE){
            printf("├── %s\n", curr->name);
        }
        else{
            printf("├── ");
            for(size_t j = 0; j < curr->namesSize; j++){
                printf("%s,", curr->names[j]);
            }
            printf("\n");
        }
        optionPrint(curr, level + 1);
    }
}

void commandLineDraw(struct CommandLine *cmd, GLint modelUniformLocation, GLint colorUniform){
    textDrawOnScreen(text, cmd->command, -1, -1.0f + 0.02f, modelUniformLocation);
}

void commandSugestion(struct CommandLine *cmd, struct Option *o, bool *valid){

}

size_t countSize(char *c){
    size_t s = 0;
    while(c[s] && c[s] != ' '){
        s++;
    }
    return s;
}

struct Option *commandLineCurrOption(struct CommandLine *cmd){
    struct Option *curr = cmd->rootOption;
    bool found = false;
    char *command = &cmd->command[1];
    printf("===== start =====\n");
    size_t matchSize = 0;
    while(curr){
        size_t size = countSize(command);
        found = false;
        if(size == 0){
            printf("no more characters, end");
            cmd->matchSize = matchSize;
            return curr;
        }

        printf("curr word size %zu\n", size);
        printf("- curr option '%s' has %zu options\n", curr->name, curr->optionsSize);
        for(size_t i = 0; i < curr->optionsSize && !found; i++){
            struct Option *currOption = curr->options[i];
            if(currOption->type == OPTION_SINGLE){
                size_t sSize = strlen(currOption->name);
                printf("trying '%s' == '%s'\n", currOption->name, command);
                if(size == sSize && strncmp(command, currOption->name, size) == 0){
                    printf("matched %s\n", currOption->name);
                    curr = currOption;
                    found = true;
                    cmd->optionsIndicies[matchSize++] = i;
                }
            }
            else{
                for(size_t o = 0; o < currOption->namesSize && !found; o++){
                    size_t sSize = strlen(currOption->names[o]);
                    printf("trying '%s' == '%s'\n", currOption->names[o], command);
                    if(size == sSize && strncmp(command, currOption->names[o], size) == 0){
                        printf("matched %s\n", currOption->names[o]);
                        curr = currOption;
                        found = true;
                        cmd->optionsIndicies[matchSize++] = i;
                        cmd->optionsIndicies[matchSize++] = o;
                    }
                }
            }
        }
        
        if(!found){
            printf("not found\n");
            cmd->matchSize = 0;
            return NULL;
        }
        else{
            bool space = command[size] == ' ';
            command = &command[size + space];
        }
    }

    cmd->matchSize = 0;
    return NULL;
}

void commandLineExecute(struct CommandLine *cmd){
    struct Option *option = commandLineCurrOption(cmd);
    if(option && option->function){
        printf("%p\n", option->function);
        void (*f)() = option->function;
        f();
    }
}

void drawCommandLine(char *text, uint8_t size, struct Graphics *g){

}

void questionBool(char *question){
    
}

void commandLineStateMachine(struct CommandLine *cmd){
    
}
