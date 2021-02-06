#include "pch.h"
#include "Hooks.h"

DWORD WINAPI MainThread(HMODULE hModule) {

#if 1
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    //freopen("CONIN$", "r", stdin);
#endif

    while (!GetModuleHandleA("serverbrowser.dll"))
        Sleep(50);

    printf("[nezu] loading...\n");

    I::CaptureInterfaces();
    M::FindAll();
    H::ApplyHooks();

    printf("[nezu] loaded\n");

    //printf("[nezu] x=%X\n", (DWORD)H::oHooked_EndScene);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

