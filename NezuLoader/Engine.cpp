#include "pch.h"
#include "Hooks.h"

f_GetDemoPlaybackParameters H::oGetDemoPlaybackParameters;

//pasted from Osiris
CDemoPlaybackParameters_t* __fastcall H::Hooked_GetDemoPlaybackParameters(IEngine* thisPtr, void* edx) {
	CDemoPlaybackParameters_t* params = oGetDemoPlaybackParameters(thisPtr);
	if (params && Cfg::c.misc.ow_reveal && _ReturnAddress() != M::demoFileEndReached) {
		static CDemoPlaybackParameters_t customParams;
		customParams = *params;
		customParams.m_bAnonymousPlayerIdentity = false;
		return &customParams;
	}
	return params;
}