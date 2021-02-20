#pragma once

using f_LoadLibraryA = HINSTANCE(WINAPI*)(const char* lpLibFilename);
using f_GetProcAddress = UINT_PTR(WINAPI*)(HINSTANCE hModule, const char* lpProcName);
using f_MessageBoxA = int(WINAPI*)(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
using f_DLL_ENTRY_POINT = BOOL(WINAPI*)(void* hDll, DWORD dwReason, void* pReserved);

struct MANUAL_MAPPING_DATA
{
	f_LoadLibraryA		pLoadLibraryA;
	f_GetProcAddress	pGetProcAddress;
	HINSTANCE			hMod;
};

namespace Injector {
	bool ManualMap(HANDLE hProc, const WCHAR* filename);
	bool ManualMap(HANDLE hProc, const BYTE* data);
	bool CoppyToWindowsDir(const WCHAR* filename, WCHAR* new_filename);
	bool LoadLib(HANDLE hProc, const WCHAR* filename, bool bypass_secure = false);
	std::string& GetLastErrorString();
	void SetLastErrorString(LPCSTR fmt, ...);
}

