#include "pch.h"

#include "Symbol Parser.h"

SYMBOL_PARSER::SYMBOL_PARSER() {
	m_Initialized	= false;
	m_Ready			= false;
	m_SymbolTable	= 0;
	m_hPdbFile		= nullptr;
	m_hProcess		= nullptr;

	m_bInterruptEvent	= false;
	m_hInterruptEvent	= CreateEvent(nullptr, FALSE, FALSE, nullptr);
	m_fProgress			= 0.0f;
	m_szError			= "";
}

SYMBOL_PARSER::~SYMBOL_PARSER() {
	if (m_hInterruptEvent)
		CloseHandle(m_hInterruptEvent);
	if (m_Initialized) {
		if (m_SymbolTable)
			SymUnloadModule64(m_hProcess, m_SymbolTable);

		SymCleanup(m_hProcess);
	}
	if (m_hProcess)
		CloseHandle(m_hProcess);
	if (m_hPdbFile)
		CloseHandle(m_hPdbFile);
}

bool SYMBOL_PARSER::VerifyExistingPdb(const GUID & guid) {
	std::ifstream f(m_szPdbPath.c_str(), std::ios::binary | std::ios::ate);
	if (f.bad())
		return false;

	size_t size_on_disk = static_cast<size_t>(f.tellg());
	if (!size_on_disk) {
		f.close();
		return false;
	}

	char * pdb_raw = new char[size_on_disk];
	if (!pdb_raw) {
		f.close();
		return false;
	}

	f.seekg(std::ios::beg);
	f.read(pdb_raw, size_on_disk);
	f.close();

	if (size_on_disk < sizeof(PDBHeader7)) {
		delete[] pdb_raw;
		return false;
	}

	auto * pPDBHeader = (PDBHeader7*)pdb_raw;

	if (memcmp(pPDBHeader->signature, "Microsoft C/C++ MSF 7.00\r\n\x1A""DS\0\0\0", sizeof(PDBHeader7::signature))) {
		delete[] pdb_raw;
		return false;
	}

	if (size_on_disk < (size_t)pPDBHeader->page_size * pPDBHeader->file_page_count) {
		delete[] pdb_raw;
		return false;
	}

	int		* pRootPageNumber	= (int*)(pdb_raw + (size_t)pPDBHeader->root_stream_page_number_list_number * pPDBHeader->page_size);
	auto	* pRootStream		= (RootStream7*)(pdb_raw + (size_t)(*pRootPageNumber) * pPDBHeader->page_size);

	std::map<int, std::vector<int>> streams;
	int current_page_number = 0;
	
	for (int i = 0; i != pRootStream->num_streams; ++i) {
		int current_size = pRootStream->stream_sizes[i] == 0xFFFFFFFF ? 0 : pRootStream->stream_sizes[i];

		int current_page_count = current_size / pPDBHeader->page_size;
		if (current_size % pPDBHeader->page_size)
			++current_page_count;

		std::vector<int> numbers;

		for (int j = 0; j != current_page_count; ++j, ++current_page_number)
			numbers.push_back(pRootStream->stream_sizes[pRootStream->num_streams + current_page_number]);

		streams.insert({ i, numbers });
	}

	auto pdb_info_page_index = streams.at(1).at(0);

	auto * stream_data = (GUID_StreamData*)(pdb_raw + (size_t)(pdb_info_page_index) * pPDBHeader->page_size);

	int guid_eq = memcmp(&stream_data->guid, &guid, sizeof(GUID));

	delete[] pdb_raw;
	
	return (guid_eq == 0);
}

