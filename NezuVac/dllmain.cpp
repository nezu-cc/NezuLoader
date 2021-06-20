#include "pch.h"
#include "NezuVac.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(hModule);
        vac_status.loaded = TRUE;
        HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)NezuVac::MainThread, hModule, 0, NULL);
        if(hThread)
            CloseHandle(hThread);
    }
    break;
    default:
        break;
    }
    return TRUE;
}

