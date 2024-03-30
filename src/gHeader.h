#ifndef G_HEADER
#define G_HEADER

#define IGNORE __attribute__ ((unused))
#define UNUSED(x) (void)(x)
#define UNUSEDS(...) (void)(__VA_ARGS__)

#define APPEND_STRING(buffer, size, maxSize, val){\
    if(maxSize <= size){\
        fprintf(stderr, "command is bigger then the max buffer size %i\n", MAX_COMMAND_BUFFER_SIZE); \
        exit(1);\
    }\
    buffer[size++] = val;\
}

#endif
