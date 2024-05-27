#ifndef G_HEADER
#define G_HEADER

#define IGNORE __attribute__ ((unused))
#define UNUSED(x) (void)(x)
#define UNUSEDS(...) (void)(__VA_ARGS__)

#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a < b) ? b : a)

#endif
