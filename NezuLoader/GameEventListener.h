#pragma once
class GameEventListener : public IGameEventListener {
public:
	void init();
	void remove();
	void FireGameEvent(IGameEvent* event) override;
	int GetEventDebugID(void) override { return EVENT_DEBUG_ID_INIT; }

	static GameEventListener* singleton() {
		static GameEventListener* instance = new GameEventListener();
		return instance;
	}
};

