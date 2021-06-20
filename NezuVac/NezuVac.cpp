#include "pch.h"
#include "NezuVac.h"
#include "MinHook.h"

//#define DEBUG_CONSOLE


NezuVac::f_loading oHooked_loading = NULL;
NezuVac::f_calling oHooked_calling = NULL;

#pragma section(NEZUVACSTATUS_SECTION_NAME_1, read, write) 
__declspec(allocate(NEZUVACSTATUS_SECTION_NAME_1)) NezuVac::NeuzStatus vac_status;

#if 0 //ignore this
DWORD debug_1 = sizeof(NezuVac::NeuzStatus);
DWORD debug_2 = sizeof(vac_status);
#endif

DWORD FindPattern(std::string moduleName, std::string pattern);

DWORD WINAPI NezuVac::MainThread(HMODULE hModule) {

#ifdef DEBUG_CONSOLE
    AllocConsole();
    (void)freopen("CONOUT$", "w", stdout);
    (void)freopen("CONIN$", "r", stdin);
#endif //DEBUG_CONSOLE

	while (!GetModuleHandle(_T("tier0_s.dll")) || !GetModuleHandle(_T("steamservice.dll")))
        Sleep(50);

	if (MH_Initialize() != MH_OK) {
		vac_status.error = Error::MHInit;
        return 1;
	}

	DWORD loading = FindPattern("steamservice.dll", "55 8B EC 83 EC 28 53 56 8B 75 08 8B");
	if (!loading) {
		MH_Uninitialize();
		vac_status.error = Error::FindPattern_loading;
		return 1;
	}

	MH_STATUS status = MH_CreateHook((LPVOID)loading, Hooked_loading, (LPVOID*)&oHooked_loading);
	if (status != MH_OK) {
		MH_Uninitialize();
		vac_status.error = Error::CreateHook_loading;
		return 1;
	}

	DWORD calling = FindPattern("steamservice.dll", "55 8B EC 6A FF 68 ? ? ? ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 64 89 25 ? ? ? ? 83 EC 6C 53 56");
	if (!calling) {
		vac_status.error = Error::FindPattern_calling;
		MH_Uninitialize();
		return 1;
	}

	status = MH_CreateHook((LPVOID)calling, Hooked_calling, (LPVOID*)&oHooked_calling);
	if (status != MH_OK) {
		MH_Uninitialize();
		vac_status.error = Error::CreateHook_calling;
		return 1;
	}

	status = MH_EnableHook(MH_ALL_HOOKS);
	if (status != MH_OK) {
		MH_DisableHook(MH_ALL_HOOKS);
		MH_Uninitialize();
		vac_status.error = Error::ApplyHook;
		return 1;
	}

	vac_status.error = Error::Success;
	vac_status.initialized = TRUE;

#ifdef DEBUG_CONSOLE
    printf("[nezu] loaded\n");
#endif
    return 0;
}

bool __stdcall NezuVac::Hooked_loading(vac_buffer* h_mod, char injection_flags) {

	DWORD header_crc = [](LPVOID mod) -> DWORD {
		PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)mod;
		if (!dos)
			return 0;

		PIMAGE_NT_HEADERS32 nt = (PIMAGE_NT_HEADERS32)((DWORD)dos + dos->e_lfanew);
		if (nt->Signature != 0x4550) //'PE'
			return 0;

		DWORD nt_size = offsetof(IMAGE_NT_HEADERS, OptionalHeader) + nt->FileHeader.SizeOfOptionalHeader + (nt->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER));

		return Crc32::hash((LPCVOID)nt, nt_size, 0xFFFFFFFF);

	} (h_mod->m_pRawModule);

	bool ret = oHooked_loading(h_mod, injection_flags);

	if (header_crc) {
#ifdef DEBUG_CONSOLE
		printf("[nezu] loading VAC module [0x%.8X] {0x%.8X}.\n", header_crc, h_mod->m_unCRC32);
#endif
		if (header_crc == 0xCC29049A || header_crc == 0x2B8DD987) {
#ifdef DEBUG_CONSOLE
			printf("[nezu] [0x%.8X] is whitelisted, loading as normal\n", header_crc);
#endif
			return ret;
		}
	}

	if (h_mod->m_pRunFunc) {
		h_mod->m_pRunFunc = NULL;
#ifdef DEBUG_CONSOLE
		printf("[nezu] m_pRunFunc nulled {0x%.8X}\n", h_mod->m_unCRC32);
#endif
		vac_status.num_blocked_modules++;
	}

	return ret;

}

int __fastcall NezuVac::Hooked_calling(void* ecx, void* edx, DWORD crc_hash, char injection_mode, int unused_maybe, int runfunc_param1,
	int runfunc_param2, int runfunc_param3, int* runfunc_param4, int* region_or_size_check_maybe, int* module_status) {

	oHooked_calling(ecx, edx, crc_hash, injection_mode, unused_maybe, runfunc_param1, runfunc_param2, runfunc_param3, runfunc_param4, region_or_size_check_maybe, module_status);

	if (*module_status != VAC_SUCCESS && *module_status != VAC_OTHER_SUCCESS) {
		*module_status = VAC_SUCCESS;
		//printf("[nezu] patched return from uid {0x%.8X}.\n", crc_hash);
	}

	return 1;

}

#define INRANGE(x,a,b)    (x >= a && x <= b)
#define getBits( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))

DWORD FindPattern(std::string moduleName, std::string pattern) {
	const char* pat = pattern.c_str();
	DWORD firstMatch = 0;
	DWORD rangeStart = (DWORD)GetModuleHandleA(moduleName.c_str());
	if (!rangeStart)
		return NULL;
	MODULEINFO miModInfo;
	GetModuleInformation(GetCurrentProcess(), (HMODULE)rangeStart, &miModInfo, sizeof(MODULEINFO));
	DWORD rangeEnd = rangeStart + miModInfo.SizeOfImage;
	for (DWORD pCur = rangeStart; pCur < rangeEnd; pCur++) {
		if (!*pat) return firstMatch;
		if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == getByte(pat)) {
			if (!firstMatch) firstMatch = pCur;
			if (!pat[2]) return firstMatch;
			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?') pat += 3;
			else pat += 2;
		}
		else {
			pat = pattern.c_str();
			firstMatch = 0;
		}
	}
	return NULL;
}