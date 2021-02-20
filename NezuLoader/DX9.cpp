#include "pch.h"
#include "Hooks.h"
#include "NezuLoader.h"

f_EndScene H::oHooked_EndScene;
f_Reset H::oHooked_Reset;

HRESULT __stdcall H::Hooked_EndScene(IDirect3DDevice9* pDevice) {
	static bool init = false;
	if (!init && M::Window) {
		Menu::init(pDevice);
		init = true;
		return oHooked_EndScene(pDevice);
	}
	else if (!Menu::open || !init || G::unload) return oHooked_EndScene(pDevice);

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	Menu::Draw();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	HRESULT ret = H::oHooked_EndScene(pDevice);
	if(G::unload)
		NezuLoader::Unload();
	return ret;
}

HRESULT __stdcall H::Hooked_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {

	ImGui_ImplDX9_InvalidateDeviceObjects();
	ImGui_ImplDX9_CreateDeviceObjects();

	return H::oHooked_Reset(pDevice, pPresentationParameters);
}