BOOL SYMBOL_PARSER::Initialize(const std::wstring szModulePath, const std::wstring path, std::wstring * pdb_path_out, bool Redownload, bool WaitForConnection) {
	if (m_Ready) {
		//return SYMBOL_ERR_ALREADY_INITIALIZED;
		m_szError = "";
		return TRUE;
	}

	m_bInterruptEvent = false;

	std::ifstream File(szModulePath.c_str(), std::ios::binary | std::ios::ate);
	if (!File.good()) {
		m_szError = "SYMBOL_PARSER::SYMBOL_ERR_CANT_OPEN_MODULE";
		return FALSE;
	}

	auto FileSize = File.tellg();
	if (!FileSize) {
		m_szError = "SYMBOL_PARSER::SYMBOL_ERR_FILE_SIZE_IS_NULL";
		return FALSE;
	}

	BYTE * pRawData = new BYTE[static_cast<size_t>(FileSize)];
	if (!pRawData) {
		m_szError = "SYMBOL_PARSER::SYMBOL_ERR_CANT_ALLOC_MEMORY_NEW";
		delete[] pRawData;
		File.close();
		return FALSE;
	}

	File.seekg(0, std::ios::beg);
	File.read((char*)pRawData, FileSize);
	File.close();

	IMAGE_DOS_HEADER	* pDos	= (IMAGE_DOS_HEADER*)pRawData;
	IMAGE_NT_HEADERS	* pNT	= (IMAGE_NT_HEADERS*)(pRawData + pDos->e_lfanew);
	IMAGE_FILE_HEADER	* pFile = &pNT->FileHeader;

	IMAGE_OPTIONAL_HEADER * pOpt = &pNT->OptionalHeader;

	if (pFile->Machine != IMAGE_FILE_MACHINE_I386) {
		m_szError = "SYMBOL_PARSER::SYMBOL_ERR_INVALID_FILE_ARCHITECTURE";
		delete[] pRawData;
		return FALSE;
	}

	DWORD ImageSize = pOpt->SizeOfImage;
	BYTE * pLocalImageBase = (BYTE*)(VirtualAlloc(nullptr, ImageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
	if (!pLocalImageBase) {
		m_szError = "SYMBOL_PARSER::SYMBOL_ERR_CANT_ALLOC_MEMORY";
		delete[] pRawData;
		return FALSE;
	}

	memcpy(pLocalImageBase, pRawData, pOpt->SizeOfHeaders);

	auto * pCurrentSectionHeader = IMAGE_FIRST_SECTION(pNT);
	for (UINT i = 0; i != pFile->NumberOfSections; ++i, ++pCurrentSectionHeader) {
		if (pCurrentSectionHeader->SizeOfRawData)
			memcpy(pLocalImageBase + pCurrentSectionHeader->VirtualAddress, pRawData + pCurrentSectionHeader->PointerToRawData, pCurrentSectionHeader->SizeOfRawData);
	}

	IMAGE_DATA_DIRECTORY * pDataDir = &pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];

	IMAGE_DEBUG_DIRECTORY * pDebugDir = (IMAGE_DEBUG_DIRECTORY*)(pLocalImageBase + pDataDir->VirtualAddress);

	if (!pDataDir->Size || IMAGE_DEBUG_TYPE_CODEVIEW != pDebugDir->Type) {
		m_szError = "SYMBOL_PARSER::SYMBOL_ERR_NO_PDB_DEBUG_DATA1";
		VirtualFree(pLocalImageBase, 0, MEM_RELEASE);
		delete[] pRawData;
		return FALSE;
	}

	PdbInfo * pdb_info = (PdbInfo*)(pLocalImageBase + pDebugDir->AddressOfRawData);
	m_szError = "SYMBOL_PARSER::SYMBOL_ERR_NO_PDB_DEBUG_DATA2";
	if (pdb_info->Signature != 0x53445352) {
		VirtualFree(pLocalImageBase, 0, MEM_RELEASE);
		delete[] pRawData;
		return FALSE;
	}
	
	m_szPdbPath = path;
	
	if (m_szPdbPath[m_szPdbPath.length() - 1] != '\\')
		m_szPdbPath += '\\';

	if (!CreateDirectoryW(m_szPdbPath.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
		m_szError = "SYMBOL_PARSER::SYMBOL_ERR_PATH_DOESNT_EXIST";
		return FALSE;
	}

	std::wstring szPdbFileName(UTF8WCHAR(pdb_info->PdbFileName));
	m_szPdbPath += szPdbFileName;
		
	DWORD Filesize = 0;
	WIN32_FILE_ATTRIBUTE_DATA file_attr_data{ 0 };
	if (GetFileAttributesExW(m_szPdbPath.c_str(), GetFileExInfoStandard, &file_attr_data)) {
		Filesize = file_attr_data.nFileSizeLow;
		if (!Redownload && !VerifyExistingPdb(pdb_info->Guid))
			Redownload = true;

		if (Redownload)
			DeleteFileW(m_szPdbPath.c_str());
	}	
	else
		Redownload = true;

	if (Redownload) {
		wchar_t w_GUID[100]{ 0 };
		if (!StringFromGUID2(pdb_info->Guid, w_GUID, 100)) {
			m_szError = "SYMBOL_PARSER::SYMBOL_ERR_CANT_CONVERT_PDB_GUID";
			VirtualFree(pLocalImageBase, 0, MEM_RELEASE);
			delete[] pRawData;
			return FALSE;
		}

		std::wstring guid_filtered;
		for (UINT i = 0; w_GUID[i]; ++i) {
			if ((w_GUID[i] >= '0' && w_GUID[i] <= '9') || (w_GUID[i] >= 'A' && w_GUID[i] <= 'F') || (w_GUID[i] >= 'a' && w_GUID[i] <= 'f'))
				guid_filtered += w_GUID[i];
		}

		std::wstring url = L"https://msdl.microsoft.com/download/symbols/";
		url += szPdbFileName;
		url += '/';
		url += guid_filtered;
		url += std::to_wstring(pdb_info->Age);
		url += '/';
		url += szPdbFileName;

		if (WaitForConnection) {
			while (InternetCheckConnectionW(L"https://msdl.microsoft.com", FLAG_ICC_FORCE_CONNECTION, NULL) == FALSE) {
				Sleep(25);
				if (m_bInterruptEvent) {
					m_szError = "SYMBOL_PARSER::SYMBOL_ERR_INTERRUPT";
					VirtualFree(pLocalImageBase, 0, MEM_RELEASE);
					delete[] pRawData;
					return FALSE;
				}
			}
		}

		wchar_t szCacheFile[MAX_PATH]{ 0 };

		m_DlMgr.SetInterruptEvent(m_hInterruptEvent);

		if (FAILED(URLDownloadToCacheFileW(nullptr, url.c_str(), szCacheFile, MAX_PATH , NULL, &m_DlMgr))) {
			VirtualFree(pLocalImageBase, 0, MEM_RELEASE);
			delete[] pRawData;
			m_szError = "SYMBOL_PARSER::SYMBOL_ERR_DOWNLOAD_FAILED";
			return FALSE;
		}

		if (!CopyFileW(szCacheFile, m_szPdbPath.c_str(), FALSE)) {
			m_szError = "SYMBOL_PARSER::SYMBOL_ERR_COPYFILE_FAILED";
			VirtualFree(pLocalImageBase, 0, MEM_RELEASE);
			delete[] pRawData;
			return FALSE;
		}
	}

	m_fProgress = 1.0f;

	VirtualFree(pLocalImageBase, 0, MEM_RELEASE);

	delete[] pRawData;

	if (!Filesize) {
		if (!GetFileAttributesExW(m_szPdbPath.c_str(), GetFileExInfoStandard, &file_attr_data)) {
			m_szError = "SYMBOL_PARSER::SYMBOL_ERR_CANT_ACCESS_PDB_FILE";
			return FALSE;
		}
		Filesize = file_attr_data.nFileSizeLow;
	}

	m_hPdbFile = CreateFileW(m_szPdbPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, NULL, nullptr);
	if (m_hPdbFile == INVALID_HANDLE_VALUE) {
		m_szError = "SYMBOL_PARSER::SYMBOL_ERR_CANT_OPEN_PDB_FILE";
		return FALSE;
	}

	//can't this just be GetCurrentProcess()? to lazy to check
	m_hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, GetCurrentProcessId());
	if (!m_hProcess) {
		m_szError = "SYMBOL_PARSER::SYMBOL_ERR_CANT_OPEN_PROCESS";
		CloseHandle(m_hPdbFile);
		return FALSE;
	}

	if (!SymInitializeW(m_hProcess, m_szPdbPath.c_str(), FALSE)) {
		m_szError = "SYMBOL_PARSER::SYMBOL_ERR_SYM_INIT_FAIL";
		CloseHandle(m_hProcess);
		CloseHandle(m_hPdbFile);
		return FALSE;
	}

	m_Initialized = true;

	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_AUTO_PUBLICS);

	m_SymbolTable = SymLoadModuleExW(m_hProcess, nullptr, m_szPdbPath.c_str(), nullptr, 0x10000000, Filesize, nullptr, NULL);

	if (!m_SymbolTable) {
		m_szError = "SYMBOL_PARSER::SYMBOL_ERR_SYM_LOAD_TABLE";
		SymCleanup(m_hProcess);
		CloseHandle(m_hProcess);
		CloseHandle(m_hPdbFile);
		return FALSE;
	}

	if (pdb_path_out)
		*pdb_path_out = m_szPdbPath;

	m_Ready = true;

	m_szError = "";
	return TRUE;
}

