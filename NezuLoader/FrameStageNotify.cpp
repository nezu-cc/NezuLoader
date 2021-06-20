#include "pch.h"
#include "Hooks.h"

f_FrameStageNotify H::oFrameStageNotify;

void __fastcall H::Hooked_FrameStageNotify(void* thisPtr, void* edx, ClientFrameStage_t curStage) {

	if (curStage == FRAME_START) {
		G::LocalPlayer = I::ClientEntityList->GetClientEntity(I::Engine->GetLocalPlayer());
		G::viewMatrix = I::Engine->WorldToScreenMatrix();
	}

	if (curStage == FRAME_RENDER_START) {

		H::Hook_NET_SendLong(Cfg::c.misc.crasher);
		if (Cfg::c.misc.crasher) {

			static ConVar* sv_maxroutable = I::CVar->FindVar("sv_maxroutable");
			static ConVar* net_maxroutable = I::CVar->FindVar("net_maxroutable");
			static ConVar* cl_flushentitypacket = I::CVar->FindVar("cl_flushentitypacket");
			static ConVar* net_compresspackets_minsize = I::CVar->FindVar("net_compresspackets_minsize");
			static ConVar* net_compresspackets = I::CVar->FindVar("net_compresspackets");
			static ConVar* net_threaded_socket_recovery_time = I::CVar->FindVar("net_threaded_socket_recovery_time");
			static ConVar* net_threaded_socket_recovery_rate = I::CVar->FindVar("net_threaded_socket_recovery_rate");
			static ConVar* net_threaded_socket_burst_cap = I::CVar->FindVar("net_threaded_socket_burst_cap");
			static ConVar* rate = I::CVar->FindVar("rate");

			*(int*)((DWORD)&rate->fnChangeCallback + 0xC) = NULL;
			*(int*)((DWORD)&net_threaded_socket_burst_cap->fnChangeCallback + 0xC) = NULL;
			*(int*)((DWORD)&net_threaded_socket_recovery_rate->fnChangeCallback + 0xC) = NULL;
			*(int*)((DWORD)&net_threaded_socket_recovery_time->fnChangeCallback + 0xC) = NULL;
			*(int*)((DWORD)&net_compresspackets->fnChangeCallback + 0xC) = NULL;
			*(int*)((DWORD)&net_compresspackets_minsize->fnChangeCallback + 0xC) = NULL;
			*(int*)((DWORD)&net_maxroutable->fnChangeCallback + 0xC) = NULL;
			*(int*)((DWORD)&sv_maxroutable->fnChangeCallback + 0xC) = NULL;
			*(int*)((DWORD)&cl_flushentitypacket->fnChangeCallback + 0xC) = NULL;

			net_maxroutable->bHasMin = false;
			net_maxroutable->bHasMax = false;

			rate->SetValue(99999999);
			net_threaded_socket_recovery_time->SetValue(2);
			net_threaded_socket_recovery_rate->SetValue(999999);
			net_threaded_socket_burst_cap->SetValue(999999);
			net_compresspackets->SetValue(true);
			net_compresspackets_minsize->SetValue(0);
			net_maxroutable->SetValue(120);
			net_maxroutable->SetValue(120.f);
			sv_maxroutable->SetValue(50);

		}
		else {
			static ConVar* net_maxroutable = I::CVar->FindVar("net_maxroutable");
			*(int*)((DWORD)&net_maxroutable->fnChangeCallback + 0xC) = NULL;
			net_maxroutable->bHasMin = false;
			net_maxroutable->bHasMax = false;
			net_maxroutable->SetValue(1200);
			net_maxroutable->SetValue(1200.f);
		}

		//has to be done in a game thread, that's why it's here
		if (Cfg::c.lobby.auto_queue.enabled && G::MainMenuPanel) {
			if (*M::UiComponent_PartyList && I::ClientState->signonState == 0
				&& (*M::MatchmakingUIStatusString == NULL || **M::MatchmakingUIStatusString == '\0') //panorama uses the same check btw
				&& (*M::CurrentMatchState & 2) == 0 && GetTickCount64() >= G::NextAutoQueueCall) {

				G::NextAutoQueueCall = GetTickCount64() + 500; //no need to spam

				bool ok_to_start = false;
				bool OpenPlayMenu = false;
				int& min_players = Cfg::c.lobby.auto_queue.min_players;

				MatchSession* match_session = I::IMatchFramework->get_match_session();

				if (min_players == 1 && (!match_session || match_session->get_lobby_id() == 0)) {//solo, no lobby required. If we have a lobby and we are still solo the next check will pass
					OpenPlayMenu = true;
					ok_to_start = true;
				}
				else if (match_session && match_session->get_lobby_id() != 0) {//check for if we have an "online" lobby
					if (I::SteamMatchmaking->GetLobbyOwner(CSteamID(match_session->get_lobby_id())) == I::SteamUser->GetSteamID()) { //check if we are lobby owner
						KeyValues* settings = match_session->get_session_settings();
						const char* gamemode = settings->GetString("game/mode");

						int num_slots = GetMaxLobbySlotsForGameMode(gamemode);
						int num_players = (*M::UiComponent_PartyList)->GetCount();

						if (min_players == 0 && num_slots == num_players)
							ok_to_start = true;
						else if (min_players >= 1 && num_players >= min_players && num_players <= num_slots)
							ok_to_start = true;
					}

				}

				if (ok_to_start) {
					const char* script;
					if (OpenPlayMenu) //if we are alone we need to open PlayMenu first to initialise the offline lobby, there is a better way to do this(manualy) but i'm too lazy
						script = "$.DispatchEvent('OpenPlayMenu'); LobbyAPI.StartMatchmaking('', '', '', '')";
					else
						script = "LobbyAPI.StartMatchmaking('', '', '', '')";

					I::PanoramaUIEngine->AccessUIEngine()->RunScript(G::MainMenuPanel, script, "panorama/layout/mainmenu.xml", 8, 10, false, false);
				}
			}

		}

		if (Cfg::c.lobby.auto_invite.auto_join && *M::UiComponent_PartyBrowser) {
			for (size_t i = 0; i < G::PendingInvites.size(); i++) {
				auto& invite = G::PendingInvites[i];
				if ((*M::UiComponent_PartyBrowser)->HasBeenInvited(invite.first)) {
					G::PendingInvites.erase(G::PendingInvites.begin() + i);
					*M::JoinLobbyId = invite.first;
					M::JoinLobby();
					break;
				}
				else if (GetTickCount64() >= invite.second + 5000) { //delete invalid after 5 seconds
					G::PendingInvites.erase(G::PendingInvites.begin() + i);
					break;
				}
			}
		}
		else if (!Cfg::c.lobby.auto_invite.auto_join)
			G::PendingInvites.clear();

		if (!G::ExecuteJs.empty() && G::MainMenuPanel) {
			I::PanoramaUIEngine->AccessUIEngine()->RunScript(G::MainMenuPanel, G::ExecuteJs.c_str(), G::ExecuteJsContext.c_str(), 8, 10, false, false);
			G::ExecuteJs.clear();
		}


	}
	if (I::Engine->IsInGame()) {
		if (curStage == FRAME_RENDER_START || curStage == FRAME_RENDER_END) {

			//thx osiris
			auto getGrassMaterialName = []() noexcept -> const char* {
				switch (fnv::hashRuntime(I::Engine->GetLevelNameShort())) {
				case fnv::hash("dz_blacksite"): return "detail/detailsprites_survival";
				case fnv::hash("dz_sirocco"): return "detail/dust_massive_detail_sprites";
				case fnv::hash("coop_autumn"): return "detail/autumn_detail_sprites";
				case fnv::hash("dz_frostbite"): return "ski/detail/detailsprites_overgrown_ski";
				default: return nullptr;
				}
			};

			if (const auto grassMaterialName = getGrassMaterialName())
				I::MaterialSystem->FindMaterial(grassMaterialName, NULL, true, NULL)->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, curStage == FRAME_RENDER_START && Cfg::c.visuals.remove_grass);
		}
	}

	oFrameStageNotify(thisPtr, curStage);
}