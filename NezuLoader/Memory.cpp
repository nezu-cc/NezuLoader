#include "pch.h"
#include "Memory.h"

namespace M {

	IDirect3DDevice9* d3d9Device;
	HWND Window;
	f_CreateSession CreateSession;
	f_CreateSessionUI CreateSessionUI;
	f_InitKeyValues InitKeyValues;
	//void* HasPrimeFunc;
	void* ChangeGameUIStateFunc;
	void* demoFileEndReached;
	const char** MatchmakingUIStatusString;
	void* KeyValues_ReadString;
	CUiComponent_PartyList** UiComponent_PartyList;
	CUiComponent_PartyBrowser** UiComponent_PartyBrowser;
	void* UiComponent_PartyBrowser_HasBeenInvited;
	f_JoinLobby JoinLobby;
	uint64_t* JoinLobbyId;
	void* StopMatchmakingFunc;
	DWORD* CurrentMatchState;
	void* CLobbyMenuSingleton_ReadyUpForMatch;
	void* CParticleCollection_InitializeNewParticles;
	void* CParticleCollection_Simulate;
	void* CVoiceStatus_SetPlayerBlockedState;
	void* ProcessConnectionless;
	void* NET_SendLong;
	int HOST_VERSION;
	void* Calls_SendToImpl;
	void* CNetChan_SendSubChannelData;

	void FindAll() {

		d3d9Device = **(IDirect3DDevice9***)(FindPattern("shaderapidx9.dll", "A1 ? ? ? ? 50 8B 08 FF 51 0C") + 1);
		CreateSession = (f_CreateSession)relativeToAbsolute(FindPattern("client.dll", "E8 ? ? ? ? 83 EC 14 E8") + 1);
		CreateSessionUI = (f_CreateSessionUI)relativeToAbsolute(FindPattern("client.dll", "E8 ? ? ? ? 8B 50 08") + 1);
		InitKeyValues = (f_InitKeyValues)relativeToAbsolute(FindPattern("client.dll", "E8 ? ? ? ? 5F 89 06") + 1);
		//HasPrimeFunc = (void*)relativeToAbsolute(FindPattern("client.dll", "E8 ? ? ? ? 88 44 24 13 84 C0") + 1);
		ChangeGameUIStateFunc = (void*)relativeToAbsolute(FindPattern("client.dll", "E8 ? ? ? ? F3 0F 10 4D ? C6 86") + 1);
		demoFileEndReached = (void*)FindPattern("client.dll", "8B C8 85 C9 74 1F 80 79 10");
		MatchmakingUIStatusString = *(const char***)(FindPattern("client.dll", "8B F1 B9 ? ? ? ? 6A 00") + 3);
		KeyValues_ReadString = (void*)FindPattern("client.dll", "55 8B EC 83 E4 C0 81 EC ? ? ? ? 53 8B 5D 08");
		UiComponent_PartyList = *(CUiComponent_PartyList***)(FindPattern("client.dll", "EB 35 8D 04 92") + 0x2 + 0x35 + 0x1);// (jmp opcode + operand) + jump distance + mov opcode
		UiComponent_PartyBrowser = *(CUiComponent_PartyBrowser***)(FindPattern("client.dll", "83 E4 F8 8B 0D ? ? ? ? 85 C9") + 5);
		UiComponent_PartyBrowser_HasBeenInvited = (void*)relativeToAbsolute(FindPattern("client.dll", "E8 ? ? ? ? 85 C0 74 37 85 F6") + 1);
		JoinLobby = (f_JoinLobby)FindPattern("client.dll", "55 8B EC 83 E4 F8 8B 0D ? ? ? ? 85 C9");
		JoinLobbyId = (uint64_t*)*(DWORD*)(FindPattern("client.dll", "A1 ? ? ? ? 0B 05 ? ? ? ? 74 13") + 1);
		StopMatchmakingFunc = (void*)FindPattern("client.dll", "8B 41 08 83 C1 08 68");
		CurrentMatchState = *(DWORD**)(FindPattern("client.dll", "A1 ? ? ? ? 8B 4D 08 D1 E8") + 1);
		CLobbyMenuSingleton_ReadyUpForMatch = (void*)FindPattern("client.dll", "55 8B EC 53 8A 5D 08 56 8B F1 84 DB 74 0B");
		CParticleCollection_InitializeNewParticles = (void*)relativeToAbsolute(FindPattern("client.dll", "E8 ? ? ? ? 8B 45 F8 8B 55 F4") + 1);
		CParticleCollection_Simulate = (void*)relativeToAbsolute(FindPattern("client.dll", "E8 ? ? ? ? 8B 0E 83 C1 10") + 1);
		CVoiceStatus_SetPlayerBlockedState = (void*)relativeToAbsolute(FindPattern("client.dll", "E8 ? ? ? ? A1 ? ? ? ? 8B 4C 24 0C") + 1);
		ProcessConnectionless = (void*)FindPattern("engine.dll", "55 8B EC 83 E4 F0 81 EC ? ? ? ? 56 57 8B F9 8B 4D 08");
		NET_SendLong = (void*)relativeToAbsolute(FindPattern("engine.dll", "E8 ? ? ? ? 83 C4 08 8B F0 83 BD") + 1);
		HOST_VERSION = *(int*)(FindPattern("engine.dll", "A1 ? ? ? ? 50 FF 77 08") + 1);
		Calls_SendToImpl = (void*)relativeToAbsolute(FindPattern("engine.dll", "E8 ? ? ? ? 83 C4 08 FF 45 F8") + 1);
		CNetChan_SendSubChannelData = (void*)relativeToAbsolute(FindPattern("engine.dll", "E8 ? ? ? ? 84 C0 74 05 80 4C 24") + 1);

	}

}
