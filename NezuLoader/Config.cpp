#include "pch.h"
#include "Config.h"

std::vector<std::string> Cfg::configs;

Cfg::cfg_global Cfg::global;
Cfg::cfg Cfg::c;

json mods;

LPCWSTR Cfg::GetPath(LPCSTR cfg_name, bool global) {

    static WCHAR path[MAX_PATH] = { 0 };

    if (path[0] == 0) {
        PWSTR appdata_dir = NULL;
        if (FAILED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &appdata_dir))) {
            CoTaskMemFree(appdata_dir);
            L::Error("Failed to find config path (SHGetKnownFolderPath) 0x%X", GetLastError());
            return NULL;
        }

        if (!PathCombineW(path, appdata_dir, L"Nezu")) {
            CoTaskMemFree(appdata_dir);
            path[0] = 0;
            L::Error("Failed to find config path (PathCombineW - 1) 0x%X", GetLastError());
            return NULL;
        }

        CoTaskMemFree(appdata_dir);

        if (!CreateDirectoryW(path, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
            L::Error("Failed to find config path (CreateDirectoryW - 1) 0x%X", GetLastError());
            path[0] = 0;
            return NULL;
        }
    }

    static WCHAR final_path[MAX_PATH]; //ik this is bad, but the address of this never gets saved and race conditions are imposible
    if (!global) {
        if (!PathCombineW(final_path, path, L"NezuLoader")) {
            L::Error("Failed to find config path (PathCombineW - 2) 0x%X", GetLastError());
            final_path[0] = 0;
            return NULL;
        }

        if (!CreateDirectoryW(final_path, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
            L::Error("Failed to find config path (CreateDirectoryW - 2) 0x%X", GetLastError());
            final_path[0] = 0;
            return NULL;
        }
    }

    if (cfg_name) {
        if (!PathCombineW(final_path, global ? path : final_path, (UTF8WCHAR(cfg_name) + L".cfg").c_str())) {
            L::Error("Failed to find config path (PathCombineW - 3) 0x%X", GetLastError());
            path[0] = 0;
            return NULL;
        }
    }
    else {
        wcscpy_s(final_path, global ? path : final_path);
    }

    return final_path;
}

void Cfg::RefreshList() {
    L::Debug("refreshing config list");
    configs.clear();
    LPCWSTR path = GetPath("*");
    if (!path) {
        L::Error("Failed to get config dir path");
        return;
    }
    WIN32_FIND_DATAW ffd;
    HANDLE hFind = FindFirstFileW(path, &ffd);
    if (INVALID_HANDLE_VALUE == hFind) {
        L::Warning("No configs found");
        return;
    }
    do {
        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {//is a file file
            std::string filename = WCHARUTF8(ffd.cFileName);
            configs.push_back(filename.substr(0, filename.find_last_of(".")));//strip extension
        }
    } while (FindNextFileW(hFind, &ffd) != 0);
    L::Info("Found %d config(s)", configs.size());
}

bool Cfg::LoadCurrentMods() {
    if (NezuLoader::cheat->mod) { //supported cheat already loaded
        if (!mods[NezuLoader::cheat->mod->GetName()].is_null()) { //we have this mod in config
            json mod = mods[NezuLoader::cheat->mod->GetName()];
            for (CheatMod* cheat_mod : NezuLoader::cheat->mod->GetMods()) {
                bool val;
                read_val_full(mod, cheat_mod->name, val, false);
                if (cheat_mod->applied != val) //apply only if changed
                    cheat_mod->Apply(val);
            }
            L::Info("Mod config for %s loaded", NezuLoader::cheat->mod->GetName());
            return true;
        }
        L::Debug("No mod configuration found in current config for %s", NezuLoader::cheat->mod->GetName());
    }
    return false;
}

