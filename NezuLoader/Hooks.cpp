#include "pch.h"
#include "Hooks.h"
#include "MinHook.h"

void H::ApplyHooks() {
	//using namespace H;
	if (MH_Initialize() != MH_OK)
		return;
	
	oHookedPresent = (f_Present)MH_HookVtbl(M::d3d9Device, 17, &Hooked_Present); if (!oHookedPresent) printf("Failed to hook Present\n");
	oHooked_Reset = (f_Reset)MH_HookVtbl(M::d3d9Device, 16, &Hooked_Reset); if (!oHooked_Reset) printf("Failed to hook Reset\n");
	oHooked_LockCursor = (f_LockCursor)MH_HookVtbl(I::Surface, 67, &Hooked_LockCursor); if (!oHooked_LockCursor) printf("Failed to hook LockCursor\n");
	//oHasPrime = (f_HasPrime)MH_Hook(M::HasPrimeFunc, &Hooked_HasPrime); if (!oHasPrime) printf("Failed to hook HasPrime\n");
	oChangeGameUIState = (f_ChangeGameUIState)MH_Hook(M::ChangeGameUIStateFunc, &Hooked_ChangeGameUIState); if (!oChangeGameUIState) printf("Failed to hook ChangeGameUIState\n");
	oGetDemoPlaybackParameters = (f_GetDemoPlaybackParameters)MH_HookVtbl(I::Engine, 218, &Hooked_GetDemoPlaybackParameters); if (!oGetDemoPlaybackParameters) printf("Failed to hook GetDemoPlaybackParameters\n");
	oFrameStageNotify = (f_FrameStageNotify)MH_HookVtbl(I::Client, 37, &Hooked_FrameStageNotify); if (!oFrameStageNotify) printf("Failed to hook FrameStageNotify\n");
	oStopMatchmaking = (f_GenericThisCall)MH_Hook(M::StopMatchmakingFunc, &Hooked_StopMatchmaking); if (!oStopMatchmaking) printf("Failed to hook StopMatchmaking\n");
	oReadyUpForMatch = (f_ReadyUpForMatch)MH_Hook(M::CLobbyMenuSingleton_ReadyUpForMatch, &Hooked_ReadyUpForMatch); if (!oReadyUpForMatch) printf("Failed to hook ReadyUpForMatch\n");
	//oCParticleCollection_InitializeNewParticles = (f_InitializeNewParticles)MH_Hook(M::CParticleCollection_InitializeNewParticles, &Hooked_CParticleCollection_InitializeNewParticles);
	//if (!oCParticleCollection_InitializeNewParticles) printf("Failed to hook CParticleCollection_InitializeNewParticles\n");
	oCParticleCollection_Simulate = (f_Simulate)MH_Hook(M::CParticleCollection_Simulate, &Hooked_CParticleCollection_Simulate);
	if (!oCParticleCollection_Simulate) printf("Failed to hook CParticleCollection_Simulate\n");
	oCVoiceStatus_SetPlayerBlockedState = (f_SetPlayerBlockedState)MH_Hook(M::CVoiceStatus_SetPlayerBlockedState, &Hooked_CVoiceStatus_SetPlayerBlockedState);
	if (!oCVoiceStatus_SetPlayerBlockedState) printf("Failed to hook CVoiceStatus_SetPlayerBlockedState\n");
	oProcessConnectionless = (f_ProcessConnectionless)MH_Hook(M::ProcessConnectionless, &Hooked_ProcessConnectionless); if (!oProcessConnectionless) printf("Failed to hook ProcessConnectionless\n");
	oCNET_SendSubChannelData = (f_CNET_SendSubChannelData)MH_Hook(M::CNetChan_SendSubChannelData, &Hooked_CNET_SendSubChannelData); 
	if (!oCNET_SendSubChannelData) printf("Failed to hook CNET_SendSubChannelData\n");
	oGetEconGameAccountClient = (f_GetEconGameAccountClient)MH_Hook(M::GetEconGameAccountClient, &Hooked_GetEconGameAccountClient);
	if (!oGetEconGameAccountClient) printf("Failed to hook GetEconGameAccountClient\n");
	oIsPrime = (f_IsPrime)MH_Hook(M::IsPrime, &Hooked_IsPrime); if (!oIsPrime) printf("Failed to hook IsPrime\n");

	//steam
	oRetrieveMessage = (f_RetrieveMessage)MH_HookVtbl(I::SteamGameCoordinator, 2, &Hooked_RetrieveMessage); if (!oRetrieveMessage) printf("Failed to hook RetrieveMessage\n");
#ifdef _DEBUG
	oSendMessage = (f_SendMessage)MH_HookVtbl(I::SteamGameCoordinator, 0, &Hooked_SendMessage); if (!oSendMessage) printf("Failed to hook SendMessage\n");
#endif

	while (!(M::Window = FindWindowA("Valve001", nullptr)))
		Sleep(50);
	void* wnd_proc = IsWindowUnicode(M::Window) ? (void*)GetWindowLongPtrW(M::Window, GWLP_WNDPROC) : (void*)GetWindowLongPtrA(M::Window, GWLP_WNDPROC);
	oHooked_WndProc = (f_WndProc)MH_Hook(wnd_proc, Hooked_WndProc); if (!oHooked_WndProc) printf("Failed to hook WndProc\n");

	MH_EnableHook(MH_ALL_HOOKS);

	oNET_SendLong = (f_NET_SendLong)MH_Hook(M::NET_SendLong, &Hooked_NET_SendLong);
	if (!oNET_SendLong) 
		printf("Failed to hook NET_SendLong\n");
}

