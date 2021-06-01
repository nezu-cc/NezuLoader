#pragma once

extern IProcess* proc;

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
    std::string sandbox;
    bool AdvSettingsOpen = false;
    bool CloseAfterLoad = false;
    bool UseCustomCredentials = false;
    std::pair<std::string, std::string> Creds;
    std::string CustomSteamArgs;
    bool AlwaysOnTop = false;
} NezuInjectorSettings;

DWORD WINAPI LoaderThread(LPVOID lpThreadParameter);
