#pragma once
#include "isteamfriends.h"
#include "isteammatchmaking.h"
#include "isteamutils.h"
#include "isteamuser.h"
#include "isteamgamecoordinator.h"

namespace I {
	void CaptureInterfaces();
	extern ISteamFriends* SteamFriends;
	extern ISteamMatchmaking* SteamMatchmaking;
	extern ISteamUtils* SteamUtils;
	extern ISteamUser* SteamUser;
	extern ISteamGameCoordinator* SteamGameCoordinator;
	extern ISurface* Surface;
	extern IEngine* Engine;
	extern IClientEntityList* ClientEntityList;
	extern IInputSystem* InputSystem;
	extern MatchFramework* IMatchFramework;
	extern IGameEventManager* GameEventManager;
	extern IBaseClientDll* Client;
	extern IPanoramaUIEngine* PanoramaUIEngine;
	extern IClientState* ClientState;
	extern IMaterialSystem* MaterialSystem;
	extern ICVar* CVar;
	extern CGlobalVarsBase* Globals;
}