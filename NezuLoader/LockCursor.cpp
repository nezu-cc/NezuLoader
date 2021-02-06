#include "pch.h"
#include "Hooks.h"

f_LockCursor H::oHooked_LockCursor;

void __fastcall H::Hooked_LockCursor(void* thisPtr, void* edx) {
	if (Menu::open) GetVFunc<void(__thiscall*)(void*)>(thisPtr, 66)(thisPtr); //UnlockCursor
	else oHooked_LockCursor(thisPtr);
}