#pragma once
#include "NezuVacStatus.h"

extern NezuVac::NeuzStatus vac_status;

DWORD __stdcall MainThread(HMODULE hModule);

enum vac_result_t {
	VAC_SUCCESS = 1,
	VAC_OTHER_SUCCESS = 2,
};

struct mapped_module {
	BYTE pad04[4];
	void* m_pModuleBase;
	PIMAGE_NT_HEADERS m_pNTHeaders;
};

struct vac_buffer {
	DWORD m_unCRC32;
	BYTE pad04[4];
	mapped_module* m_pMappedModule;
	void* m_pRunFunc; // entrypoint
	int m_nLastResult;
	int m_nModuleSize;
	void* m_pRawModule;
};

bool __stdcall Hooked_loading(vac_buffer* h_mod, char injection_flags);
int __fastcall Hooked_calling(void* ecx, void* edx, DWORD crc_hash, char injection_mode, int unused_maybe, int runfunc_param1, 
	int runfunc_param2, int runfunc_param3, int* runfunc_param4, int* region_or_size_check_maybe, int* module_status);
DWORD FindPattern(std::string moduleName, std::string pattern);

typedef bool(__stdcall* f_loading)(vac_buffer*, char);
typedef int(__fastcall* f_calling)(void*, void*, DWORD, char, int, int, int, int, int*, int*, int*);
