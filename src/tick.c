#include <stddef.h>
#include <Windows.h>

#include "tick.h"
#include "util/pattern/pattern_scan.h"
#include "util/pointer.h"
#include "util/logging.h"

/* misc */

static unsigned char* g_scan_lo = NULL;
static unsigned char* g_scan_hi = NULL;

static bool get_module_region(const char* module, unsigned char** lo, unsigned char** hi)
{
    HMODULE module_handle;
    IMAGE_DOS_HEADER* module_d_header;
    IMAGE_NT_HEADERS* module_n_header;

    if ((lo == NULL) || (hi == NULL))
    {
        return false;
    }

    module_handle = GetModuleHandle(module);

    if (!module_handle)
    {
        return false;
    }

    module_d_header = (IMAGE_DOS_HEADER*)(module_handle);
    module_n_header = (IMAGE_NT_HEADERS*)(((unsigned char*)(module_d_header)) + module_d_header->e_lfanew);

    *lo = (unsigned char*)(module_handle);
    *hi = (unsigned char*)(module_handle)+module_n_header->OptionalHeader.SizeOfImage;

    return true;
}

static void* scan(bool* result, const char* const name, const char* p)
{
    void* addr = NULL;

    addr = find_pattern(p, g_scan_lo, g_scan_hi);
    if (addr)
    {
        LOG_DBG("Found (%p) \"%s\".", addr, name);
    }
    else
    {
        LOG_ERR("Failed to find \"%s\"", name);
    }

    *result &= (addr != NULL);
    return addr;
}

/* mod */

/*
* Naming convention:
*
* "game__" = game namespace.
* Example, "g_use_speed_limiter" is a global variable used by ETS2, so it's inside the game namespace.
* Anything in the game namespace attempts to use the correct name for the variable.
* g_use_speed_limiter is the correct variable name, so it's what I've used.
*
* "offsets__" = offsets namespace
* The offsets namespace is for class member offsets.
* Instead of creating a struct to represent the truck class, I've created signatures to find the offsets at runtime.
* I have done this because the truck class changes from time to time meaning member variables change offsets, and I don't want to have to update this code.
*
* "offsets__truck__" = offsets::truck namespace.
* This is for offsets related to the truck, as mentioned above.
* 
* 
* The switch between snake_case and cammelCase is because the game seems to use cammelCase for member variables. Again, I'm trying to be correct with the naming of variables.
*/
static void* game__g_use_speed_limiter = NULL;

static void** game__g_active_player = NULL;

static size_t offsets__truck__m_percentDamageChassis = 0x00;
static size_t offsets__truck__m_percentDamageWheels = 0x00;
static size_t offsets__truck__m_percentDamageEngine = 0x00;
static size_t offsets__truck__m_percentDamageTransmission = 0x00;
static size_t offsets__truck__m_percentDamageCabin = 0x00;
static size_t offsets__truck__m_percentFueltank = 0x00;

void* game__getActiveTruck(void)
{
    void* ptr = *game__g_active_player;

    if (ptr == NULL)
    {
        return NULL;
    }

    ptr = *(void**)util__ptr_add(ptr, 0x18);

    if (ptr == NULL)
    {
        return NULL;
    }

    ptr = *(void**)util__ptr_add(ptr, 0x80);
    return ptr;
}

static inline void set_float(void* ptr, const size_t offset, float value)
{
    if (ptr == NULL)
        return;

    *(float*)util__ptr_add(ptr, offset) = value;
}

static bool init_pointers(void)
{
    bool  foundall = true;
    void* location = NULL;

    if (!get_module_region(NULL, &g_scan_lo, &g_scan_hi))
    {
        return false;
    }

    if (location = scan(&foundall, "Speed Limiter", "67 5F 75 73 65 5F 73 70 65 65 64 5F 6C 69 6D 69 74 65 72"))
    {
        g_use_speed_limiter = location;
    }

    if (location = scan(&foundall, "Activce Player", "4C 8B 3D ? ? ? ? F3 0F 10 1D ? ? ? ? 49 3B 76 50 0F 83"))
    {
        location = util__ptr_add(location, 0x03);
        location = util__ptr_rip(location, 0x00);
        game__g_active_player = location;
    }

    if (location = scan(&foundall, "Damage Misc", "F3 0F 10 80 ? ? ? ? F3 0F 5F 80 ? ? ? ? F3 0F 5F 40 ? F3 0F 5F 80 ? ? ? ? 48 8B 01 41 0F 2F C3 0F 97 C2"))
    {
        offsets__truck__m_percentDamageChassis = *(uint8_t*)util__ptr_add(location, 0x14);
        offsets__truck__m_percentDamageEngine = *(uint32_t*)util__ptr_add(location, 0x0C);
        offsets__truck__m_percentDamageTransmission = *(uint32_t*)util__ptr_add(location, 0x19);
        offsets__truck__m_percentDamageCabin = *(uint32_t*)util__ptr_add(location, 0x04);
    }

    if (location = scan(&foundall, "Damage Wheels", "F3 0F 10 81 ? ? ? ? 0F 57 C9 0F 2F C1 4C 8B C1"))
    {
        offsets__truck__m_percentDamageWheels = *(uint32_t*)util__ptr_add(location, 0x04);
    }

    if (location = scan(&foundall, "Fuel Level", "89 83 ? ? ? ? 48 8B 05 ? ? ? ? 48 8B 88 ? ? ? ? 48 8B 01"))
    {
        offsets__truck__m_percentFueltank = *(uint32_t*)util__ptr_add(location, 0x02);
    }

    return foundall;
}

bool ets2util_init(void)
{
    if (!init_pointers())
    {
        LOG_FTL("Failed to find one or more patterns. Check log for details.");
        return false;
    }
    return true;
}

void ets2util_tick(void)
{
    static bool speed_limit_bypass = false;

    static DWORD64 last_tick = 0;
    const  DWORD64 crnt_tick = GetTickCount64();

    if ((crnt_tick - last_tick) > 2000)
    {
        last_tick = crnt_tick;
        set_float(game__getActiveTruck(), offsets__truck__m_percentDamageChassis, 0.f);
        set_float(game__getActiveTruck(), offsets__truck__m_percentDamageWheels, 0.f);
        set_float(game__getActiveTruck(), offsets__truck__m_percentDamageEngine, 0.f);
        set_float(game__getActiveTruck(), offsets__truck__m_percentDamageTransmission, 0.f);
        set_float(game__getActiveTruck(), offsets__truck__m_percentDamageCabin, 0.f);
        set_float(game__getActiveTruck(), offsets__truck__m_percentFueltank, 1.f);
    }

    *(bool*)util__ptr_add(game__g_use_speed_limiter, 0x110) = false;
}
