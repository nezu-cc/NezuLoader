#include "pch.h"
#include "GameEventListener.h"

void GameEventListener::init() {
	I::GameEventManager->AddListener(this, "cs_win_panel_match");
    I::GameEventManager->AddListener(this, "round_freeze_end");
}

void GameEventListener::remove() {
	I::GameEventManager->RemoveListener(this);
}

void GameEventListener::FireGameEvent(IGameEvent* event) {
    switch (fnv::hashRuntime(event->GetName())) {
    case fnv::hash("cs_win_panel_match"):
        L::Debug("cs_win_panel_match");
        if (Cfg::c.lobby.auto_queue.auto_disconnect) {
            I::Engine->ClientCmd_Unrestricted("disconnect");
            I::InputSystem->resetInputState();
            if (Cfg::c.lobby.auto_invite.enabled)
                G::AutoInviteOnUiChange = true;
        }
        break;
    case fnv::hash("round_freeze_end"):
        L::Debug("round_freeze_end");
        if (Cfg::c.misc.crasher_auto)
            G::crashPrimed = true;
        break;
    }
}
