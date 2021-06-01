#include "pch.h"
#include "Sandboxie.h"

static HMODULE hSbieDll = NULL;
static std::string version_string = "";
static std::vector<std::string> boxes;
static bool inSandbox = false;
static std::string cur_sandbox_name = "";

/// <summary>
/// Loads SbieDll.dll
/// </summary>
/// <returns>TRUE on success FALSE otherwise</returns>
BOOL Sandboxie::Init() {

	if (hSbieDll)
		return TRUE;

	//if we are sandboxed this dll will already be loaded. We can't controll sandboxie from inside of a sandbox
	hSbieDll = GetModuleHandle(TEXT("SbieDll.dll"));
	if (hSbieDll) {
		inSandbox = true;
		return TRUE;
	}

	TCHAR path[MAX_PATH];
	if (!U::GetServiceBinaryPath(TEXT("SbieSvc"), path, MAX_PATH))
		return FALSE;

	PathRemoveFileSpec(path);

	BOOL isWow64 = FALSE;
	if (!IsWow64Process(GetCurrentProcess(), &isWow64))
		return FALSE;

	LPCTSTR SbieDll = isWow64 ? TEXT("32\\SbieDll.dll") : TEXT("SbieDll.dll");
	if (!PathCombine(path, path, SbieDll))
		return FALSE;

	hSbieDll = LoadLibrary(path);
	if (!hSbieDll)
		return FALSE;

	if (!GetVersion()) {
		FreeLibrary(hSbieDll);
		hSbieDll = NULL;
		return FALSE;
	}

	return TRUE;
}

/// <summary>
/// is SbieDll.dll loaded
/// </summary>
/// <returns>TRUE if SbieDll.dll is loaded FALSE otherwise</returns>
BOOL Sandboxie::IsLoaded() {
	return hSbieDll != NULL;
}

/// <summary>
/// Gets sandboxie version
/// </summary>
/// <returns>version string on success NULL otherwise</returns>
LPCSTR Sandboxie::GetVersion() {
	if (!hSbieDll)
		return NULL;

	if (!version_string.empty()) //if string length is > 0
		return version_string.c_str();

	static f_SbieApi_GetVersion SbieApi_GetVersion = (f_SbieApi_GetVersion)GetProcAddress(hSbieDll, "SbieApi_GetVersion");
	if (!SbieApi_GetVersion)
		return NULL;
	
	std::wstring str;
	str.resize(16);
	if (!NT_SUCCESS(SbieApi_GetVersion(&str[0])))
		return NULL;

	version_string = WCHARUTF8(str);

	return version_string.c_str();
}

/// <summary>
/// checks if the current version is equal or newer than the specified version
/// </summary>
/// <param name="version">version as 3 ints</param>
BOOL Sandboxie::IsVersionAtLeast(const int (&version)[3]) {
	LPCSTR ver = GetVersion();
	if (!ver)
		return FALSE;
	std::istringstream parser(ver);
	int cur_version[3];
	parser >> cur_version[0];
	parser.get();// dot
	parser >> cur_version[1];
	parser.get();// dot
	parser >> cur_version[2];

	if (cur_version[0] < version[0])
		return FALSE;
	if (cur_version[0] > version[0])
		return TRUE;
	if (cur_version[1] < version[1])
		return FALSE;
	if (cur_version[1] > version[1])
		return TRUE;
	if (cur_version[2] < version[2])
		return FALSE;
	return TRUE;
}

/// <summary>
/// Gets all box names
/// </summary>
/// <param name="force_update">force refresh</param>
/// <returns>vector of box names</returns>
std::vector<std::string>& Sandboxie::GetBoxes(bool update) {

	if(!update || !hSbieDll)
		return boxes;

	boxes.clear();

	static f_SbieApi_EnumBoxes SbieApi_EnumBoxes = (f_SbieApi_EnumBoxes)GetProcAddress(hSbieDll, "SbieApi_EnumBoxes");
	if (!SbieApi_EnumBoxes)
		return boxes;

	int next_id = -1;
	do {
		std::wstring boxName;
		boxName.resize(34);
		next_id = SbieApi_EnumBoxes(next_id, &boxName[0]);
		if (next_id == -1)
			break;
		boxName.resize(wcslen(boxName.c_str()));
		boxes.push_back(WCHARUTF8(boxName));
	} while (true);
	return boxes;
}

/// <summary>
/// If inside a sandbox, gets the name of the box it's running in
/// </summary>
/// <returns>name of the sandbox</returns>
LPCSTR Sandboxie::GetCurrentSandboxName() {
	if (!hSbieDll)
		return NULL;

	if (!inSandbox)
		return NULL;

	if (!cur_sandbox_name.empty()) //if string length is > 0
		return cur_sandbox_name.c_str();

	static f_SbieApi_QueryProcessEx SbieApi_QueryProcessEx = (f_SbieApi_QueryProcessEx)GetProcAddress(hSbieDll, "SbieApi_QueryProcessEx");
	if (!SbieApi_QueryProcessEx)
		return NULL;

	std::wstring str;
	str.resize(34);
	if (SbieApi_QueryProcessEx((HANDLE)GetCurrentProcessId(), 0, &str[0], NULL, NULL, NULL) != 0)
		return NULL;

	cur_sandbox_name = WCHARUTF8(str);

	return cur_sandbox_name.c_str();
}

