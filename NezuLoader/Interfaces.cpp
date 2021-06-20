#include "pch.h"
#include "Interfaces.h"
#include "isteamclient.h"

void* CaptureInterface(LPCSTR strModule, LPCSTR strInterface) {
	typedef void* (*CreateInterfaceFn)(const char* szName, int iReturn);
	return ((CreateInterfaceFn)GetProcAddress(GetModuleHandleA(strModule), "CreateInterface"))(strInterface, 0);
}

namespace I {
	ISteamFriends* SteamFriends;
	ISteamMatchmaking* SteamMatchmaking;
	ISteamUtils* SteamUtils;
	ISteamUser* SteamUser;
	ISteamGameCoordinator* SteamGameCoordinator;
	ISurface* Surface;
	IEngine* Engine;
	IClientEntityList* ClientEntityList;
	IInputSystem* InputSystem;
	MatchFramework* IMatchFramework;
	IGameEventManager* GameEventManager;
	IBaseClientDll* Client;
	IPanoramaUIEngine* PanoramaUIEngine;
	IClientState* ClientState;
	IMaterialSystem* MaterialSystem;
	ICVar* CVar;
	CGlobalVarsBase* Globals;

	void CaptureInterfaces() {

		I::Surface = (ISurface*)CaptureInterface("vguimatsurface.dll", "VGUI_Surface031");
		I::Engine = (IEngine*)CaptureInterface("engine.dll", "VEngineClient014");
		I::ClientEntityList = (IClientEntityList*)CaptureInterface("client.dll", "VClientEntityList003");
		I::IMatchFramework = (MatchFramework*)CaptureInterface("matchmaking.dll", "MATCHFRAMEWORK_001");
		I::GameEventManager = (IGameEventManager*)CaptureInterface("engine.dll", "GAMEEVENTSMANAGER002");
		I::Client = (IBaseClientDll*)CaptureInterface("client.dll", "VClient018");
		I::PanoramaUIEngine = (IPanoramaUIEngine*)CaptureInterface("panorama.dll", "PanoramaUIEngine001");
		I::ClientState = **(IClientState***)(FindPattern("engine.dll", "A1 ? ? ? ? 33 D2 6A 00 6A 00 33 C9 89 B0") + 1);
		I::MaterialSystem = (IMaterialSystem*)CaptureInterface("materialsystem.dll", "VMaterialSystem080");
		I::InputSystem = (IInputSystem*)CaptureInterface("inputsystem.dll", "InputSystemVersion001");
		I::CVar = (ICVar*)CaptureInterface("vstdlib.dll", "VEngineCvar007");
		I::Globals = **(CGlobalVarsBase***)((FindPattern("client.dll", "A1 ? ? ? ? 5E 8B 40 10") + 1));

		//steam
		HSteamUser hSteamUser = ((HSteamUser(__cdecl*)(void))GetProcAddress(GetModuleHandleA("steam_api.dll"), "SteamAPI_GetHSteamUser"))();
		HSteamPipe hSteamPipe = ((HSteamPipe(__cdecl*)(void))GetProcAddress(GetModuleHandleA("steam_api.dll"), "SteamAPI_GetHSteamPipe"))();
		ISteamClient* pSteamClient = ((ISteamClient * (__cdecl*)(void))GetProcAddress(GetModuleHandleA("steam_api.dll"), "SteamClient"))();
		I::SteamFriends = pSteamClient->GetISteamFriends(hSteamUser, hSteamPipe, STEAMFRIENDS_INTERFACE_VERSION);
		I::SteamMatchmaking = pSteamClient->GetISteamMatchmaking(hSteamUser, hSteamPipe, STEAMMATCHMAKING_INTERFACE_VERSION);
		I::SteamUtils = pSteamClient->GetISteamUtils(hSteamPipe, STEAMUTILS_INTERFACE_VERSION);
		I::SteamUser = pSteamClient->GetISteamUser(hSteamUser, hSteamPipe, STEAMUSER_INTERFACE_VERSION);
		I::SteamGameCoordinator = (ISteamGameCoordinator*)pSteamClient->GetISteamGenericInterface(hSteamUser, hSteamPipe, STEAMGAMECOORDINATOR_INTERFACE_VERSION);
	}
}
