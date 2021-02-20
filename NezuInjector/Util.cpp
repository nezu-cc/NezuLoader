#include "pch.h"
#include "Util.h"

/// <summary>
/// simple file picker for DLL files
/// </summary>
/// <param name="owner">owning window. Can be NULL</param>
/// <returns>the selected file path ancoded in utf8 or empty string on error</returns>
std::string U::DllFilePicker(HWND owner) {
    TCHAR filename[MAX_PATH];
    OPENFILENAME ofn;
    ZeroMemory(&filename, sizeof(filename));
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = _T("DLL Files\0*.dll\0All Fils\0*.*\0");
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = _T("Select Dll to inject");
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        return TCHARUTF8(filename);
        _ftprintf(stdout, _T("You chose the file %s\n"), filename);
    }
    return "";
}

/// <summary>
/// External implementation of DisableThreadLibraryCalls that also can be re enabled
/// </summary>
/// <param name="hProc">Target process handle</param>
/// <param name="cMod">name of the module</param>
/// <param name="set">true to disable false to enable</param>
/// <returns>TRUE on success FALSE otherwise</returns>
BOOL U::Set_DontCallForThreads(HANDLE hProc, const WCHAR* cMod, bool set) {
    PROCESS_BASIC_INFORMATION pbi = { 0 };
    PEB peb = { 0 };

#pragma warning( suppress : 6387 ) //ntdll.dll is always loaded first no matter what
    tNtQueryInformationProcess NtQueryInformationProcess =
        (tNtQueryInformationProcess)GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "NtQueryInformationProcess");

    //get peb location
    if (!NT_SUCCESS(NtQueryInformationProcess(hProc, ProcessBasicInformation, &pbi, sizeof(pbi), 0))) {
        L::Error("DontCallForThread failed (NtQueryInformationProcess) 0x%X", GetLastError());
        return FALSE;
    }

    //read peb
#pragma warning( suppress : 6387 ) //we literaly just checked it
    if (!ReadProcessMemory(hProc, pbi.PebBaseAddress, &peb, sizeof(peb), 0)) { //only checking the first RPM call to make sure we have VM_READ permisions
        L::Error("DontCallForThread failed (ReadProcessMemory) 0x%X", GetLastError());
        return FALSE;
    }

    //read LDR_DATA from peb
    _PEB_LDR_DATA ldr = { 0 };
    ReadProcessMemory(hProc, peb.Ldr, &ldr, sizeof(ldr), 0);

    //get first module
    DWORD moduleListTail = ((DWORD)peb.Ldr + offsetof(_PEB_LDR_DATA, InMemoryOrderModuleList));
    DWORD moduleList = 0;
    ReadProcessMemory(hProc, (void*)(moduleListTail + offsetof(_LIST_ENTRY, Flink)), &moduleList, sizeof(moduleList), 0);

    WCHAR BaseName[MAX_PATH] = { 0 };
    do {
        DWORD modulePtrWithOffset = moduleList - sizeof(LIST_ENTRY); //LDR_DATA_TABLE_ENTRY address

        LDR_DATA_TABLE_ENTRY entry = { 0 };
        ReadProcessMemory(hProc, (void*)modulePtrWithOffset, &entry, sizeof(entry), 0);
        if (entry.BaseDllName.Buffer) {
            ReadProcessMemory(hProc, entry.BaseDllName.Buffer, BaseName, sizeof(BaseName), 0);
            if (!_wcsicmp(cMod, BaseName)) {
                entry.DontCallForThreads = set ? 1 : 0;
                if (!WriteProcessMemory(hProc, (void*)modulePtrWithOffset, &entry, sizeof(entry), 0)) {
                    L::Error("DontCallForThread failed (WriteProcessMemory) 0x%X", GetLastError());
                    return FALSE;
                }
                ReadProcessMemory(hProc, (void*)modulePtrWithOffset, &entry, sizeof(entry), 0);//re-read it to make sure it was set (not nececary but i like it)
                if (entry.DontCallForThreads != set ? 1 : 0) {
                    L::Error("DontCallForThread failed (verification failed)");
                    return FALSE;
                }
                return TRUE;
            }
        }

        //get address of next link in the chain
        ReadProcessMemory(hProc, (void*)(moduleList + offsetof(_LIST_ENTRY, Flink)), &moduleList, sizeof(moduleList), 0);
    } while (moduleList != moduleListTail); //last one will point to the first

    L::Error("DLL not found in PEB");
    return FALSE;
}

