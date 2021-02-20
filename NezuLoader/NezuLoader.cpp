#include "pch.h"
#include "NezuLoader.h"
#include "Hooks.h"

void NezuLoader::Load(HMODULE hModule) {
    printf("[nezu] loading...\n");

    I::CaptureInterfaces();
    M::FindAll();
    H::ApplyHooks();

    printf("[nezu] loaded\n");
}

void NezuLoader::Unload() {

    H::RemoveHooks();


}
