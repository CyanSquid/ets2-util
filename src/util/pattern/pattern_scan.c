#pragma once
#include <intrin.h>

#include "pattern_scan.h"
#include "pattern.h"

static bool g_setup = false;
static bool g_avx2 = false; // does CPU support avx2?
static bool g_sse4 = false; // does CPU support sse 4.1?

static inline void setup(void)
{
    if (g_setup)
    {
        return;
    }
    g_setup = true;

    {
        int cpuid[4];
        __cpuid(cpuid, 0);

        if (cpuid[0] >= 7)
        {
            __cpuidex(cpuid, 7, 0);
            g_avx2 = ((cpuid[1] & (1 << 5)) != 0);
        }

        if (cpuid[0] >= 1)
        {
            __cpuidex(cpuid, 1, 0);
            g_sse4 = ((cpuid[2] & (1 << 19)) != 0);
        }
    }
}

static inline bool does_byte_match(unsigned char data, unsigned char our_data, unsigned char our_mask)
{
    return !((data ^ our_data) & our_mask);
}

static inline bool does_pattern_match(const pattern* p, const unsigned char* lo)
{
    for (size_t i = 0; i < p->size; i++)
    {
        if (!does_byte_match(lo[i], p->data[i], p->mask[i]))
        {
            return false;
        }
    }
    return true;
}

static inline void align_pattern(const pattern* p, const size_t align, unsigned char* aligned_data, unsigned char* aligned_mask)
{
    for (size_t i = 0; i < align; ++i)
    {
        aligned_data[i] = i < p->size ? p->data[i] : 0x00;
        aligned_mask[i] = i < p->size ? p->mask[i] : 0x00;
    }
}

static inline void invert_mask(unsigned char* mask, const size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        mask[i] ^= 0xFF;
    }
}

static void* find_pattern_256(const pattern* p, const unsigned char* lo, const unsigned char* hi)
{
    unsigned char aligned_data[32];
    unsigned char aligned_mask[32];
    align_pattern(p, 32, aligned_data, aligned_mask);
    invert_mask(aligned_mask, 32);

    const __m256i mm_data = _mm256_loadu_si256(aligned_data);
    const __m256i mm_mask = _mm256_loadu_si256(aligned_mask);

    for (; lo < (hi - 32); lo++)
    {
        const __m256i mm_lo = _mm256_loadu_si256(lo);

        if (_mm256_testc_si256(mm_mask, _mm256_xor_si256(mm_lo, mm_data)))
        {
            return lo;
        }
    }
    return NULL;
}

static void* find_pattern_128(const pattern* p, const unsigned char* lo, const unsigned char* hi)
{
    unsigned char aligned_data[16];
    unsigned char aligned_mask[16];
    align_pattern(p, 16, aligned_data, aligned_mask);
    invert_mask(aligned_mask, 16);

    const __m128i mm_data = _mm_loadu_si128(aligned_data);
    const __m128i mm_mask = _mm_loadu_si128(aligned_mask);

    for (; lo < (hi - 32); lo++)
    {
        const __m128i mm_lo = _mm_loadu_si128(lo);

        if (_mm_testc_si128(mm_mask, _mm_xor_si128(mm_lo, mm_data)))
        {
            return lo;
        }
    }
    return NULL;
}

void* find_pattern(const char* p, const unsigned char* lo, const unsigned char* hi)
{
    pattern pattern = { 0 };
    if (!create_pattern(&pattern, p))
    {
        return NULL;
    }

    setup();

    if (pattern.size <= 32 && g_avx2)
    {
        return find_pattern_256(&pattern, lo, hi);
    }

    if (pattern.size <= 16 && g_sse4)
    {
        return find_pattern_128(&pattern, lo, hi);
    }

    for (; lo < (hi - pattern.size); lo++)
    {
        if (does_pattern_match(&pattern, lo))
        {
            return lo;
        }
    }

    return NULL;
}