/// <summary>
/// checks if we are running inside a sandbox
/// </summary>
/// <returns>TRUE if we are</returns>
BOOL Sandboxie::IsInsideSandbox() {
	return hSbieDll && inSandbox;
}

/// <summary>
/// Gets all the processes running in a given sandbox
/// </summary>
/// <param name="box">sandbox name</param>
/// <returns>list of processes as <PID, Process Names> pairs</returns>
std::vector<std::pair<DWORD, std::basic_string<TCHAR>>> Sandboxie::GetAllProcesses(std::string box) {
	std::vector<std::pair<DWORD, std::basic_string<TCHAR>>> ret;

	if (!hSbieDll)
		return ret;

	if (box.length() == 0 && !inSandbox)
		return ret;

	if (inSandbox)
		box = GetCurrentSandboxName();

	static f_SbieApi_QueryProcessEx SbieApi_QueryProcessEx = (f_SbieApi_QueryProcessEx)GetProcAddress(hSbieDll, "SbieApi_QueryProcessEx");
	if (!SbieApi_QueryProcessEx)
		return ret;

	static FARPROC SbieApi_EnumProcessEx = GetProcAddress(hSbieDll, "SbieApi_EnumProcessEx");
	if (!SbieApi_EnumProcessEx)
		return ret;

	ULONG pids[512];

	if (IsVersionAtLeast({ 5, 48, 5 })) //https://github.com/sandboxie-plus/Sandboxie/blob/96b7d4625ced1b08a6a0a62e65b8c90e95d9581a/CHANGELOG.md#changed
		((f_SbieApi_EnumProcessEx_SBP_5_48_5)SbieApi_EnumProcessEx)(UTF8WCHAR(box).c_str(), FALSE, -1, pids, NULL);
	else
		((f_SbieApi_EnumProcessEx)SbieApi_EnumProcessEx)(UTF8WCHAR(box).c_str(), FALSE, -1, pids);

	WCHAR image_name[MAX_PATH];

	for (ULONG i = 1; i <= pids[0]; i++) {
		ULONG pid = pids[i];
		if (SbieApi_QueryProcessEx((HANDLE)pid, MAX_PATH, NULL, image_name, NULL, NULL) != 0)
			continue;
		ret.push_back(std::make_pair(pid, TCHARWCHAR(image_name)));
	}

	return ret;
}

/// <summary>
/// starts a process inside a sandbox
/// </summary>
/// <param name="box">name of the sandbox</param>
/// <param name="commandLine">commandline to execute</param>
/// <param name="startupInfo">STARTUPINFO</param>
/// <param name="processInformation">PROCESS_INFORMATION</param>
/// <returns>TRUE on success FALSE otherwise</returns>
BOOL Sandboxie::RunSandboxed(std::string box, LPCTSTR commandLine, LPSTARTUPINFOW startupInfo, LPPROCESS_INFORMATION processInformation) {
	if (!hSbieDll)
		return FALSE;

	static f_SbieDll_RunSandboxed SbieDll_RunSandboxed = (f_SbieDll_RunSandboxed)GetProcAddress(hSbieDll, "SbieDll_RunSandboxed");
	if (!SbieDll_RunSandboxed)
		return FALSE;

	static f_SbieApi_GetHomePath SbieApi_GetHomePath = (f_SbieApi_GetHomePath)GetProcAddress(hSbieDll, "SbieApi_GetHomePath");
	if (!SbieApi_GetHomePath)
		return FALSE;

	WCHAR home[MAX_PATH];
	SbieApi_GetHomePath(NULL, 0, home, MAX_PATH);

	return SbieDll_RunSandboxed(UTF8WCHAR(box).c_str(), TCHARWCHAR(commandLine).c_str(), home, 0, startupInfo, processInformation);
}

/// <summary>
/// checks if a given process is running inside anny sandbox
/// </summary>
/// <param name="pid"></param>
/// <returns></returns>
BOOL Sandboxie::IsProcessSandboxed(DWORD pid) {
	if (!hSbieDll)
		return FALSE;

	static f_SbieApi_QueryProcessEx SbieApi_QueryProcessEx = (f_SbieApi_QueryProcessEx)GetProcAddress(hSbieDll, "SbieApi_QueryProcessEx");
	if (!SbieApi_QueryProcessEx)
		return FALSE;

	return SbieApi_QueryProcessEx((HANDLE)pid, 0, NULL, NULL, NULL, NULL) == 0;
}