#include "pch.h"
#include "Hooks.h"
#include "MinHook/MinHook.h"



LPVOID MH_HookVtbl(LPVOID object, DWORD index, LPVOID funct) {
	LPVOID orig = NULL;
	MH_STATUS status = MH_CreateHook((*(void***)object)[index], funct, &orig);
	if (status != MH_OK) {
		printf("[nezu] MH_CreateHook failed: %d\n", status);
		return NULL;
	}
	return orig;
}

void H::ApplyHooks() {
	using namespace H;
	if (MH_Initialize() != MH_OK)
		return;
	
	oHooked_EndScene = (f_EndScene)MH_HookVtbl(M::d3d9Device, 42, &Hooked_EndScene);
	oHooked_Reset = (f_Reset)MH_HookVtbl(M::d3d9Device, 16, &Hooked_Reset);
	oHooked_LockCursor = (f_LockCursor)MH_HookVtbl(I::Surface, 67, &Hooked_LockCursor);

	while (!(M::Window = FindWindowA("Valve001", nullptr))) 
		Sleep(50);
	OldWindow = (WNDPROC)SetWindowLongPtrA(M::Window, GWL_WNDPROC, (LONG_PTR)Hooked_WndProc);


	MH_EnableHook(MH_ALL_HOOKS);
}