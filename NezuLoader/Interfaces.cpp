#include "pch.h"
#include "Interfaces.h"
#include "steam/isteamclient.h"

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

	void CaptureInterfaces() {

		I::Surface = (ISurface*)CaptureInterface("vguimatsurface.dll", "VGUI_Surface031");

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



