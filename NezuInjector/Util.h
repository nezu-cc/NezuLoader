#pragma once

#define WCHARUTF8(t) (std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>::wstring_convert().to_bytes((t)))
#define UTF8WCHAR(t) (std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>::wstring_convert().from_bytes((t)))

#ifdef UNICODE
#define TCHARUTF8(t) WCHARUTF8(t)
#define UTF8TCHAR(t) UTF8WCHAR(t)
#define TCHARWCHAR(t) (std::wstring(t))
#else
#define TCHARUTF8(t) (std::string(t))
#define UTF8TCHAR(t) (std::string(t))
#define TCHARWCHAR(t) UTF8WCHAR(t)
#endif //UNICODE

namespace U {

	std::string DllFilePicker(HWND owner);
	BOOL Set_DontCallForThreads(HANDLE hProc, const WCHAR* cMod, bool set);
	BOOL FindRemoteDll(HANDLE hProc, LPCTSTR name, HMODULE* address);
	DWORD GetExportFuncRVA(const PBYTE pAssembly, LPCSTR lpProcName);
	BOOL GetServiceBinaryPath(LPCTSTR cName, LPTSTR cBinPath, DWORD buffSize);
	BOOL SetDebugPrivilege(BOOL enable);

}
