#include <Windows.h>
#include <stdbool.h>

#include "main.h"
#include "tick.h"
#include "util/logging.h"

static HMODULE g_hmodule = NULL;
static bool    g_running = true;

void ets2util_unload(void)
{
    g_running = false;
}

static void main_handler(void)
{
    if (ets2util_init())
    {
        while (g_running)
        {
            if ((GetAsyncKeyState(VK_DELETE) & 0x8000) && (GetAsyncKeyState(VK_END) & 0x8000))
                g_running = false;
            ets2util_tick();
            Sleep(10);
        }
    }
    else
    {
        LOG_FTL("ets2util failed to initialize. Check log for details.");
    }
}

static DWORD thread_entry(PVOID unused)
{
    (void)unused;
    logger__initialize();
    main_handler();
    logger__uninitialize();
    FreeLibraryAndExitThread(g_hmodule, 0);
    return S_OK;
}

BOOL APIENTRY DllMain(HMODULE hmod, DWORD reason, PVOID unused)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hmod);

        g_hmodule = hmod;
        const HANDLE thread_handle = CreateThread(NULL, 0, thread_entry, NULL, 0, NULL);

        if (!thread_handle)
            return FALSE;

        ResumeThread(thread_handle);
        CloseHandle(thread_handle);
    }
    return TRUE;
}
