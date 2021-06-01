#include "pch.h"
#include "Hooks.h"

f_SetPlayerBlockedState H::oCVoiceStatus_SetPlayerBlockedState;

void __fastcall H::Hooked_CVoiceStatus_SetPlayerBlockedState(CVoiceStatus* thisPtr, void* edx, int iPlayer, bool bSquelch, bool bAutomatic) {

	if (bAutomatic && Cfg::c.misc.disable_auto_mute)
		return;

	oCVoiceStatus_SetPlayerBlockedState(thisPtr, iPlayer, bSquelch, bAutomatic);
}