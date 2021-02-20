#pragma once
#include "NezuVacStatus.h"

namespace NezuVac {
	DWORD FindStatusSection(HANDLE hProc, HMODULE mod);
	NezuVac::NeuzStatus GetStatus(HANDLE hProc, DWORD section_addr = NULL);
};