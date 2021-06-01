#pragma once

class IUIPanel;
struct Matrix4x4;
class Entity;

namespace G {
	
	extern bool unload;
	extern bool AutoInviteOnUiChange;
	extern IUIPanel* MainMenuPanel;
	extern std::string ExecuteJs;
	extern std::string ExecuteJsContext;
	extern std::vector<std::pair<uint64_t, uint64_t>> PendingInvites;
	extern ULONGLONG NextAutoQueueCall;
	extern Matrix4x4 viewMatrix;
	extern Entity* LocalPlayer;

};

