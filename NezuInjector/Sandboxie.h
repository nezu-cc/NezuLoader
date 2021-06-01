#pragma once

typedef LONG(__stdcall* f_SbieApi_GetVersion)(WCHAR* out_version);
typedef LONG(__stdcall* f_SbieApi_EnumBoxes)(LONG index, WCHAR* box_name);
typedef LONG(__stdcall* f_SbieApi_QueryProcessEx)(HANDLE ProcessId, ULONG image_name_len_in_wchars, WCHAR* out_box_name_wchar34, WCHAR* out_image_name_wcharXXX, WCHAR* out_sid_wchar96, ULONG* out_session_id);
typedef LONG(__stdcall* f_SbieApi_EnumProcessEx)(const WCHAR* box_name, BOOLEAN all_sessions, ULONG which_session, ULONG* boxed_pids);
typedef LONG(__stdcall* f_SbieApi_EnumProcessEx_SBP_5_48_5)(const WCHAR* box_name, BOOLEAN all_sessions, ULONG which_session, ULONG* boxed_pids, ULONG* boxed_count);
typedef BOOL(__stdcall* f_SbieDll_RunSandboxed)(const WCHAR* box_name, const WCHAR* cmd, const WCHAR* dir, ULONG creation_flags, STARTUPINFO* si, PROCESS_INFORMATION* pi);
typedef LONG(__stdcall* f_SbieApi_GetHomePath)(WCHAR* NtPath, ULONG NtPathMaxLen, WCHAR* DosPath, ULONG DosPathMaxLen);

namespace Sandboxie {

	BOOL Init();
	BOOL IsLoaded();
	LPCSTR GetVersion();
	BOOL IsVersionAtLeast(const int(&version)[3]);
	std::vector<std::string>& GetBoxes(bool update = false);
	LPCSTR GetCurrentSandboxName();
	BOOL IsInsideSandbox();
	std::vector<std::pair<DWORD, std::basic_string<TCHAR>>> GetAllProcesses(std::string box);
	BOOL RunSandboxed(std::string box, LPCTSTR commandLine, LPSTARTUPINFOW startupInfo, LPPROCESS_INFORMATION processInformation);
	BOOL IsProcessSandboxed(DWORD pid);

};

