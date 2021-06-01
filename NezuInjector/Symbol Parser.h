#pragma once

#include "pch.h"

#include "Download Manager.h"

class SYMBOL_PARSER {
	HANDLE m_hProcess;

	HANDLE			m_hPdbFile;
	std::wstring	m_szPdbPath;
	DWORD64			m_SymbolTable;

	std::wstring m_szModulePath;

	bool m_Initialized;
	bool m_Ready;

	DownloadManager m_DlMgr;
	HANDLE	m_hInterruptEvent;
	bool	m_bInterruptEvent;
	float	m_fProgress;

	bool VerifyExistingPdb(const GUID & guid);

public:

	SYMBOL_PARSER();
	~SYMBOL_PARSER();

	BOOL Initialize(const std::wstring szModulePath, const std::wstring path, std::wstring * pdb_path_out, bool Redownload, bool WaitForConnection = false);
	BOOL GetSymbolAddress(const char * szSymbolName, DWORD & RvaOut);

	void Interrupt();
	float GetDownloadProgress();

	std::string m_szError;
};

struct PdbInfo {
	DWORD	Signature;
	GUID	Guid;
	DWORD	Age;
	char	PdbFileName[1];
};

//Thanks mambda
//https://bitbucket.org/mambda/pdb-parser/src/master/
struct PDBHeader7 {
	char signature[0x20];
	int page_size;
	int allocation_table_pointer;
	int file_page_count;
	int root_stream_size;
	int reserved;
	int root_stream_page_number_list_number;
};

struct RootStream7 {
	int num_streams;
	int stream_sizes[1]; //num_streams
};

struct GUID_StreamData {
	int ver;
	int date;
	int age;
	GUID guid;
};

extern SYMBOL_PARSER sym_ntdll_native;
extern std::shared_future<BOOL> sym_ntdll_native_ret;
