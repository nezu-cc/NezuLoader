#pragma once

class KeyValues;
class CUiComponent_PartyList;
class CUiComponent_PartyBrowser;

typedef void* (*f_CreateSession)();
typedef void* (*f_CreateSessionUI)();
typedef void(__thiscall* f_InitKeyValues)(KeyValues* thisptr, const char* name);
typedef void* (*f_JoinLobby)();

namespace M {

	extern IDirect3DDevice9* d3d9Device;
	extern HWND Window;
	extern f_CreateSession CreateSession;
	extern f_CreateSessionUI CreateSessionUI;
	extern f_InitKeyValues InitKeyValues;
	//extern void* HasPrimeFunc;
	extern void* ChangeGameUIStateFunc;
	extern void* demoFileEndReached;
	extern const char** MatchmakingUIStatusString;
	extern void* KeyValues_ReadString;
	extern CUiComponent_PartyList** UiComponent_PartyList;
	extern CUiComponent_PartyBrowser** UiComponent_PartyBrowser;
	extern void* UiComponent_PartyBrowser_HasBeenInvited;
	extern f_JoinLobby JoinLobby;
	extern uint64_t* JoinLobbyId;
	extern void* StopMatchmakingFunc;
	extern DWORD* CurrentMatchState;
	extern void* CLobbyMenuSingleton_ReadyUpForMatch;
	extern void* CParticleCollection_InitializeNewParticles;
	extern void* CParticleCollection_Simulate;
	extern void* CVoiceStatus_SetPlayerBlockedState;
	extern void* ProcessConnectionless;
	extern void* NET_SendLong;
	extern int HOST_VERSION;
	extern void* Calls_SendToImpl;
	extern void* CNetChan_SendSubChannelData;
	extern void* GetEconGameAccountClient;
	extern void* IsPrime;
	extern void* fps_max_cheats;

	void FindAll();
};

