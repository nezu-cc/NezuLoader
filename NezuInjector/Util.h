#pragma once

#ifdef UNICODE
#define TCHARUTF8(t) (std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>::wstring_convert().to_bytes((t)))
#else
#define TCHARUTF8(t) (std::string(t))
#endif //UNICODE

namespace U {

	std::string DllFilePicker(HWND owner);
	BOOL Set_DontCallForThreads(HANDLE hProc, const WCHAR* cMod, bool set);
	DWORD FindProcess(LPCTSTR name);
	DWORD KillAll(LPCTSTR name);
	BOOL IsProcessOpen(LPCTSTR name);
	BOOL FindRemoteDll(HANDLE hProc, LPCTSTR name, HMODULE* address);
}
