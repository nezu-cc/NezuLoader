#include "pch.h"
#include "Loader.h"
#include "Injector.h"
#include "resource.h"
#include "NezuVacInterface.h"

DWORD WINAPI LoaderThread(LPVOID lpThreadParameter) {

	NezuInjectorSettings* settings = (NezuInjectorSettings*)lpThreadParameter;
	HANDLE hSteamProc = NULL;

	BOOL nezu_injector_dir_needed = settings->VacBypass || settings->injectionMode == InjectionMode::Dll;
	WCHAR nezu_injector_dir[MAX_PATH];

	PWSTR win_dir = NULL;
	if (FAILED(SHGetKnownFolderPath(FOLDERID_Windows, 0, NULL, &win_dir))) {
		CoTaskMemFree(win_dir);
		if (nezu_injector_dir_needed) {
			L::Error("Failed to load (SHGetKnownFolderPath) 0x%X", GetLastError());
			return FALSE;
		}
	}
	else {//SHGetKnownFolderPath success
		if (!PathCombineW(nezu_injector_dir, win_dir, L"NezuInjectorTmp")) {
			CoTaskMemFree(win_dir);
			if (nezu_injector_dir_needed) {
				L::Error("Failed to load (PathCombineW)(1) 0x%X", GetLastError());
				return FALSE;
			}
		}
		else {//PathCombineW success
			CoTaskMemFree(win_dir);
			if (!CreateDirectoryW(nezu_injector_dir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) { //It's ok if it exists
				if (nezu_injector_dir_needed) {
					L::Error("Failed to load (CreateDirectoryW)(1) 0x%X", GetLastError());
					return FALSE;
				}
			}
			else {//CreateDirectoryW success
				//next past is silent because we don't care if it fails
				WCHAR nezu_injector_dir_wildcard[MAX_PATH];
				wcscpy_s(nezu_injector_dir_wildcard, nezu_injector_dir);
				wcscat_s(nezu_injector_dir_wildcard, L"\\*");

				WIN32_FIND_DATA ffd;
				HANDLE hFind = FindFirstFileW(nezu_injector_dir_wildcard, &ffd);
				if (hFind != INVALID_HANDLE_VALUE) {
					do {
						if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
							WCHAR file_to_delete[MAX_PATH];
							if (PathCombineW(file_to_delete, nezu_injector_dir, ffd.cFileName)) {
								DeleteFileW(file_to_delete);
							}
						}
					} while (FindNextFileW(hFind, &ffd) != 0);
				}
			}
		}
	}

	if (settings->RestartSteam || proc->FindProcess(_T("steam.exe")) == 0) { //if steam isn't running (re)start it

		//make sure steam is dead
		do  {
			proc->KillAll(_T("csgo.exe"));
			proc->KillAll(_T("steam.exe"));
			proc->KillAll(_T("steamservice.exe"));
			proc->KillAll(_T("steamwebhelper.exe"));

			Sleep(100);
		} while (proc->IsProcessOpen(_T("steam.exe")) || proc->IsProcessOpen(_T("steamservice.exe")) || proc->IsProcessOpen(_T("steamwebhelper.exe")));

		L::Info("All Steam proceses killed");

		//find steam executable from registery
		HKEY hKey = NULL;
		LSTATUS status = RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Valve\\Steam"), 0, KEY_QUERY_VALUE, &hKey);
		if (status != ERROR_SUCCESS) {
			RegCloseKey(hKey);
			L::Error("Failed to find Steam install location (RegOpenKeyEx) 0x%X", status);
			return FALSE;
		}

		TCHAR steam_path[MAX_PATH];
		DWORD steam_path_size = sizeof(steam_path);
		status = RegQueryValueEx(hKey, _T("SteamExe"), NULL, NULL, (LPBYTE)steam_path, &steam_path_size);
		if (status != ERROR_SUCCESS) {
			RegCloseKey(hKey);
			L::Error("Failed to find Steam install location (RegQueryValueEx) 0x%X", status);
			return FALSE;
		}
		RegCloseKey(hKey);

		L::Debug("Found Steam in %s", TCHARUTF8(steam_path).c_str());

		//start steam
		auto commandlaine = _T("\"") + std::basic_string<TCHAR>(steam_path) + _T("\"");
		if (settings->injectionMode != InjectionMode::NezuVacOnly)
			commandlaine += _T("-silent -applaunch 730 ");//only start cs if we are injecting into it
		if (settings->UseCustomCredentials)
			commandlaine += _T("-login ") + UTF8TCHAR(settings->Creds.first) + _T(" ") + UTF8TCHAR(settings->Creds.second) + _T(" ");
		commandlaine += UTF8TCHAR(settings->CustomSteamArgs);

		//CreateProcessW can change the buffer so we need to coppy it
		DWORD commandlaine_buf_len = (commandlaine.length() + 1) * sizeof(TCHAR);
		LPTSTR commandlaine_buf = new TCHAR[commandlaine_buf_len];
		_tcsncpy_s(commandlaine_buf, commandlaine_buf_len, commandlaine.c_str(), commandlaine.length());

		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);

		PROCESS_INFORMATION pi = { 0 };
		if (!proc->SpawnProcess(commandlaine_buf, &si, &pi)) {
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			delete[] commandlaine_buf;
			L::Error("Failed to start Steam (CreateProcess) 0x%X", GetLastError());
			return FALSE;
		}
		delete[] commandlaine_buf;
		CloseHandle(pi.hThread);

		hSteamProc = pi.hProcess;

		while (!proc->IsProcessInitialized(hSteamProc))
			Sleep(50);

		L::Info("Steam started");

	}

	if (settings->VacBypass) {

		if (!hSteamProc) { //if steam was not restarted just open it normaly

			DWORD steam_pid = proc->FindProcess(_T("steam.exe"));
			if (!steam_pid) {
				L::Error("Failed to find Steam process (FindProcess) 0x%X", GetLastError());
				return FALSE;
			}

			hSteamProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, steam_pid);
			if (!hSteamProc) {
				L::Error("Failed to open Steam process (OpenProcess) 0x%X", GetLastError());
				return FALSE;
			}

		}

		while (proc->IsProcessOpen(_T("steamservice.exe"))) {
			Sleep(100);
			proc->KillAll(_T("steamservice.exe")); // just making sure
		}

		L::Debug("Injecting NezuVac into Steam");

		HRSRC nezu_vac_rc = FindResource(NULL, MAKEINTRESOURCE(IDR_RCDATA1), RT_RCDATA);
		if (!nezu_vac_rc) {
			CloseHandle(hSteamProc);
			L::Error("Failed to load NezuVac (FindResource) 0x%X", GetLastError());
			return FALSE;
		}

		DWORD nezu_vac_size = SizeofResource(NULL, nezu_vac_rc);
		if (nezu_vac_size == 0) {
			CloseHandle(hSteamProc);
			L::Error("Failed to load NezuVac (SizeofResource) 0x%X", GetLastError());
			return FALSE;
		}

		HGLOBAL nezu_vac_handle = LoadResource(NULL, nezu_vac_rc);
		if (!nezu_vac_handle) {
			CloseHandle(hSteamProc);
			L::Error("Failed to load NezuVac (LoadResource) 0x%X", GetLastError());
			return FALSE;
		}

		void* nezu_vac_data = LockResource(nezu_vac_handle);
		if (!nezu_vac_data) {
			CloseHandle(hSteamProc);
			L::Error("Failed to load NezuVac (LockResource) 0x%X", GetLastError());
			return FALSE;
		}


		WCHAR nezu_vac_filename[MAX_PATH];
		if (!PathCombineW(nezu_vac_filename, nezu_injector_dir, L"NezuVac.dll")) {
			CloseHandle(hSteamProc);
			L::Error("Failed to load NezuVac (PathCombineW)(2) 0x%X", GetLastError());
			return FALSE;
		}

		HANDLE hNezuVacFile = CreateFileW(nezu_vac_filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hNezuVacFile == INVALID_HANDLE_VALUE) {
			if (GetLastError() == ERROR_SHARING_VIOLATION) {
				L::Warning("NezuVac.dll is open elsewhere(most likely already injected), continuing injection without overriding", GetLastError());
			}
			else {
				CloseHandle(hSteamProc);
				L::Error("Failed to load NezuVac (CreateFileW) 0x%X", GetLastError());
				return FALSE;
			}
		}

		if (hNezuVacFile != INVALID_HANDLE_VALUE) {
			DWORD bytes_written = 0;
			if (WriteFile(hNezuVacFile, nezu_vac_data, nezu_vac_size, &bytes_written, NULL) == FALSE) {
				CloseHandle(hSteamProc);
				L::Error("Failed to load NezuVac (WriteFile) 0x%X", GetLastError());
				return FALSE;
			}
			if (bytes_written != nezu_vac_size) {
				L::Error("Failed to load NezuVac (bytes_written != nezu_vac_size) 0x%X", GetLastError());
				CloseHandle(hSteamProc);
				return FALSE;
			}

			CloseHandle(hNezuVacFile);

		}

		if (!Injector::LoadLib(hSteamProc, nezu_vac_filename)) {
			L::Error("Failed to load NezuVac (Injector::LoadLib) (%s)", Injector::GetLastErrorString().c_str());
			CloseHandle(hSteamProc);
			return FALSE;
		}

		HMODULE hNezuVac = NULL; 
		if (!U::FindRemoteDll(hSteamProc, _T("NezuVac.dll"), &hNezuVac) || !hNezuVac) {
			L::Error("Failed to confirm NezuVac injection, aborting injection");
			CloseHandle(hSteamProc);
			return FALSE;
		}
		L::Info("NezuVac DLL injected");

		DWORD NezuVacStatusAddress = NezuVac::FindStatusSection(hSteamProc, hNezuVac);
		if (NezuVacStatusAddress == NULL) {
			L::Error("Failed to confirm NezuVac injection (NezuVac::FindStatusSection), aborting injection");
			CloseHandle(hSteamProc);
			return FALSE;
		}

		NezuVac::NeuzStatus nezu_vac_status = NezuVac::GetStatus(hSteamProc, NezuVacStatusAddress);
		if (nezu_vac_status.magic != 'nezu') { //set at compile time, if wrong that means we have the wrong address or RPM failed
			L::Error("Failed to confirm NezuVac injection (invalid magic), aborting injection");
			CloseHandle(hSteamProc);
			return FALSE;
		}
		if (!nezu_vac_status.loaded) { //NeuzStatus::loaded is set in DllMain so it must have been called by now if we injected successfully
			L::Error("Failed to confirm NezuVac injection (DllMain not called), aborting injection");
			CloseHandle(hSteamProc);
			return FALSE;
		}
		L::Debug("Waiting for NezuVac injection confirmation...");
		int tries = 100; // 10s at 100ms intervals
		do {
			if (nezu_vac_status.initialized) {
				L::Info("NezuVac injection confirmed");
				break;
			}
			if (nezu_vac_status.error != NezuVac::Error::NotLoaded && nezu_vac_status.error != NezuVac::Error::Success) {
				L::Error("NezuVac error (%s), aborting injection", NezuVac::GetErrorString(nezu_vac_status.error));
				CloseHandle(hSteamProc);
				return FALSE;
			}
			tries--;
			Sleep(100);
			nezu_vac_status = NezuVac::GetStatus(hSteamProc, NezuVacStatusAddress);
		} while (tries > 0);
		if (tries == 0) {
			L::Error("NezuVac confirmation timeout, aborting injection", NezuVac::GetErrorString(nezu_vac_status.error));
			CloseHandle(hSteamProc);
			return FALSE;
		}

	}
	
	if (hSteamProc) {
		CloseHandle(hSteamProc);
		hSteamProc = NULL;
	}

	if (settings->injectionMode == InjectionMode::NezuVacOnly) {
		return TRUE;
	}

	DWORD csgo_pid = 0;

	if (!settings->RestartSteam) {
		csgo_pid = proc->FindProcess(_T("csgo.exe"));
	}

	if (csgo_pid == 0) {
		if (!settings->RestartSteam) {
			L::Debug("Starting CS:GO");
			proc->Execute(_T("steam://rungameid/730"));
		}

		L::Debug("Waiting for CS:GO to start...");
		do {
			csgo_pid = proc->FindProcess(_T("csgo.exe"));
			Sleep(100);
		} while (csgo_pid == 0);
	}

	L::Info("Found CS:GO as pid: %d", csgo_pid);

	HANDLE hCsProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, csgo_pid);
	if (!hCsProc) {
		L::Error("Failed to open CS:GO process (OpenProcess) 0x%X", GetLastError());
		return FALSE;
	}

	while (!proc->IsProcessInitialized(hCsProc))
		Sleep(50);

	L::Debug("Waiting for CS:GO to fully load before injecting...");

	HMODULE hServerbrowser = NULL;
	while (!hServerbrowser) {
		if (!U::FindRemoteDll(hCsProc, _T("serverbrowser.dll"), &hServerbrowser)) {
			L::Error("Error while waiting for serverbrowser.dll to be loaded, (U::FindRemoteDll)");
			CloseHandle(hCsProc);
			return FALSE;
		}
	}

	L::Debug("Disabling thread library calls for client.dll");
	//anti manual map detection(even if we don't use manual map as injection method the loaded dll might still atempt to "unpack" itself to memory triggering it)
	if (!U::Set_DontCallForThreads(hCsProc, L"client.dll", true)) {
		L::Error("Failed to disable thread library calls, aborting injection");
		CloseHandle(hCsProc);
		return FALSE;
	}

	//utf8 to wide char
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring w_dll_path = converter.from_bytes(settings->dll);
#ifdef UNICODE
	std::wstring dll_path = w_dll_path;
