#pragma once
#include "NezuVacStatus.h"

namespace NezuVac {

	DWORD __stdcall MainThread(HMODULE hModule);

	NeuzStatus* GetVacStatus();

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
	int __fastcall Hooked_calling(void* ecx, void* edx, DWORD crc_hash, char injection_mode, int unused1, int id, int param1, int unused2, int param2, int param3, int* param4, int* size_check);

	typedef bool(__stdcall* f_loading)(vac_buffer*, char);
	typedef int(__fastcall* f_calling)(void*, void*, DWORD, char, int, int, int, int, int, int, int*, int*);
									   

}



