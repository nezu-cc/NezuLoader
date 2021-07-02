#include "pch.h"
#include "Hooks.h"

f_SendNetMessage H::oSendNetMessage;

bool __fastcall H::Hooked_SendNetMessage(void* netchann, void* edx, NetworkMessage* msg, bool bForceReliable, bool bVoice) {

	if (!netchann)
		return false;

	if ((msg->getType() == CLC_Message::clc_FileCRCCheck) && Cfg::c.misc.sv_pure_bypass) {
		return true;
	}

	return oSendNetMessage(netchann, msg, bForceReliable, bVoice);
}