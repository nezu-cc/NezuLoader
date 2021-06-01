#include "pch.h"
#include "Injector.h"

std::string last_error = "";
f_RtlInsertInvertedFunctionTable Injector::RtlInsertInvertedFunctionTable = NULL;

std::string& Injector::GetLastErrorString() {
	return last_error;
}

void Injector::SetLastErrorString(LPCSTR fmt, ...) {
	CHAR buff[512];
	va_list args;
	va_start(args, fmt);
	vsprintf_s(buff, fmt, args);
	va_end(args);
	last_error = buff;
}

#ifdef _DEBUG
__declspec(noinline)
#endif
bool Injector::ManualMap(HANDLE hProc, const WCHAR* filename) {
	SetLastErrorString(""); //clear errors

	BYTE* pSrcData = NULL;

	if (GetFileAttributesW(filename) == INVALID_FILE_ATTRIBUTES) {
		SetLastErrorString("File doesn't exist\n");
		return false;
	}

	std::ifstream File(filename, std::ios::binary | std::ios::ate);

	if (File.fail()) {
		SetLastErrorString("Opening the file failed: 0x%X", (DWORD)File.rdstate());
		File.close();
		return false;
	}

	auto FileSize = File.tellg();
	if (FileSize < 0x100) {
		SetLastErrorString("Filesize is invalid");
		File.close();
		return false;
	}

	pSrcData = new BYTE[(UINT_PTR)FileSize];
	if (!pSrcData) {
		SetLastErrorString("Memory allocating failed");
		File.close();
		return false;
	}

	File.seekg(0, std::ios::beg);
	File.read((char*)pSrcData, FileSize);
	File.close();

	bool ret = ManualMap(hProc, pSrcData, NULL);

	delete[] pSrcData;
	return ret;
}

#ifdef MM_INTERNAL

f_MMapCallback m_map_callback = NULL;

void Injector::SetMMapCallback(f_MMapCallback callback) {
	m_map_callback = callback;
}

