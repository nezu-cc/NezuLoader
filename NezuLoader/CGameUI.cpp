#include "pch.h"
#include "Hooks.h"

f_ChangeGameUIState H::oChangeGameUIState;

void __fastcall H::Hooked_ChangeGameUIState(CGameUI* thisptr, void* edx, CSGOGameUIState_t nNewState) {
	CSGOGameUIState_t nOldState = thisptr->m_CSGOGameUIState;
	oChangeGameUIState(thisptr, nNewState);

	if (nOldState == nNewState)// no change
		return;

	if (G::AutoInviteOnUiChange && nOldState == CSGO_GAME_UI_STATE_INGAME && nNewState == CSGO_GAME_UI_STATE_MAINMENU) {
		CreateOnlineLobby();
		for (uint64_t id : Cfg::c.lobby.auto_invite.steam_ids) {
			invitePlayer(id);
			L::Debug("auto-invited %llu", id);
		}
		G::AutoInviteOnUiChange = false;
	}
}