bool Cfg::SaveConfig(std::string name) {
    L::Debug("Saving config %s", name.c_str());
    LPCWSTR path = GetPath(name.c_str());
    if (!path) {
        L::Error("Failed to get config path");
        return false;
    }

    json root; {
        //mods
        if (NezuLoader::cheat->mod) {
            json mod;
            for (CheatMod* cheat_mod : NezuLoader::cheat->mod->GetMods()) {
                mod[cheat_mod->name] = cheat_mod->applied;
            }
            mods[NezuLoader::cheat->mod->GetName()] = mod;
        }
        save_val_direct(root, mods);

        auto& root_s = c;
        full_config(save);

    }

    std::ofstream out(path);
    if (!out.is_open()) {
        L::Error("Failed to open file %s", WCHARUTF8(path).c_str());
        return false;
    }
    try {
#ifdef _DEBUG
        out << root.dump(2);
#else
        out << root.dump();
#endif // _DEBUG
    }
    catch (nlohmann::detail::parse_error parse_error) {
        L::Error("Error while saving config %s (%s)", name.c_str(), parse_error.what());
        out.close();
        return false;
    }
    out.close();
    L::Info("Config %s saved", name.c_str());
    return true;
}

bool Cfg::LoadConfig(std::string name) {
    L::Debug("Loading config %s", name.c_str());
    LPCWSTR path = GetPath(name.c_str());
    if (!path) {
        L::Error("Failed to get config path");
        return false;
    }
    if (GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES) { //file dosn't exist
        L::Error("File %s dosn't exist", WCHARUTF8(path).c_str());
        return false;
    }
    std::ifstream in(path);
    if (!in.is_open()) {
        L::Error("Failed to open file %s", WCHARUTF8(path).c_str());
        return false;
    }
    json root;
    try {
        in >> root;
    }
    catch (nlohmann::detail::parse_error parse_error) {
        L::Error("Error while loading config %s (%s)", name.c_str(), parse_error.what());
        in.close();
        return false;
    }
    in.close();

    {//root

        auto& root_s = c;
        full_config(read);

        //mods
        read_val_full(root, "mods", mods, {});
        LoadCurrentMods();
    }

    L::Info("Config %s loaded", name.c_str());
    return true;
}

bool Cfg::DeleteConfig(std::string name) {
    LPCWSTR path = GetPath(name.c_str());
    if (!path) {
        L::Error("Failed to get config path");
        return false;
    }

    if (name == global.default_config) {
        global.default_config.clear();
        SaveGlobalConfig();
    }

    if (!DeleteFileW(path)) {
        L::Error("Failed to delete file %s 0x%X", WCHARUTF8(path).c_str(), GetLastError());
        return false;
    }
    return true;
}

bool Cfg::LoadGlobalConfig() {
    LPCWSTR path = GetPath("NezuLoader", true);
    if (!path) {
        L::Error("Failed to get global config path");
        return false;
    }
    if (GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES) //file dosn't exist
        return false;
    std::ifstream in(path);
    if (!in.is_open()) {
        L::Error("Failed to open file %s", WCHARUTF8(path).c_str());
        return false;
    }
    json root;
    in >> root;
    in.close();

    {//root
        read_val_t(root, global, recent_dlls);
        read_val_t(root, global, default_config);
    }
	return true;
}

bool Cfg::SaveGlobalConfig() {
    LPCWSTR path = GetPath("NezuLoader", true);
    if (!path) {
        L::Error("Failed to get global config path");
        return false;
    }

	json root; {
        save_val_t(root, global, recent_dlls);
        save_val_t(root, global, default_config);
	}

    std::ofstream out(path);
    if (!out.is_open()) {
        L::Error("Failed to open global config file %s", WCHARUTF8(path).c_str());
        return false;
    }
#ifdef _DEBUG
    out << root.dump(2);
#else
    out << root.dump();
#endif // _DEBUG
    out.close();
	return true;
}

void to_json(json& j, const ImColor& v) {
    j = json{ 
        {"r", v.Value.x }, 
        {"g", v.Value.y }, 
        {"b", v.Value.z }, 
        {"a", v.Value.w },
    };
}

void from_json(const json& j, ImColor& v) {
    j.at("r").get_to(v.Value.x);
    j.at("g").get_to(v.Value.y);
    j.at("b").get_to(v.Value.z);
    j.at("a").get_to(v.Value.w);
}