BOOL SYMBOL_PARSER::GetSymbolAddress(const char * szSymbolName, DWORD & RvaOut) {
	if (!m_Ready) {
		m_szError = "SYMBOL_PARSER::SYMBOL_ERR_NOT_INITIALIZED";
		return FALSE;
	}

	if (!szSymbolName) {
		m_szError = "SYMBOL_PARSER::SYMBOL_ERR_IVNALID_SYMBOL_NAME";
		return FALSE;
	}

	SYMBOL_INFO si{ 0 };
	si.SizeOfStruct = sizeof(SYMBOL_INFO);
	if (!SymFromName(m_hProcess, szSymbolName, &si)) {
		m_szError = "SYMBOL_PARSER::SYMBOL_ERR_SYMBOL_SEARCH_FAILED";
		return FALSE;
	}

	RvaOut = (DWORD)(si.Address - si.ModBase);

	m_szError = "";
	return TRUE;
}

void SYMBOL_PARSER::Interrupt() {
	m_bInterruptEvent = true;

	if (m_hInterruptEvent) {
		SetEvent(m_hInterruptEvent);
		CloseHandle(m_hInterruptEvent);
	}

	if (m_Initialized) {
		if (m_SymbolTable)
			SymUnloadModule64(m_hProcess, m_SymbolTable);

		SymCleanup(m_hProcess);
	}

	if (m_hProcess) {
		CloseHandle(m_hProcess);
	}

	if (m_hPdbFile) {
		CloseHandle(m_hPdbFile);
	}
	
	m_Initialized	= false;
	m_Ready			= false;
	m_SymbolTable	= 0;
	m_hPdbFile		= nullptr;
	m_hProcess		= nullptr;

	m_hInterruptEvent = nullptr;
}

float SYMBOL_PARSER::GetDownloadProgress() {
	if (m_fProgress == 1.0f)
		return m_fProgress;

	return m_DlMgr.GetDownloadProgress();
}

SYMBOL_PARSER sym_ntdll_native;
std::shared_future<BOOL> sym_ntdll_native_ret;