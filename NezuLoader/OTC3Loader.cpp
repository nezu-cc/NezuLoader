#include "pch.h"
#include "NezuLoader.h"
#include "Hooks.h"
#include "MinHook.h"
#include "OTC3ModPack.h"

LPVOID mod_base = NULL;
LPVOID otc_base = NULL;

//called after every string xor in the crack wrapper dll
typedef char* (__fastcall* f_sub_10001320)(void*, void*);
f_sub_10001320 o_sub_10001320;
char* __fastcall sub_10001320(void* ecx, void* edx) {
	char* ret = o_sub_10001320(ecx, edx);
	if (ret) {
		if (!strcmp(ret, "[+] fixed relocs\n") && otc_base) {
			L::Debug("[OTC3] mapped by crack, loading mods\n");
			NezuLoader::cheat->mod = new OTC3ModPack(otc_base);
			//Cfg::LoadCurrentMods();
			L::Info("[OTC3] mods loaded\n");
		}
		//printf("[OTC3] str: \"%s\"\n", ret);
	}
	return ret;
}

//memory allocation
typedef void* (__fastcall* f_sub_10005000)(void*, void*);
f_sub_10005000 o_sub_10005000;
void* __fastcall sub_10005000(void* ecx, void* edx) {
	void* ret = o_sub_10005000(ecx, edx);
	if (ret && !ecx) { //ecx is 0 on the call that allocates memory for the main image
		otc_base = ret;
		L::Debug("[OTC3] allocated at: %X\n", (DWORD)ret);
	}
	return ret;
}

BOOL NezuLoader::OTC3Loader(LPVOID mod, DWORD mod_size) {
	mod_base = mod;

	o_sub_10001320 = (f_sub_10001320)MH_HookInstant((void*)((DWORD)mod + 0x1320), sub_10001320);
	o_sub_10005000 = (f_sub_10005000)MH_HookInstant((void*)((DWORD)mod + 0x5000), sub_10005000);
	L::Info("[OTC3] hooks placed");

	return TRUE;
}