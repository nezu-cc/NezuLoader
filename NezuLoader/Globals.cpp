#include "pch.h"
#include "Globals.h"

namespace G {

#ifdef _DEBUG
	bool debug = true;
#else
	bool debug = false;
#endif
	bool unload = false;
	bool AutoInviteOnUiChange = false;
	IUIPanel* MainMenuPanel = NULL;
	std::string ExecuteJs;
	std::string ExecuteJsContext = "panorama/layout/mainmenu.xml";
	std::vector<std::pair<uint64_t, uint64_t>> PendingInvites;
	ULONGLONG NextAutoQueueCall = GetTickCount64() + 1000;
	Matrix4x4 viewMatrix;
	Entity* LocalPlayer = NULL;
	float crasherStartTime = 0;
	bool crashPrimed = false;
}
