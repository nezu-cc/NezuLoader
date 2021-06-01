#pragma once

extern bool load_lock;
extern std::condition_variable load_lock_cv;
extern std::mutex load_lock_m;

class Cheat;
namespace NezuLoader {
	extern Cheat* cheat;

	void Load(HMODULE hModule);
	void Unload();
	BOOL MMCallback(LPVOID mod);

	//loaders
	BOOL CheatLoader(LPVOID mod, DWORD mod_size);
	//cheat specific loaders
	BOOL OTC3Loader(LPVOID mod, DWORD mod_size);

};

extern "C" DWORD WINAPI Inject(PCWCHAR path);