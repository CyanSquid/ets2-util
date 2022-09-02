#ifndef __UTIL__POINTER__
#define __UTIL__POINTER__

#include <stdint.h>

inline void* util__ptr_add(void* ptr, const size_t offset)
{
    return (char*)ptr + offset;
}

inline void* util__ptr_sub(void* ptr, const size_t offset)
{
    return (char*)ptr - offset;
}

inline void* util__ptr_rip(void* ptr, const size_t offset)
{
    const int32_t rel = *(int32_t*)ptr;
    ptr = util__ptr_add(ptr, rel);
    ptr = util__ptr_add(ptr, sizeof(rel));
    ptr = util__ptr_add(ptr, offset);
    return ptr;
}

#endif // __UTIL__LOGGING__