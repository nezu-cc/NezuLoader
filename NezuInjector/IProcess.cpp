#include "pch.h"
#include "IProcess.h"

StProcess::StProcess() { }

DWORD StProcess::FindProcess(LPCTSTR name) {
	PROCESSENTRY32 PE32{ 0 };
	PE32.dwSize = sizeof(PE32);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
		return 0;

	DWORD PID = 0;
	BOOL bRet = Process32First(hSnap, &PE32);
	while (bRet) {
		if (!_tcsicmp(name, PE32.szExeFile)) {
			if (!Sandboxie::IsProcessSandboxed(PE32.th32ProcessID)) {
				PID = PE32.th32ProcessID;
				break;
			}
		}
		bRet = Process32Next(hSnap, &PE32);
	}

	CloseHandle(hSnap);

	return PID;
}

BOOL StProcess::IsProcessOpen(LPCTSTR name) {
	PROCESSENTRY32 PE32{ 0 };
	PE32.dwSize = sizeof(PE32);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
		return (DWORD)-1;

	BOOL bRet = Process32First(hSnap, &PE32);
	while (bRet) {
		if (!_tcsicmp(name, PE32.szExeFile)) {
			if (!Sandboxie::IsProcessSandboxed(PE32.th32ProcessID)) {
				CloseHandle(hSnap);
				return true;
			}
		}
		bRet = Process32Next(hSnap, &PE32);
	}

	CloseHandle(hSnap);

	return false;
}

DWORD StProcess::KillAll(LPCTSTR name) {
	DWORD failed = 0;

	L::Debug("Killing all %s proceses", TCHARUTF8(name).c_str());

	PROCESSENTRY32 PE32{ 0 };
	PE32.dwSize = sizeof(PE32);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE) {
		L::Error("Failed to kill all %s (CreateToolhelp32Snapshot) 0x%X", TCHARUTF8(name).c_str(), GetLastError());
		return (DWORD)-1;
	}

	BOOL bRet = Process32First(hSnap, &PE32);
	while (bRet) {
		if (!_tcsicmp(name, PE32.szExeFile)) {
			if (!Sandboxie::IsProcessSandboxed(PE32.th32ProcessID)) {
				HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, PE32.th32ProcessID);
				if (hProc) {
					if (!TerminateProcess(hProc, 1)) {
						L::Error("Failed to kill %s(%d) (TerminateProcess) 0x%X", TCHARUTF8(name).c_str(), PE32.th32ProcessID, GetLastError());
						failed++;
					}
					else L::Info("Killed process %s(%d)", TCHARUTF8(name).c_str(), PE32.th32ProcessID);
					CloseHandle(hProc);
				}
				else {
					L::Error("Failed to kill %s(%d) (OpenProcess) 0x%X", TCHARUTF8(name).c_str(), PE32.th32ProcessID, GetLastError());
					failed++;
				}
			}
		}
		bRet = Process32Next(hSnap, &PE32);
	}

	CloseHandle(hSnap);

	return failed;
}

BOOL StProcess::SpawnProcess(LPTSTR commandLine, LPSTARTUPINFOW startupInfo, LPPROCESS_INFORMATION processInformation) {
	return CreateProcess(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, startupInfo, processInformation);
}

BOOL StProcess::Execute(LPCTSTR command) {
	//https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecutea#return-value
	return (DWORD)ShellExecute(NULL, NULL, command, NULL, NULL, SW_SHOW) > 32;
}

BOOL StProcess::IsProcessInitialized(HANDLE hProc) {
	return TRUE;
}

DWORD SbProcess::FindProcess(LPCTSTR name) {
	if (!Sandboxie::IsLoaded())
		return 0;
	auto processes = Sandboxie::GetAllProcesses(sandbox_name);
	for (auto& process : processes) {
		if (!_tcsicmp(name, process.second.c_str())) {
			return process.first;
		}
	}
	return 0;
}

BOOL SbProcess::IsProcessOpen(LPCTSTR name) {
	return FindProcess(name) > 0;
}

DWORD SbProcess::KillAll(LPCTSTR name) {
	if (!Sandboxie::IsLoaded())
		return (DWORD)-1;
	DWORD failed = 0;
	auto processes = Sandboxie::GetAllProcesses(sandbox_name);
	for (auto& process : processes) {
		if (!_tcsicmp(name, process.second.c_str())) {
			HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, process.first);
			if (hProc) {
				if (!TerminateProcess(hProc, 1)) {
					L::Error("Failed to kill %s(%d) (TerminateProcess) 0x%X", TCHARUTF8(name).c_str(), process.first, GetLastError());
					failed++;
				}
				else L::Info("Killed process %s(%d)", TCHARUTF8(name).c_str(), process.first);
				CloseHandle(hProc);
			}
			else {
				L::Error("Failed to kill %s(%d) (OpenProcess) 0x%X", TCHARUTF8(name).c_str(), process.first, GetLastError());
				failed++;
			}
		}
	}
	return failed;
}

BOOL SbProcess::SpawnProcess(LPTSTR commandLine, LPSTARTUPINFOW startupInfo, LPPROCESS_INFORMATION processInformation) {
	if (!Sandboxie::IsLoaded())
		return FALSE;

	if (Sandboxie::IsInsideSandbox()) {//we are inside a sandbox, just create a normal process
		return CreateProcess(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, startupInfo, processInformation) != 0;
	}

	return Sandboxie::RunSandboxed(sandbox_name, commandLine, startupInfo, processInformation);
}

BOOL SbProcess::Execute(LPCTSTR command) {
	if (!Sandboxie::IsLoaded())
		return FALSE;

	if (Sandboxie::IsInsideSandbox()) {//we are inside a sandbox, just create a normal process
		return (DWORD)ShellExecute(NULL, NULL, command, NULL, NULL, SW_SHOW) > 32;
	}

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi = { 0 };
	BOOL ret = Sandboxie::RunSandboxed(sandbox_name, TCHARWCHAR(std::basic_string<TCHAR>(_T("Start.exe ")) + command).c_str(), &si, &pi);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return ret;
}

BOOL SbProcess::IsProcessInitialized(HANDLE hProc) {
	L::SuppressAllMessages(true);
	HMODULE hMod = NULL;
	BOOL ret = U::FindRemoteDll(hProc, _T("SbieDll.dll"), &hMod) && hMod;
	L::SuppressAllMessages(false);
	return ret;
}