#ifdef _DEBUG
__declspec(noinline)
#endif
bool Injector::ManualMap(HANDLE hProc, const BYTE* pSrcData, HMODULE* hRemoteModule) {

	SetLastErrorString(""); //clear errors

	if (hProc != GetCurrentProcess()) {
		SetLastErrorString("Memory allocation failed (ex) 0x%X", GetLastError());
		return false;
	}

	const IMAGE_NT_HEADERS* pOldNtHeader = nullptr;
	const IMAGE_OPTIONAL_HEADER* pOldOptHeader = nullptr;
	const IMAGE_FILE_HEADER* pOldFileHeader = nullptr;
	BYTE* pTargetBase = nullptr;

	if (((const IMAGE_DOS_HEADER*)pSrcData)->e_magic != 0x5A4D) { //'MZ'
		SetLastErrorString("Invalid file");
		return false;
	}

	pOldNtHeader = (const IMAGE_NT_HEADERS*)(pSrcData + ((const IMAGE_DOS_HEADER*)pSrcData)->e_lfanew);
	pOldOptHeader = &pOldNtHeader->OptionalHeader;
	pOldFileHeader = &pOldNtHeader->FileHeader;

	pTargetBase = (BYTE*)VirtualAlloc((void*)pOldOptHeader->ImageBase, pOldOptHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pTargetBase) {
		pTargetBase = (BYTE*)VirtualAlloc(nullptr, pOldOptHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!pTargetBase) {
			SetLastErrorString("Memory allocation failed (ex) 0x%X", GetLastError());
			return false;
		}
	}

	auto* pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
	for (UINT i = 0; i != pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader)
		if (pSectionHeader->SizeOfRawData)
			memcpy(pTargetBase + pSectionHeader->VirtualAddress, pSrcData + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData);

	memcpy(pTargetBase, pSrcData, pOldOptHeader->SizeOfHeaders);

	BYTE* LocationDelta = pTargetBase - pOldOptHeader->ImageBase;
	if (LocationDelta) {
		if (!pOldOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)
			return FALSE;

		auto* pRelocData = (IMAGE_BASE_RELOCATION*)(pTargetBase + pOldOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		while (pRelocData->VirtualAddress) {
			UINT AmountOfEntries = (pRelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			WORD* pRelativeInfo = (WORD*)(pRelocData + 1);

			for (UINT i = 0; i != AmountOfEntries; ++i, ++pRelativeInfo) {
				if ((*pRelativeInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW) {
					UINT_PTR* pPatch = (UINT_PTR*)(pTargetBase + pRelocData->VirtualAddress + ((*pRelativeInfo) & 0xFFF));
					*pPatch += (UINT_PTR)LocationDelta;
				}
			}
			pRelocData = (IMAGE_BASE_RELOCATION*)((BYTE*)pRelocData + pRelocData->SizeOfBlock);
		}
	}

	if (pOldOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
		auto* pImportDescr = (IMAGE_IMPORT_DESCRIPTOR*)(pTargetBase + pOldOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		while (pImportDescr->Name) {
			char* szMod = (char*)(pTargetBase + pImportDescr->Name);
			HINSTANCE hDll = LoadLibraryA(szMod);

			ULONG_PTR* pThunkRef = (ULONG_PTR*)(pTargetBase + pImportDescr->OriginalFirstThunk);
			ULONG_PTR* pFuncRef = (ULONG_PTR*)(pTargetBase + pImportDescr->FirstThunk);

			if (!pThunkRef)
				pThunkRef = pFuncRef;

			for (; *pThunkRef; ++pThunkRef, ++pFuncRef) {
				if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef)) {
					*pFuncRef = (UINT_PTR)GetProcAddress(hDll, (char*)(*pThunkRef & 0xFFFF));
				}
				else {
					auto* pImport = (IMAGE_IMPORT_BY_NAME*)(pTargetBase + (*pThunkRef));
					*pFuncRef = (UINT_PTR)GetProcAddress(hDll, pImport->Name);
				}
			}
			++pImportDescr;
		}
	}

	if (m_map_callback) {
		if (!m_map_callback(pTargetBase)) {
			VirtualFree(pTargetBase, 0, MEM_RELEASE);
			SetLastErrorString("MM calback failed");
			return false;
		}
	}

	if (pOldOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size) {
		auto* pTLS = (IMAGE_TLS_DIRECTORY*)(pTargetBase + pOldOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
		auto* pCallback = (PIMAGE_TLS_CALLBACK*)(pTLS->AddressOfCallBacks);
		for (; pCallback && *pCallback; ++pCallback)
			(*pCallback)((PVOID)pTargetBase, DLL_PROCESS_ATTACH, nullptr);
	}

	if (RtlInsertInvertedFunctionTable && NT_FAIL(RtlInsertInvertedFunctionTable((PVOID)pTargetBase, pOldOptHeader->SizeOfImage))) {
		VirtualFree(pTargetBase, 0, MEM_RELEASE);
		SetLastErrorString("RtlInsertInvertedFunctionTable failed 0x%X", GetLastError());
		return false;
	}

	auto _DllMain = (f_DLL_ENTRY_POINT)(pTargetBase + pOldOptHeader->AddressOfEntryPoint);
	_DllMain((PVOID)pTargetBase, DLL_PROCESS_ATTACH, nullptr);

	return true;
}
#else

BOOL  __declspec(code_seg(".inj_sec$1")) __stdcall Shellcode(MANUAL_MAPPING_DATA* pData);
DWORD __declspec(code_seg(".inj_sec$2")) __stdcall Shellcode_End();

#ifdef _DEBUG
__declspec(noinline)
#endif
bool Injector::ManualMap(HANDLE hProc, const BYTE* pSrcData, HMODULE* hRemoteModule) {

	SetLastErrorString(""); //clear errors

	const IMAGE_NT_HEADERS* pOldNtHeader = nullptr;
	const IMAGE_OPTIONAL_HEADER* pOldOptHeader = nullptr;
	const IMAGE_FILE_HEADER* pOldFileHeader = nullptr;
	BYTE* pTargetBase = nullptr;

	if (((const IMAGE_DOS_HEADER*)pSrcData)->e_magic != 0x5A4D) { //'MZ'
		SetLastErrorString("Invalid file");
		return false;
	}

	pOldNtHeader = (const IMAGE_NT_HEADERS*)(pSrcData + ((const IMAGE_DOS_HEADER*)pSrcData)->e_lfanew);
	pOldOptHeader = &pOldNtHeader->OptionalHeader;
	pOldFileHeader = &pOldNtHeader->FileHeader;

	pTargetBase = (BYTE*)VirtualAllocEx(hProc, (void*)pOldOptHeader->ImageBase, pOldOptHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pTargetBase) {
		pTargetBase = (BYTE*)VirtualAllocEx(hProc, nullptr, pOldOptHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!pTargetBase) {
			SetLastErrorString("Memory allocation failed (ex) 0x%X", GetLastError());
			return false;
		}
	}

	MANUAL_MAPPING_DATA data{ 0 };
	data.base = pTargetBase;
	data.pLoadLibraryA = LoadLibraryA;
	data.pGetProcAddress = (f_GetProcAddress)GetProcAddress;
	data.pRtlInsertInvertedFunctionTable = RtlInsertInvertedFunctionTable; //might be NULL

	auto* pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
	for (UINT i = 0; i != pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
		if (pSectionHeader->SizeOfRawData) {
			if (!WriteProcessMemory(hProc, pTargetBase + pSectionHeader->VirtualAddress, pSrcData + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData, nullptr)) {
				SetLastErrorString("Can't map sections: 0x%x", GetLastError());
				VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
				return false;
			}
		}
	}

	DWORD headers_size = ((DWORD)pSectionHeader + sizeof(*pSectionHeader)) - (DWORD)pSrcData; //(last header + size of last header) - base = size of all headers
	WriteProcessMemory(hProc, pTargetBase, pSrcData, headers_size, nullptr);

	DWORD shellcode_size = (DWORD)Shellcode_End - (DWORD)Shellcode;

	void* pShellcode = VirtualAllocEx(hProc, nullptr, shellcode_size + sizeof(data), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!pShellcode) {
		SetLastErrorString("Memory allocation failed (1) (ex) 0x%X", GetLastError());
		VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
		return false;
	}

	WriteProcessMemory(hProc, pShellcode, Shellcode, shellcode_size, nullptr);
	WriteProcessMemory(hProc, (BYTE*)pShellcode + shellcode_size, &data, sizeof(data), nullptr);

	HANDLE hThread = CreateRemoteThread(hProc, nullptr, 0, (LPTHREAD_START_ROUTINE)pShellcode, (BYTE*)pShellcode + shellcode_size, 0, nullptr);
	if (!hThread) {
		SetLastErrorString("Thread creation failed 0x%X", GetLastError());
		VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
		VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE);
		return false;
	}

	DWORD wait_res = WaitForSingleObject(hThread, 10000);
	if (wait_res != WAIT_OBJECT_0) { //10 sec should be enough
		SetLastErrorString("WaitForSingleObject failed 0x%X", wait_res == WAIT_FAILED ? GetLastError() : wait_res);
		if (wait_res != WAIT_TIMEOUT) { //dont free if thread is still running
			VirtualFreeEx(hProc, pTargetBase, 0, MEM_RELEASE);
			VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE);
		}
		CloseHandle(hThread);
		return false;
	}

	DWORD exit_code = 0;
	if (!GetExitCodeThread(hThread, &exit_code)) {
		SetLastErrorString("GetExitCodeThread failed 0x%X", wait_res == WAIT_FAILED ? GetLastError() : wait_res);
		CloseHandle(hThread);
		return false;
	}

	CloseHandle(hThread);

	if (exit_code == FALSE) {
		SetLastErrorString("Shellcode failed in remote process(return)");
		return false;
	}

	MANUAL_MAPPING_DATA data_checked{ 0 };
	ReadProcessMemory(hProc, (BYTE*)pShellcode + shellcode_size, &data_checked, sizeof(data_checked), nullptr);

	VirtualFreeEx(hProc, pShellcode, 0, MEM_RELEASE);

	if (hRemoteModule != NULL) {
		*hRemoteModule = data_checked.hMod;
	}

	if (data_checked.hMod == NULL) {
		SetLastErrorString("Shellcode failed in remote process(hMod)");
		return false;
	}

	SetLastErrorString("");
	return true;
}

BOOL __declspec(code_seg(".inj_sec$1")) __stdcall Shellcode(MANUAL_MAPPING_DATA* pData) {
	if (!pData)
		return FALSE;

	BYTE* pBase = pData->base;
	auto* pOpt = &((IMAGE_NT_HEADERS*)(pBase + ((IMAGE_DOS_HEADER*)pBase)->e_lfanew))->OptionalHeader;

	auto _LoadLibraryA = pData->pLoadLibraryA;
	auto _GetProcAddress = pData->pGetProcAddress;
	auto _RtlInsertInvertedFunctionTable = pData->pRtlInsertInvertedFunctionTable;
	auto _DllMain = (f_DLL_ENTRY_POINT)(pBase + pOpt->AddressOfEntryPoint);

	BYTE* LocationDelta = pBase - pOpt->ImageBase;
	if (LocationDelta) {
		if (!pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)
			return FALSE;

		auto* pRelocData = (IMAGE_BASE_RELOCATION*)(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		while (pRelocData->VirtualAddress) {
			UINT AmountOfEntries = (pRelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			WORD* pRelativeInfo = (WORD*)(pRelocData + 1);

			for (UINT i = 0; i != AmountOfEntries; ++i, ++pRelativeInfo) {
				if ((*pRelativeInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW) {
					UINT_PTR* pPatch = (UINT_PTR*)(pBase + pRelocData->VirtualAddress + ((*pRelativeInfo) & 0xFFF));
					*pPatch += (UINT_PTR)LocationDelta;
				}
			}
			pRelocData = (IMAGE_BASE_RELOCATION*)((BYTE*)pRelocData + pRelocData->SizeOfBlock);
		}
	}

	if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
		auto* pImportDescr = (IMAGE_IMPORT_DESCRIPTOR*)(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		while (pImportDescr->Name) {
			char* szMod = (char*)(pBase + pImportDescr->Name);
			HINSTANCE hDll = _LoadLibraryA(szMod);

			ULONG_PTR* pThunkRef = (ULONG_PTR*)(pBase + pImportDescr->OriginalFirstThunk);
			ULONG_PTR* pFuncRef = (ULONG_PTR*)(pBase + pImportDescr->FirstThunk);

			if (!pThunkRef)
				pThunkRef = pFuncRef;

			for (; *pThunkRef; ++pThunkRef, ++pFuncRef) {
				if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef)) {
					*pFuncRef = _GetProcAddress(hDll, (char*)(*pThunkRef & 0xFFFF));
				}
				else {
					auto* pImport = (IMAGE_IMPORT_BY_NAME*)(pBase + (*pThunkRef));
					*pFuncRef = _GetProcAddress(hDll, pImport->Name);
				}
			}
			++pImportDescr;
		}
	}

	if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size) {
		auto* pTLS = (IMAGE_TLS_DIRECTORY*)(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
		auto* pCallback = (PIMAGE_TLS_CALLBACK*)(pTLS->AddressOfCallBacks);
		for (; pCallback && *pCallback; ++pCallback)
			(*pCallback)(pBase, DLL_PROCESS_ATTACH, nullptr);
	}

	//if we have it, use it to enable SEH
	if (_RtlInsertInvertedFunctionTable && NT_FAIL(_RtlInsertInvertedFunctionTable(pBase, pOpt->SizeOfImage)))
		return FALSE;

	_DllMain(pBase, DLL_PROCESS_ATTACH, nullptr);

	pData->hMod = (HINSTANCE)pBase;

	return TRUE;
}

DWORD __declspec(code_seg(".inj_sec$2")) __stdcall Shellcode_End() {
	return 0;
}

#endif // MM_INTERNAL



bool Injector::CoppyToWindowsDir(const WCHAR* filename, WCHAR* new_filename) {

	PWSTR win_dir = NULL;
	if (FAILED(SHGetKnownFolderPath(FOLDERID_Windows, 0, NULL, &win_dir))) {
		CoTaskMemFree(win_dir);
		SetLastErrorString("SHGetKnownFolderPath failed 0x%X", GetLastError());
		return false;
	}

	if (!PathCombineW(new_filename, win_dir, L"NezuInjectorTmp")) {
		CoTaskMemFree(win_dir);
		SetLastErrorString("PathCombineW failed 0x%X", GetLastError());
		return false;
	}

	CoTaskMemFree(win_dir);

	if (!CreateDirectoryW(new_filename, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) { //It's ok if it exists
		SetLastErrorString("CreateDirectoryW failed 0x%X", GetLastError());
		return false;
	}

	LPCWSTR just_file = PathFindFileNameW(filename);
	if (filename == just_file) {//Returns a pointer to the address of the string if successful, or a pointer to the beginning of the path otherwise.
		SetLastErrorString("PathFindFileNameW failed on %ws", filename);
		return false;
	}

	WCHAR new_file_name[MAX_PATH];
#pragma warning( suppress : 28159 ) //we dont care if GetTickCount loops
	swprintf_s(new_file_name, L"%06d_%s", GetTickCount() % 100000, just_file);

	if (!PathCombineW(new_filename, new_filename, new_file_name)) { //idk if it's safe to put the output buffer as one of the inputs but seems to work fine
		SetLastErrorString("PathCombineW failed 0x%X", GetLastError());
		return false;
	}

	if (!CopyFileW(filename, new_filename, TRUE)) { //we could allow it to reuse a file but to avoid hard to trace problems we don't(src dll was moddified but old one loaded and similar bugs)
		SetLastErrorString("CopyFileW failed 0x%X", GetLastError());
		return false;
	}

	if (GetFileAttributesW(new_filename) == INVALID_FILE_ATTRIBUTES) { //just making sure
		SetLastErrorString("New file doesn't exist");
		return false;
	}

	return true;

}

#ifdef _DEBUG
__declspec(noinline)
#endif
bool Injector::LoadLib(HANDLE hProc, const WCHAR* filename, bool bypass_secure) {

	SetLastErrorString(""); //clear errors

	if (GetFileAttributesW(filename) == INVALID_FILE_ATTRIBUTES) {
		SetLastErrorString("File doesn't exist");
		return false;
	}

	WCHAR nezu_path[MAX_PATH];

	if (bypass_secure) {

		if (!CoppyToWindowsDir(filename, nezu_path))
			return false;

		filename = nezu_path;
	}

	void* pMem = VirtualAllocEx(hProc, NULL, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); //MAX_PATH * sizeof(WCHAR) is still way below a page so we just alocate a single 4k page
	if (!pMem) {
		SetLastErrorString("Memory allocation failed (ex) 0x%X", GetLastError());
		return false;
	}

	if (!WriteProcessMemory(hProc, pMem, filename, (wcsnlen(filename, 0x1000 / sizeof(WCHAR)) + 1) * sizeof(WCHAR), NULL)) {
		SetLastErrorString("WriteProcessMemory failed: 0x%x", GetLastError());
		VirtualFreeEx(hProc, pMem, 0, MEM_RELEASE);
		return false;
	}

	HANDLE hThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryW, pMem, 0, NULL);
	if (!hThread) {
		SetLastErrorString("Thread creation failed 0x%X", GetLastError());
		VirtualFreeEx(hProc, pMem, 0, MEM_RELEASE);
		return false;
	}

	DWORD wait_res = WaitForSingleObject(hThread, 10000);
	if (wait_res != WAIT_OBJECT_0) { //10 sec should be enough
		SetLastErrorString("WaitForSingleObject failed 0x%X", wait_res == WAIT_FAILED ? GetLastError() : wait_res);
		if(wait_res != WAIT_TIMEOUT) //dont free if thread is still running
			VirtualFreeEx(hProc, pMem, 0, MEM_RELEASE);
		CloseHandle(hThread);
		return false;

	}

	VirtualFreeEx(hProc, pMem, 0, MEM_RELEASE);

	DWORD exit_code = 0;
	if (!GetExitCodeThread(hThread, &exit_code)) {
		SetLastErrorString("GetExitCodeThread failed 0x%X", wait_res == WAIT_FAILED ? GetLastError() : wait_res);
		CloseHandle(hThread);
		return false;
	}

	CloseHandle(hThread);

	if (exit_code == 0) {
		SetLastErrorString("LoadLibraryW failed in remote process");
		return false;
	}

	return true;
}

