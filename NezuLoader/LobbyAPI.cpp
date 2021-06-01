#include "pch.h"
#include "Hooks.h"

f_GenericThisCall H::oStopMatchmaking;

void* __fastcall H::Hooked_StopMatchmaking(void* thisPtr, void* edx) {
	Cfg::c.lobby.auto_queue.enabled = false;
	return oStopMatchmaking(thisPtr);
}