#pragma once

using f_LoadLibraryA = HINSTANCE(WINAPI*)(const char* lpLibFilename);
using f_GetProcAddress = UINT_PTR(WINAPI*)(HINSTANCE hModule, const char* lpProcName);
using f_MessageBoxA = int(WINAPI*)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
using f_DLL_ENTRY_POINT = BOOL(WINAPI*)(void* hDll, DWORD dwReason, void* pReserved);
typedef BOOL(*f_MMapCallback)(void* hInst);
typedef BOOL(__fastcall* f_RtlInsertInvertedFunctionTable)(void* hDll, DWORD SizeOfImage);

struct MANUAL_MAPPING_DATA {
	BYTE* base;
	f_LoadLibraryA pLoadLibraryA;
	f_GetProcAddress pGetProcAddress;
	f_RtlInsertInvertedFunctionTable pRtlInsertInvertedFunctionTable;
	HINSTANCE hMod;
};

#ifndef NT_FAIL
#define NT_FAIL(status) (status < 0)
#endif

namespace Injector {
	bool ManualMap(HANDLE hProc, const WCHAR* filename);
	bool ManualMap(HANDLE hProc, const BYTE* pSrcData, HMODULE* hRemoteModule);
	bool CoppyToWindowsDir(const WCHAR* filename, WCHAR* new_filename);
	bool LoadLib(HANDLE hProc, const WCHAR* filename, bool bypass_secure = false);
	std::string& GetLastErrorString();
	void SetLastErrorString(LPCSTR fmt, ...);
#ifdef MM_INTERNAL
	void SetMMapCallback(f_MMapCallback callback);
#endif //MM_INTERNAL

	extern f_RtlInsertInvertedFunctionTable RtlInsertInvertedFunctionTable;
}

