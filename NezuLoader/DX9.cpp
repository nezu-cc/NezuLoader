#include "pch.h"
#include "Hooks.h"
#include "NezuLoader.h"
#include "Visuals.h"

f_Present* H::fake_Present = NULL;
f_Reset* H::fake_Reset = NULL;

f_Present H::oHookedPresent;
f_Reset H::oHooked_Reset;

HRESULT __stdcall H::Hooked_Present(IDirect3DDevice9* pDevice, const RECT* src, const RECT* dest, HWND wnd_override, const RGNDATA* dirty_region) {
	if (fake_Present && *fake_Present) {
		(*fake_Present)(pDevice, src, dest, wnd_override, dirty_region);
	}

	static bool init = false;
	if (!init && M::Window) {
		Menu::init(pDevice);
		init = true;
		return oHookedPresent(pDevice, src, dest, wnd_override, dirty_region);
	}
	else if (!init || G::unload) return oHookedPresent(pDevice, src, dest, wnd_override, dirty_region);

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	Visuals::DrawEsp(ImGui::GetBackgroundDrawList());

	if (Menu::open)
		Menu::Draw(pDevice);

	ImGui::EndFrame();
	ImGui::Render();

	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	HRESULT ret = oHookedPresent(pDevice, src, dest, wnd_override, dirty_region);
	if (G::unload) {
		NezuLoader::Unload();
		Menu::uninit();
	}
	return ret;
}

HRESULT __stdcall H::Hooked_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {

	ImGui_ImplDX9_InvalidateDeviceObjects();
	RELEASE_TEXTURE(Menu::textures.reload);
	RELEASE_TEXTURE(Menu::textures.player_avatar);

	return H::oHooked_Reset(pDevice, pPresentationParameters);
}
