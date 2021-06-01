#include "pch.h"
#include "NezuLoader.h"

bool load_lock = false;
std::condition_variable load_lock_cv;
std::mutex load_lock_m;

DWORD WINAPI MainThread(HMODULE hModule) {

    while (!GetModuleHandleA("serverbrowser.dll"))
        Sleep(50);

    NezuLoader::Load(hModule);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
#if _DEBUG && 1
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        //freopen("CONIN$", "r", stdin);
#endif
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

#define EXPORT comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)

extern "C" DWORD WINAPI Inject(PCWCHAR path) {
#pragma EXPORT
    if (!path)
        return FALSE;
    
    // wait for our dll to fully init before injecting anything
    // seems to work weel but am I doing thins the right way? Im bad at modern c++
    std::unique_lock<std::mutex> lk(load_lock_m);
    if (!load_lock) {
        load_lock_cv.wait(lk, [] {return load_lock; });
    }

    NezuLoader::cheat->filepath = WCHARUTF8(path);
    NezuLoader::cheat->LoadState = Cheat::LoadState::Mapping;
    L::Debug("Injecting %s", NezuLoader::cheat->filepath.c_str());
    bool ret = Injector::ManualMap(GetCurrentProcess(), path);
    NezuLoader::cheat->LoadState = ret ? Cheat::LoadState::Mapped : Cheat::LoadState::Error;
    if (ret)
        L::Info("%s injected successfully", NezuLoader::cheat->filepath.c_str());
    else
        L::Error("failed to inject %s", NezuLoader::cheat->filepath.c_str());

    return ret ? TRUE : FALSE;
}