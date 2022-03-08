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
    }
    return "";
}

/// <summary>
/// simple picker for directories
/// </summary>
/// <param name="owner">owning window. Can be NULL</param>
/// <returns>the selected path ancoded in utf8 or empty string on error</returns>
std::string U::DirectoryPicker(HWND owner) {
    TCHAR dir[MAX_PATH];
    BROWSEINFO bInfo;
    bInfo.hwndOwner = owner;
    bInfo.pidlRoot = NULL;
    bInfo.pszDisplayName = dir;
    bInfo.lpszTitle = _T("Select local config directory. Example: C:\\Program Files (x86)\\Steam\\userdata\\<SteamID>\\730\\local");
    bInfo.ulFlags = 0;
    bInfo.lpfn = NULL;
    bInfo.lParam = 0;
    bInfo.iImage = -1;

    LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);
    if (lpItem != NULL) {
        SHGetPathFromIDList(lpItem, dir);
        return TCHARUTF8(dir);
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
/// Finds the address of a module in a remote process
/// </summary>
/// <param name="hProc">handle to the target process</param>
/// <param name="name">name of the module</param>
/// <param name="address">address if the module if found (may be NULL)</param>
/// <param name="fast">don't retry after fail</param>
/// <returns>TRUE if no error occurred(address will be set to NULL if module not found) and FALSE on failure</returns>
BOOL U::FindRemoteDll(HANDLE hProc, LPCTSTR name, HMODULE* address, BOOL fast) {

    DWORD cbNeeded = 0;
    DWORD tries = 0;
    while (!EnumProcessModules(hProc, NULL, 0, &cbNeeded)) {
        if (fast)
            return FALSE;
        Sleep(100);//problem reading memory, just wait. The process is propably not initalised yet
        tries++;
        if (tries == 50) {//5 seconds
            L::Error("EnumProcessModules failed 0x%X", GetLastError());
            *address = NULL;
            return FALSE;
        }
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

/// <summary>
/// Gets the RVA or a exported function
/// </summary>
/// <param name="pAssembly">pointer to buffer countaining a DLL file</param>
/// <param name="lpProcName">name of the export to find(dosn't support ordinals)</param>
/// <returns>RVA on success or 0 if not found</returns>
DWORD U::GetExportFuncRVA(const PBYTE pAssembly, LPCSTR lpProcName) {
    PIMAGE_DOS_HEADER dos_hdr = (PIMAGE_DOS_HEADER)pAssembly;
    PIMAGE_NT_HEADERS nt_hdr = (PIMAGE_NT_HEADERS)(pAssembly + dos_hdr->e_lfanew);
    PIMAGE_OPTIONAL_HEADER opt_hdr = &nt_hdr->OptionalHeader;
    PIMAGE_FILE_HEADER file_hdr = &nt_hdr->FileHeader;

    //map all sections so we can use virtual addresses
    PBYTE mapped = new BYTE[opt_hdr->SizeOfImage];
    auto* section_hdr = IMAGE_FIRST_SECTION(nt_hdr);
    for (UINT i = 0; i != file_hdr->NumberOfSections; ++i, ++section_hdr)
        if (section_hdr->SizeOfRawData)
            memcpy(mapped + section_hdr->VirtualAddress, pAssembly + section_hdr->PointerToRawData, section_hdr->SizeOfRawData);

    PIMAGE_DATA_DIRECTORY data_dir = (PIMAGE_DATA_DIRECTORY)&opt_hdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    PIMAGE_EXPORT_DIRECTORY export_dir = (PIMAGE_EXPORT_DIRECTORY)(mapped + data_dir->VirtualAddress);

    DWORD* addresses = (DWORD*)(mapped + export_dir->AddressOfFunctions);
    DWORD* names = (DWORD*)(mapped + export_dir->AddressOfNames);
    WORD* names_ordinals = (WORD*)(mapped + export_dir->AddressOfNameOrdinals);

    DWORD i = export_dir->NumberOfNames;
    while (i--) {
        char* name = (char*)(mapped + *names);
#pragma warning( suppress : 6001 ) //no, we're infact not using uninitialized memory
        if (!strcmp(name, lpProcName)) {
            addresses += *names_ordinals;
            DWORD RVA = *addresses;
            delete[] mapped;
            return RVA;
        }
        names++;
        names_ordinals++;
    }
    delete[] mapped;
    return 0;
}

/// <summary>
/// Finds the binary path for a given service
/// </summary>
/// <param name="cName">name of the service</param>
/// <param name="cBinPath">buffer for the returned path</param>
/// <param name="buffSize">size of cBinPath</param>
/// <returns>TRUE on success FALSE otherwise</returns>
BOOL U::GetServiceBinaryPath(LPCTSTR cName, LPTSTR cBinPath, DWORD buffSize) {

    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!schSCManager)
        return FALSE;

    SC_HANDLE schService = OpenService(schSCManager, cName, SERVICE_QUERY_CONFIG);
    if (!schService) {
        CloseServiceHandle(schSCManager);
        return FALSE;
    }

    DWORD dwBytesNeeded = 0;
    if (!QueryServiceConfig(schService, NULL, 0, &dwBytesNeeded) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return FALSE;
    }

    if (!dwBytesNeeded) {
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return FALSE;
    }

    LPQUERY_SERVICE_CONFIG lpsc = (LPQUERY_SERVICE_CONFIG)LocalAlloc(LMEM_FIXED, dwBytesNeeded);
    if (!lpsc) {
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return FALSE;
    }

    if (!QueryServiceConfig(schService, lpsc, dwBytesNeeded, &dwBytesNeeded)) {
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        LocalFree(lpsc);
        return FALSE;
    }

    _tcscpy_s(cBinPath, buffSize, lpsc->lpBinaryPathName);

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    LocalFree(lpsc);

    return TRUE;

}

BOOL U::SetDebugPrivilege(BOOL enable) {

    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return FALSE;

    TOKEN_PRIVILEGES tp = { 0 };
    if (!LookupPrivilegeValue(NULL, L"SeDebugPrivilege", &tp.Privileges[0].Luid)) {
        CloseHandle(hToken);
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = enable ? SE_PRIVILEGE_ENABLED : SE_PRIVILEGE_USED_FOR_ACCESS;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, 0)) {
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);
    return TRUE;

}