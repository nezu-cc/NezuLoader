#include "pch.h"
#include "Hooks.h"
#include <intrin.h>

f_SvCheatsGetBool H::oSvCheatsGetBool;

bool __fastcall H::Hooked_SvCheatsGetBool(void* thisPtr, void* edx) {
	if (_ReturnAddress() == M::fps_max_cheats)
		return true;
	return oSvCheatsGetBool(thisPtr);
}