/// <summary>
/// find first process with given name
/// </summary>
/// <param name="name">name of the process</param>
/// <returns>PID or 0 if none found</returns>
DWORD U::FindProcess(LPCTSTR name) {
    PROCESSENTRY32 PE32{ 0 };
    PE32.dwSize = sizeof(PE32);

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE)
        return 0;

    DWORD PID = 0;
    BOOL bRet = Process32First(hSnap, &PE32);
    while (bRet) {
        if (!_tcsicmp(name, PE32.szExeFile)) {
            PID = PE32.th32ProcessID;
            break;
        }
        bRet = Process32Next(hSnap, &PE32);
    }

    CloseHandle(hSnap);

    return PID;
}

/// <summary>
/// Tries to kill all proceses with given filename
/// </summary>
/// <param name="name">name of the process</param>
/// <returns>number of proceses that failed to be killed or (DWORD)-1 on failure</returns>
DWORD U::KillAll(LPCTSTR name) {
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
        bRet = Process32Next(hSnap, &PE32);
    }

    CloseHandle(hSnap);

    return failed;
}

/// <summary>
/// Checks if a process is running
/// </summary>
/// <param name="name">name of the process</param>
/// <returns>TRUE if at least one process with a given name is running, FALSE otherwise</returns>
BOOL U::IsProcessOpen(LPCTSTR name) {
    PROCESSENTRY32 PE32{ 0 };
    PE32.dwSize = sizeof(PE32);

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE)
        return (DWORD)-1;

    BOOL bRet = Process32First(hSnap, &PE32);
    while (bRet) {
        if (!_tcsicmp(name, PE32.szExeFile)) {
            CloseHandle(hSnap);
            return true;
        }
        bRet = Process32Next(hSnap, &PE32);
    }

    CloseHandle(hSnap);

    return false;
}

/// <summary>
/// Finds the address of a module in a remote process
/// </summary>
/// <param name="hProc">handle to the target process</param>
/// <param name="name">name of the module</param>
/// <param name="address">address if the module if found</param>
/// <returns>TRUE if no error occurred(address will be set to NULL if module not found) and FALSE on failure</returns>
BOOL U::FindRemoteDll(HANDLE hProc, LPCTSTR name, HMODULE* address) {

    DWORD cbNeeded = 0;
    if (!EnumProcessModules(hProc, NULL, 0, &cbNeeded)) {
        L::Error("EnumProcessModules failed 0x%X", GetLastError());
        *address = NULL;
        return FALSE;
    }

    int module_count = cbNeeded / sizeof(HMODULE) + 20;
    HMODULE* modules = new HMODULE[module_count]; //a bit more in case anny get loaded in beetween calls
    if (!EnumProcessModules(hProc, modules, module_count * sizeof(HMODULE), &cbNeeded)) {
        *address = NULL;
        delete[] modules;
        L::Error("EnumProcessModules(2) failed 0x%X", GetLastError());
        return FALSE;
    }
    if (cbNeeded > module_count * sizeof(HMODULE)) {//fuck this rare race condition, just error out
        *address = NULL;
        delete[] modules;
        L::Error("EnumProcessModules(2) failed (not enough memory allocated)");
        return FALSE;
    }

    module_count = cbNeeded / sizeof(HMODULE);

    TCHAR ModuleNameBuffer[MAX_PATH];
    for (int i = 0; i < module_count; i++) {
        if (!GetModuleBaseName(hProc, modules[i], ModuleNameBuffer, sizeof(ModuleNameBuffer) / sizeof(TCHAR))) {
            /**address = NULL;
            delete[] modules;
            L::Error("GetModuleBaseName failed 0x%X", GetLastError());
            return FALSE;*/
            continue; //just ignore
        }

        if (!_tcsicmp(ModuleNameBuffer, name)) {
            *address = modules[i];
            delete[] modules;
            return TRUE;
        }

    }
    *address = NULL;
    delete[] modules;
    return TRUE;
}