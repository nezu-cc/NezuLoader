#include "pch.h"
#include "NezuVacInterface.h"

DWORD NezuVac::FindStatusSection(HANDLE hProc, HMODULE mod) {
	IMAGE_DOS_HEADER dos_header = { 0 };
	if (!ReadProcessMemory(hProc, (void*)mod, &dos_header, sizeof(dos_header), NULL)) {
		return 0;//the only thing that can fail here is ReadProcessMemory so i don't bother with error logging 
	}

	DWORD nt_headers_address = (DWORD)mod + dos_header.e_lfanew;
	IMAGE_NT_HEADERS nt_headers = { 0 };
	if (!ReadProcessMemory(hProc, (void*)nt_headers_address, &nt_headers, sizeof(nt_headers), NULL)) {
		return 0;
	}

	DWORD nr_section_headers = nt_headers.FileHeader.NumberOfSections;
	IMAGE_SECTION_HEADER* section_headers = new IMAGE_SECTION_HEADER[nr_section_headers];
	DWORD section_headers_address = nt_headers_address + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader) + nt_headers.FileHeader.SizeOfOptionalHeader;
	if (!ReadProcessMemory(hProc, (void*)section_headers_address, section_headers, nr_section_headers * sizeof(IMAGE_SECTION_HEADER), NULL)) {
		delete[] section_headers;
		return 0;
	}

	for (DWORD i = 0; i < nr_section_headers; i++) {
		IMAGE_SECTION_HEADER* section = &section_headers[i];
		CHAR name[IMAGE_SIZEOF_SHORT_NAME + 1];
		memcpy(name, &section->Name, IMAGE_SIZEOF_SHORT_NAME);//if name is 8 characters it won't be null terminated, copy to a new buffer to avoid that
		name[IMAGE_SIZEOF_SHORT_NAME] = '\0';//add null termination
		if (!strcmp(name, NEZUVACSTATUS_SECTION_NAME)) {
			return (DWORD)mod + section->VirtualAddress;
		}
	}

	delete[] section_headers;
	return 0;
}

NezuVac::NeuzStatus NezuVac::GetStatus(HANDLE hProc, DWORD section_addr) {
	NezuVac::NeuzStatus status;
	ZeroMemory(&status, sizeof(status));
	ReadProcessMemory(hProc, (void*)section_addr, &status, sizeof(status), NULL);
	return status;
}

