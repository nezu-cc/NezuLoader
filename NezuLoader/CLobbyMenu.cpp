#include "pch.h"
#include "Hooks.h"

f_ReadyUpForMatch H::oReadyUpForMatch;

bool AutoAccepted = false;

//CLobbyMenuSingleton_ReadyUpForMatch
void __fastcall H::Hooked_ReadyUpForMatch(void* thisPtr, void* edx, bool shouldShow, int playersReadyCount, int numTotalClientsInReservation) {
	oReadyUpForMatch(thisPtr, shouldShow, playersReadyCount, numTotalClientsInReservation);

	if (!shouldShow) {
		AutoAccepted = false;
		return;
	}
	
	printf("Ready players: %d/%d\n", playersReadyCount, numTotalClientsInReservation);

	auto& set = Cfg::c.lobby.auto_queue;

	if (!set.auto_accept) {
		AutoAccepted = false;
		return;
	}

	if (playersReadyCount == 0) {
		IUIEvent* e = CreatePanoramaEvent("MatchAssistedAccept");
		if (!e) {
			L::Error("Failed to create \"MatchAssistedAccept\" event");
			AutoAccepted = false;
			return;
		}
		I::PanoramaUIEngine->AccessUIEngine()->DispatchEventAsync(0.1f, e);
		AutoAccepted = true;
		return;
	}

	if (set.detect_auto_accept && AutoAccepted) {
		L::Info("Detected %d possible auto accept(s)", playersReadyCount - 1);
	}

	AutoAccepted = false;

}