#ifndef COMMAND_LINE
#define COMMAND_LINE

#include <stdio.h>
#include <stdarg.h>

#include "graphics.h"
#include "text.h"

#define COMMAND_MAX_SIZE 1024

enum OptionType{
    OPTION_SINGLE,
    OPTION_MULTY,
    OPTION_ARGUMENT
};

struct Option{
    char *name;
    char **names;
    size_t namesSize;
    struct Option **options;
    size_t optionsSize;
    void *function;
    enum OptionType type;
};

struct CommandLine{
    struct Option *rootOption;
    size_t *optionsIndicies;
    size_t matchSize;
};

struct CommandLine *commandLineInit(struct Option *rootOption);
void commandLineDraw(struct CommandLine *cmd, GLint modelUniformLocation, GLint colorUniform);

void commandLinePaste();
void commandLineCopy(struct GLFWwindow *w);

struct Option *commandLineCurrOption(struct CommandLine *cmd);
void commandSugestion(struct CommandLine *cmd, struct Option *o, bool *valid);

void commandLineExecute(struct CommandLine *cmd);

struct Option *optionNew(const char name[], void *function, size_t size, ...);
struct Option *optionList(char **list, size_t listSize, void *function, size_t size, ...);
struct Option *optionArgument(void *function, size_t size, ...);

void optionPrint(struct Option *option, uint8_t level);

#endif
