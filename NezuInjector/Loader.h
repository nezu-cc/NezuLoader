#pragma once

enum class InjectionMode {
    NezuLoader = 0,
    Dll,
    DllManual,
    NezuVacOnly,
};

typedef struct t_NezuInjectorSettings {
public:
    bool RestartSteam = true;
    bool VacBypass = true;
    InjectionMode injectionMode = InjectionMode::Dll;
    std::string dll;
} NezuInjectorSettings;

DWORD WINAPI LoaderThread(LPVOID lpThreadParameter);