void H::Hook_NET_SendLong(bool on) {
	if (on)
		MH_EnableHook(M::NET_SendLong);
	else
		MH_DisableHook(M::NET_SendLong);
}

//debug only
void H::RemoveHooks() {
	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();
}

LPVOID MH_HookVtbl(LPVOID object, DWORD index, LPVOID funct) {
	LPVOID orig = NULL;
	MH_STATUS status = MH_CreateHook((*(void***)object)[index], funct, &orig);
	if (status != MH_OK) {
		printf("[nezu] MH_CreateHook failed: %s\n", MH_StatusToString(status));
		return NULL;
	}
	return orig;
}

LPVOID MH_Hook(LPVOID func, LPVOID funct) {
	LPVOID orig = NULL;
	MH_STATUS status = MH_CreateHook(func, funct, &orig);
	if (status != MH_OK) {
		printf("[nezu] MH_CreateHook failed: %s\n", MH_StatusToString(status));
		return NULL;
	}
	return orig;
}

LPVOID MH_HookInstant(LPVOID func, LPVOID funct) {
	LPVOID orig = MH_Hook(func, funct);
	MH_EnableHook(func);
	return orig;
}

LPVOID HookIAT(LPVOID mod, LPCSTR mod_name, LPCSTR func_name, LPVOID funct) {
	IMAGE_NT_HEADERS* ntHdr = (IMAGE_NT_HEADERS*)((DWORD)mod + ((IMAGE_DOS_HEADER*)mod)->e_lfanew);
	IMAGE_OPTIONAL_HEADER* optHdr = &ntHdr->OptionalHeader;

	if (optHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
		auto* pImportDescr = (IMAGE_IMPORT_DESCRIPTOR*)((DWORD)mod + optHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		while (pImportDescr->Name) {
			char* szMod = (char*)((DWORD)mod + pImportDescr->Name);
			if (_strcmpi(szMod, mod_name) == 0) {
				DWORD* pThunkRef = (DWORD*)((DWORD)mod + pImportDescr->OriginalFirstThunk);
				DWORD* pFuncRef = (DWORD*)((DWORD)mod + pImportDescr->FirstThunk);

				if (!pThunkRef)
					pThunkRef = pFuncRef;

				for (; *pThunkRef; ++pThunkRef, ++pFuncRef) {
					if (!IMAGE_SNAP_BY_ORDINAL(*pThunkRef)) {
						IMAGE_IMPORT_BY_NAME* pImport = (IMAGE_IMPORT_BY_NAME*)((DWORD)mod + *pThunkRef);
						if (_strcmpi(pImport->Name, func_name) != 0)
							continue;

						//our module will already be writable
						//if you are pasting this make sure to add VirtualProtect
						DWORD orig = *pFuncRef;
						*pFuncRef = (DWORD)funct;
						return (LPVOID)orig;
					}
				}
			}
			++pImportDescr;
		}
	}
	return NULL;
}