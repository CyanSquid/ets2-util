#ifndef __UTIL__PATTERN__
#define __UTIL__PATTERN__

#include <stdbool.h>

#define MAX_PATTERN_SIZE (256)

typedef struct pattern pattern;

struct pattern
{
    unsigned char data[MAX_PATTERN_SIZE];
    unsigned char mask[MAX_PATTERN_SIZE];
    size_t size;
};

bool create_pattern(pattern* const n, const char* d);

#endif // __UTIL__PATTERN__