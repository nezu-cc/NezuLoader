#pragma once
#include "steam/isteamfriends.h"
#include "steam/isteammatchmaking.h"
#include "steam/isteamutils.h"
#include "steam/isteamuser.h"
#include "steam/isteamgamecoordinator.h"

namespace I {
	void CaptureInterfaces();
	extern ISteamFriends* SteamFriends;
	extern ISteamMatchmaking* SteamMatchmaking;
	extern ISteamUtils* SteamUtils;
	extern ISteamUser* SteamUser;
	extern ISteamGameCoordinator* SteamGameCoordinator;
	extern ISurface* Surface;
}