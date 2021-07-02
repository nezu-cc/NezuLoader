#pragma once

typedef HRESULT(_stdcall* f_EndScene)(IDirect3DDevice9* pDevice);
typedef HRESULT(_stdcall* f_Present)(IDirect3DDevice9* pDevice, const RECT* src, const RECT* dest, HWND wnd_override, const RGNDATA* dirty_region);
typedef HRESULT(_stdcall* f_Reset)(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
typedef void(__thiscall* f_LockCursor)(void* thisPtr);
typedef LRESULT(__stdcall* f_WndProc)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef LONG(WINAPI* f_GetWindowLongW)(HWND hWnd, int nIndex);
typedef LONG(WINAPI* f_SetWindowLongW)(HWND hWnd, int nIndex, LONG dwNewLong);
typedef void(__thiscall* f_ChangeGameUIState)(CGameUI* thisPtr, CSGOGameUIState_t nNewState);
typedef CDemoPlaybackParameters_t* (__thiscall* f_GetDemoPlaybackParameters)(IEngine* thisPtr);
typedef void(__thiscall* f_FrameStageNotify)(void*, ClientFrameStage_t);
typedef void* (__thiscall* f_GenericThisCall)(void*);
typedef void(__thiscall* f_ReadyUpForMatch)(void*, bool, int, int);
typedef void(__thiscall* f_InitializeNewParticles)(CParticleCollection* thisPtr, int nFirstParticle, int nParticleCount, uint32_t nInittedMask, bool bApplyingParentKillList);
typedef void(__thiscall* f_Simulate)(CParticleCollection* thisPtr/*, float dt*/);
typedef void(__thiscall* f_SetPlayerBlockedState)(CVoiceStatus* thisPtr, int iPlayer, bool bSquelch, bool bAutomatic);
typedef int(__fastcall* f_NET_SendLong)(void* netchannel, DWORD* socket, int nBytesLeft, int nMaxRoutableSize);
typedef bool(__thiscall* f_ProcessConnectionless)(void* thisPtr, netpacket_t* packet);
typedef bool(__thiscall* f_CNET_SendSubChannelData)(void* thisPtr, void* buf);
typedef CEconGameAccountClient*(__fastcall* f_GetEconGameAccountClient)(void* thisPtr);
typedef bool(*f_IsPrime)();
typedef bool(__thiscall* f_SvCheatsGetBool)(void*);

//steam
typedef EGCResults(__thiscall* f_RetrieveMessage)(ISteamGameCoordinator* thisPtr, uint32_t* punMsgType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize);
typedef EGCResults(__thiscall* f_SendMessage)(ISteamGameCoordinator* thisPtr, uint32_t unMsgType, const void* pubData, uint32_t cubData);

namespace H {
	extern f_Present* fake_Present;
	extern f_Reset* fake_Reset;
	//extern f_EndScene oHooked_EndScene;
	extern f_Present oHookedPresent;
	extern f_Reset oHooked_Reset;
	extern f_LockCursor oHooked_LockCursor;
	extern f_WndProc oHooked_WndProc;
	extern f_RetrieveMessage oRetrieveMessage;
	extern f_SendMessage oSendMessage;
	extern f_ChangeGameUIState oChangeGameUIState;
	extern f_GetDemoPlaybackParameters oGetDemoPlaybackParameters;
	extern f_FrameStageNotify oFrameStageNotify;
	extern f_GenericThisCall oStopMatchmaking;
	extern f_ReadyUpForMatch oReadyUpForMatch;
	extern f_InitializeNewParticles oCParticleCollection_InitializeNewParticles;
	extern f_Simulate oCParticleCollection_Simulate;
	extern f_SetPlayerBlockedState oCVoiceStatus_SetPlayerBlockedState;
	extern f_NET_SendLong oNET_SendLong;
	extern f_ProcessConnectionless oProcessConnectionless;
	extern f_CNET_SendSubChannelData oCNET_SendSubChannelData;
	extern f_GetEconGameAccountClient oGetEconGameAccountClient;
	extern f_IsPrime oIsPrime;
	extern f_SvCheatsGetBool oSvCheatsGetBool;

	void ApplyHooks();
	void Hook_NET_SendLong(bool on);
	void RemoveHooks();

	//HRESULT __stdcall Hooked_EndScene(IDirect3DDevice9* pDevice);
	HRESULT __stdcall Hooked_Present(IDirect3DDevice9* pDevice, const RECT* src, const RECT* dest, HWND wnd_override, const RGNDATA* dirty_region);
	HRESULT __stdcall Hooked_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
	LRESULT __stdcall Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void __fastcall Hooked_LockCursor(void* thisPtr, void* edx);
	EGCResults __fastcall Hooked_SendMessage(ISteamGameCoordinator* thisPtr, void* edx, uint32_t unMsgType, const void* pubData, uint32_t cubData);
	EGCResults __fastcall Hooked_RetrieveMessage(ISteamGameCoordinator* thisPtr, void* edx, uint32_t* punMsgType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize);
	void __fastcall Hooked_ChangeGameUIState(CGameUI* thisptr, void* edx, CSGOGameUIState_t nNewState);
	void __fastcall Hooked_FrameStageNotify(void* thisPtr, void* edx, ClientFrameStage_t curStage);
	CDemoPlaybackParameters_t* __fastcall Hooked_GetDemoPlaybackParameters(IEngine* thisPtr, void* edx);
	void* __fastcall Hooked_StopMatchmaking(void* thisPtr, void* edx);
	void __fastcall Hooked_ReadyUpForMatch(void* thisPtr, void* edx, bool shouldShow, int playersReadyCount, int numTotalClientsInReservation);
	void __fastcall Hooked_CParticleCollection_InitializeNewParticles(CParticleCollection* thisPtr, void* edx, int nFirstParticle, int nParticleCount, uint32_t nInittedMask, bool bApplyingParentKillList);
	void __fastcall Hooked_CParticleCollection_Simulate(CParticleCollection* thisPtr, void* edx/*, float dt*/);
	void __fastcall Hooked_CVoiceStatus_SetPlayerBlockedState(CVoiceStatus* thisPtr, void* edx, int iPlayer, bool bSquelch, bool bAutomatic);
	int __fastcall Hooked_NET_SendLong(void* netchannel, DWORD* socket, int nBytesLeft, int nMaxRoutableSize);
	bool __fastcall Hooked_ProcessConnectionless(void* thisPtr, void* edx, netpacket_t* pPacket);
	bool __fastcall Hooked_CNET_SendSubChannelData(void* ecx, void* edx, void* thisPtr);
	CEconGameAccountClient* __fastcall Hooked_GetEconGameAccountClient(void* thisPtr, void* edx);
	bool Hooked_IsPrime();
	bool __fastcall Hooked_SvCheatsGetBool(void* thisPtr, void* edx);

};

LPVOID MH_HookVtbl(LPVOID object, DWORD index, LPVOID funct);
LPVOID MH_Hook(LPVOID func, LPVOID funct);
LPVOID MH_HookInstant(LPVOID func, LPVOID funct);
LPVOID HookIAT(LPVOID mod, LPCSTR mod_name, LPCSTR func_name, LPVOID funct);