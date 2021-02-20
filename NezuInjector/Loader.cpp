#include "pch.h"
#include "Loader.h"
#include "Injector.h"
#include "resource.h"
#include "NezuVacInterface.h"

DWORD WINAPI LoaderThread(LPVOID lpThreadParameter) {

	NezuInjectorSettings* settings = (NezuInjectorSettings*)lpThreadParameter;
	HANDLE hSteamProc = NULL;

	if (settings->RestartSteam) {

		//make sure steam is dead
		do  {
			U::KillAll(_T("csgo.exe"));
			U::KillAll(_T("steam.exe"));
			U::KillAll(_T("steamservice.exe"));
			U::KillAll(_T("steamwebhelper.exe"));

			Sleep(100);
		} while (U::IsProcessOpen(_T("steam.exe")) || U::IsProcessOpen(_T("steamservice.exe")) || U::IsProcessOpen(_T("steamwebhelper.exe")));

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
		if (settings->injectionMode != InjectionMode::NezuVacOnly) {
			commandlaine += _T("-silent -applaunch 730");//only start cs if we are injecting into it
		}
		//CreateProcessW can change the buffer so we need to coppy it
		DWORD commandlaine_buf_len = (commandlaine.length() + 1) * sizeof(TCHAR);
		LPTSTR commandlaine_buf = new TCHAR[commandlaine_buf_len];
		_tcsncpy_s(commandlaine_buf, commandlaine_buf_len, commandlaine.c_str(), commandlaine.length());

		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);

		PROCESS_INFORMATION pi = { 0 };
		if (!CreateProcess(NULL, commandlaine_buf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			delete[] commandlaine_buf;
			L::Error("Failed to start Steam (CreateProcess) 0x%X", GetLastError());
			return FALSE;
		}
		delete[] commandlaine_buf;
		CloseHandle(pi.hThread);

		L::Info("Steam started");

		hSteamProc = pi.hProcess;

	}

	if (settings->VacBypass) {

		if (!hSteamProc) { //if steam was not restarted just open it normaly

			DWORD steam_pid = U::FindProcess(_T("steam.exe"));
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

		PWSTR win_dir = NULL;
		if (FAILED(SHGetKnownFolderPath(FOLDERID_Windows, 0, NULL, &win_dir))) {
			CoTaskMemFree(win_dir);
			CloseHandle(hSteamProc);
			L::Error("Failed to load NezuVac (SHGetKnownFolderPath) 0x%X", GetLastError());
			return FALSE;
		}

		WCHAR nezu_vac_filename[MAX_PATH];
		if (!PathCombineW(nezu_vac_filename, win_dir, L"NezuInjectorTmp\\NezuVac.dll")) {
			CoTaskMemFree(win_dir);
			CloseHandle(hSteamProc);
			L::Error("Failed to load NezuVac (PathCombineW) 0x%X", GetLastError());
			return FALSE;
		}

		CoTaskMemFree(win_dir);

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
		int tries = 50; // 5s at 100ms intervals
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
		csgo_pid = U::FindProcess(_T("csgo.exe"));
	}

	if (csgo_pid == 0) {
		if (!settings->RestartSteam) {
			L::Debug("Starting CS:GO");
			ShellExecute(NULL, NULL, _T("steam://rungameid/730"), NULL, NULL, SW_SHOW);
		}

		L::Debug("Waiting for CS:GO to start...");
		do {
			csgo_pid = U::FindProcess(_T("csgo.exe"));
			Sleep(100);
		} while (csgo_pid == 0);
	}

	L::Info("Found CS:GO as pid: %d", csgo_pid);

	HANDLE hCsProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, csgo_pid);
	if (!hCsProc) {
		L::Error("Failed to open CS:GO process (OpenProcess) 0x%X", GetLastError());
		return FALSE;
	}

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

#ifdef UNICODE
	//utf8 to WCHAR
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring dll_path = converter.from_bytes(settings->dll);
#else
    std::string dll_path = settings.dll;
#endif // UNICODE

	L::Debug("Injecting DLL into CS:GO...");

	bool inj_res = false;
	switch (settings->injectionMode)
	{
	case InjectionMode::NezuLoader:
			inj_res = false;//not implemented
			break;
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

	L::Info("DLL injected");

	return TRUE;
}