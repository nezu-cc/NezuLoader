#include "pch.h"
#include "NezuLoader.h"
#include "Hooks.h"
#include "GameEventListener.h"

extern Cheat* NezuLoader::cheat = new Cheat();

void NezuLoader::Load(HMODULE hModule) {

    Injector::SetMMapCallback(&MMCallback);

    L::Debug("capturing interfaces");
    I::CaptureInterfaces();
    L::Debug("finding offsets");
    M::FindAll();
    L::Debug("initializing netvars");
    NetVarManager::Initialize();
    L::Debug("hooking functions");
    H::ApplyHooks();
    L::Debug("registering callbacks");
    GameEventListener::singleton()->init();
    L::Debug("Locating event constructors");
    CreatePanoramaEvent("MatchAssistedAccept", true);

    Cfg::RefreshList();
    if (Cfg::LoadGlobalConfig()) {
        L::Info("global config loaded");
        if (!Cfg::global.default_config.empty()) {
            L::Debug("loading default config");
            Cfg::LoadConfig(Cfg::global.default_config);
        } 
        else 
            L::Debug("no default config set");
    }

    //this is just pasted as i don't expect this exploit to last long
    //so it will be stripped soon anyway
    //https://github.com/Harpoon-Inc/Harpoon/blob/a59dd07a36c7b0500aa77276d0ab4ee05069163a/Harpoon/Hooks.cpp#L1576

    auto clMoveChokeClamp = FindPattern("engine.dll", "B8 ? ? ? ? 3B F0 0F 4F F0 89 5D FC") + 1;

    auto nMaxRoutableClamp2 = FindPattern("engine.dll", "B8 ? ? ? ? EB 05 3B C6") + 1;

    auto TooManyQueuedPackets = FindPattern("steamnetworkingsockets.dll", "8D 0C 16") + 3;
    auto TooManyQueuedPackets2 = FindPattern("steamnetworkingsockets.dll", "3B 08 7E 54") + 2;

    if (clMoveChokeClamp > 0x1000) {
        unsigned long protect = 0;
        VirtualProtect((void*)clMoveChokeClamp, 4, PAGE_EXECUTE_READWRITE, &protect);
        *(std::uint32_t*)clMoveChokeClamp = 150;
        VirtualProtect((void*)clMoveChokeClamp, 4, protect, &protect);
    }
    else
        L::Error("failed to patch clMoveChokeClamp");

    if (nMaxRoutableClamp2 > 0x1000) {
        unsigned long protect1 = 0;
        VirtualProtect((void*)nMaxRoutableClamp2, 4, PAGE_EXECUTE_READWRITE, &protect1);
        *(std::uint32_t*)nMaxRoutableClamp2 = 36;
        VirtualProtect((void*)nMaxRoutableClamp2, 4, protect1, &protect1);
    }
    else
        L::Error("failed to patch nMaxRoutableClamp2");

    if (TooManyQueuedPackets > 0x1000) {
        unsigned long protect2 = 0;
        VirtualProtect((void*)TooManyQueuedPackets, 2, PAGE_EXECUTE_READWRITE, &protect2); /* cause jle to not set*/
        *(BYTE*)TooManyQueuedPackets = (BYTE)0x90;
        *(BYTE*)(TooManyQueuedPackets + sizeof(BYTE)) = (BYTE)0x90; /* Remove Compare */
        VirtualProtect((void*)TooManyQueuedPackets, 2, protect2, &protect2);
    }
    else
        L::Error("failed to patch TooManyQueuedPackets");
    
    if (TooManyQueuedPackets2 > 0x1000) {
        unsigned long protect3 = 0;
        VirtualProtect((void*)TooManyQueuedPackets2, 1, PAGE_EXECUTE_READWRITE, &protect3);
        if (*(BYTE*)(TooManyQueuedPackets2) == (BYTE)0x7E) {
            *(BYTE*)(TooManyQueuedPackets2) = (BYTE)0xEB; /* jle -> jmp */
        }
        VirtualProtect((void*)TooManyQueuedPackets2, 1, protect3, &protect3);
    }
    else
        L::Error("failed to patch TooManyQueuedPackets2");

    G::MainMenuPanel = TryGetMenuPanel(true, 100);
    L::Debug("found MainMenuPanel");
}

void NezuLoader::Unload() {
    Injector::SetMMapCallback(NULL);
    GameEventListener::singleton()->remove();
    H::RemoveHooks();
    I::InputSystem->enableInput(true);
    I::InputSystem->resetInputState();
}

BOOL NezuLoader::MMCallback(LPVOID mod) {
    PIMAGE_OPTIONAL_HEADER opt_hdr = &((PIMAGE_NT_HEADERS)((DWORD)mod + ((PIMAGE_DOS_HEADER)mod)->e_lfanew))->OptionalHeader;

    if (!CheatLoader(mod, opt_hdr->SizeOfImage))
        return FALSE;

    DWORD mod_crc = Crc32::hash((void*)mod, opt_hdr->SizeOfHeaders);
    NezuLoader::cheat->crc32 = mod_crc;

    L::Debug("Module CRC32: 0x%X\n", mod_crc);
    int res = -1;
    switch (mod_crc)
    {
    case 0x92823581: //OTC3
        NezuLoader::cheat->name = "OTC3";
        res = OTC3Loader(mod, opt_hdr->SizeOfImage) ? 1 : 0;
        break;
    default:
        NezuLoader::cheat->name = "unknown";
        break;
    }
    if (res != -1) {
        Cfg::LoadCurrentMods();
        return res;
    }
    return TRUE;
}