#else
    std::string dll_path = settings.dll;
#endif // UNICODE

	L::Debug("Injecting DLL into CS:GO...");

	bool inj_res = false;
	switch (settings->injectionMode)
	{
	case InjectionMode::NezuLoader:
	{
		HRSRC nezu_loader_rc = FindResource(NULL, MAKEINTRESOURCE(IDR_RCDATA2), RT_RCDATA);
		if (!nezu_loader_rc) {
			L::Error("Failed to load NezuLoader (FindResource) 0x%X", GetLastError());
			inj_res = false;
			break;
		}

		if (SizeofResource(NULL, nezu_loader_rc) == 0) {
			L::Error("Failed to load NezuLoader (SizeofResource) 0x%X", GetLastError());
			inj_res = false;
			break;
		}

		HGLOBAL nezu_loader_handle = LoadResource(NULL, nezu_loader_rc);
		if (!nezu_loader_handle) {
			L::Error("Failed to load NezuLoader (LoadResource) 0x%X", GetLastError());
			inj_res = false;
			break;
		}

		void* nezu_loader_data = LockResource(nezu_loader_handle);
		if (!nezu_loader_data) {
			L::Error("Failed to load NezuLoader (LockResource) 0x%X", GetLastError());
			inj_res = false;
			break;
		}

		HMODULE hRemoteModule = NULL;
		if (!Injector::ManualMap(hCsProc, (const BYTE*)nezu_loader_data, &hRemoteModule)) {
			L::Error("Failed to load NezuLoader (Injector::ManualMap) (%s)", Injector::GetLastErrorString().c_str());
			inj_res = false;
			break;
		}

		L::Info("NezuLoader injected");

		//no DLL selected
		if (settings->dll.empty()) {
			inj_res = true;
			break;
		}

		L::Debug("Injecting DLL using NezuLoader...");

		DWORD dwRemoteInjectFuncRVA = U::GetExportFuncRVA((const PBYTE)nezu_loader_data, "Inject");
		if (dwRemoteInjectFuncRVA == 0) {
			L::Error("Failed to inject DLL (U::GetExportFuncRVA), NezuLoader was injected but the selected DLL was not");
			inj_res = false;
			break;
		}

		PVOID pStringArg = VirtualAllocEx(hCsProc, NULL, (w_dll_path.size() + 1) * sizeof(WCHAR), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if (pStringArg == NULL) {
			L::Error("Failed to inject DLL (VirtualAllocEx) 0x%X, NezuLoader was injected but the selected DLL was not", GetLastError());
			inj_res = false;
			break;
		}

		if (!WriteProcessMemory(hCsProc, pStringArg, w_dll_path.c_str(), (w_dll_path.size() + 1) * sizeof(WCHAR), NULL)) {
			VirtualFreeEx(hCsProc, pStringArg, 0, MEM_RELEASE);
			L::Error("Failed to inject DLL (WriteProcessMemory) 0x%X, NezuLoader was injected but the selected DLL was not", GetLastError());
			inj_res = false;
			break;
		}

		HANDLE hNezuLoaderRemoteThread = CreateRemoteThread(hCsProc, NULL, 0, (LPTHREAD_START_ROUTINE)((DWORD)hRemoteModule + dwRemoteInjectFuncRVA), pStringArg, 0, NULL);
		if (hNezuLoaderRemoteThread == NULL) {
			VirtualFreeEx(hCsProc, pStringArg, 0, MEM_RELEASE);
			L::Error("Failed to inject DLL (WriteProcessMemory) 0x%X, NezuLoader was injected but the selected DLL was not", GetLastError());
			inj_res = false;
			break;
		}

		DWORD wait_res = WaitForSingleObject(hNezuLoaderRemoteThread, 10000);
		if (wait_res != WAIT_OBJECT_0) { //10 sec should be enough
			L::Error("Failed to inject DLL (WaitForSingleObject) 0x%X, NezuLoader was injected but the selected DLL was not", wait_res == WAIT_FAILED ? GetLastError() : wait_res);
			if (wait_res != WAIT_TIMEOUT) { //dont free if thread is still running
				VirtualFreeEx(hCsProc, pStringArg, 0, MEM_RELEASE);
			}
			CloseHandle(hNezuLoaderRemoteThread);
			inj_res = false;
			break;
		}

		VirtualFreeEx(hCsProc, pStringArg, 0, MEM_RELEASE);

		DWORD exit_code = 0;
		if (!GetExitCodeThread(hNezuLoaderRemoteThread, &exit_code)) {
			L::Error("Failed to inject DLL (GetExitCodeThread) 0x%X, NezuLoader was injected but the selected DLL was not", GetLastError());
			CloseHandle(hNezuLoaderRemoteThread);
			inj_res = false;
			break;
		}

		CloseHandle(hNezuLoaderRemoteThread);

		if (exit_code == FALSE) {
			L::Error("NezuLoader returned an error while injecting DLL, check NezuLoader log from more details");
			inj_res = false;
			break;
		}

		inj_res = true;
		break;
	}
	case InjectionMode::Dll:
		inj_res = Injector::LoadLib(hCsProc, dll_path.c_str(), true);
		break;
	case InjectionMode::DllManual:
		inj_res = Injector::ManualMap(hCsProc, dll_path.c_str());
		break;
	}

	if (!inj_res) {
		CloseHandle(hCsProc);
		L::Error("Injection failed");
		return FALSE;
	}

	CloseHandle(hCsProc);

	L::Info("Load successful");

	return TRUE;
}