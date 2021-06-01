#include "pch.h"
#include "NezuLoader.h"
#include "Hooks.h"

//WndProc stuff
f_WndProc real_oHooked_WndProc;
LRESULT __stdcall Fake_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return real_oHooked_WndProc(hWnd, uMsg, wParam, lParam);
}

LONG WINAPI GetWindowLongW_Hooked(HWND hWnd, int nIndex) {
	if (nIndex == GWL_WNDPROC) {
		return (LONG)&Fake_WndProc;
	}
	return GetWindowLongW(hWnd, nIndex);
}

LONG WINAPI GetWindowLongA_Hooked(HWND hWnd, int nIndex) {
	if (nIndex == GWL_WNDPROC) {
		return (LONG)&Fake_WndProc;
	}
	return GetWindowLongA(hWnd, nIndex);
}

LONG WINAPI SetWindowLongW_Hooked(HWND hWnd, int nIndex, LONG dwNewLong) {
	L::Info("SetWindowLongW emulated %d", nIndex);
	if (nIndex == GWL_WNDPROC) {
		LONG orig = (LONG)real_oHooked_WndProc;
		real_oHooked_WndProc = (f_WndProc)dwNewLong;
		return orig;
	}
	return SetWindowLongW(hWnd, nIndex, dwNewLong);
}

LONG WINAPI SetWindowLongA_Hooked(HWND hWnd, int nIndex, LONG dwNewLong) {
	L::Info("SetWindowLongA emulated %d", nIndex);
	if (nIndex == GWL_WNDPROC) {
		LONG orig = (LONG)real_oHooked_WndProc;
		real_oHooked_WndProc = (f_WndProc)dwNewLong;
		return orig;
	}
	return SetWindowLongA(hWnd, nIndex, dwNewLong);
}

//DirectX9 stuff

HRESULT __stdcall Fake_Present(IDirect3DDevice9* pDevice, const RECT* src, const RECT* dest, HWND wnd_override, const RGNDATA* dirty_region);
f_Present pFake_Present = &Fake_Present;

HRESULT __stdcall Fake_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
f_Reset pFake_Reset = &Fake_Reset;

#pragma pack(push, 1)
struct t_gor {
	//some padding just so patern dosn't start at offset 0
	BYTE pad_1[0x100] = { 0x00 };

	//reset C7 45 ? ? ? ? ? FF 15 ? ? ? ? 8B F8 + 9
	BYTE reset_1[9] = { 0xC7, 0x45, 0x69, 0x69, 0x69, 0x69, 0x69, 0xFF, 0x15 };
	f_Reset* reset_addr = &pFake_Reset;
	BYTE reset_3[2] = { 0x8B, 0xF8 };
	
	//some more padding
	BYTE pad_2[0x100] = { 0x00 };

	//present FF 15 ? ? ? ? 8B F8 85 DB + 2
	BYTE present_1[2] = { 0xFF, 0x15 };
	f_Present* present_addr = &pFake_Present;
	BYTE present_2[4] = { 0x8B, 0xF8, 0x85, 0xDB };

	//and even more padding
	BYTE pad_3[0x100] = { 0x00 };

};
#pragma pack(pop)

static t_gor fake_gor;

HRESULT __stdcall Fake_Present(IDirect3DDevice9* pDevice, const RECT* src, const RECT* dest, HWND wnd_override, const RGNDATA* dirty_region) {
	static void* addr = Fake_Present;
	if (*fake_gor.present_addr != addr) {
		addr = *fake_gor.present_addr;
		L::Warning("Cheat attempted to hook Steam overlay. Steam overlay was spoofed. The cheat will no longer be stream proof!");
	}
	return D3D_OK;
}

HRESULT __stdcall Fake_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
	return D3D_OK;
}

BOOL WINAPI K32GetModuleInformation_Hooked(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb) {
	BOOL ret = K32GetModuleInformation(hProcess, hModule, lpmodinfo, cb);
	if (ret && lpmodinfo) {
		CHAR name[MAX_PATH];
		if (GetModuleBaseNameA(hProcess, hModule, name, sizeof(name)) && !_strcmpi(name, "gameoverlayrenderer.dll")) {
			lpmodinfo->lpBaseOfDll = &fake_gor;
			lpmodinfo->SizeOfImage = sizeof(fake_gor);
			//printf("GetModuleInformation_Hooked -> %s %X\n", name, (DWORD)lpmodinfo->lpBaseOfDll);
		}
	}
	return ret;
}

BOOL NezuLoader::CheatLoader(LPVOID mod, DWORD mod_size) {
	//We are the alpha male here, all other cheats should be below us

	if (H::oHooked_WndProc != Fake_WndProc) {
		real_oHooked_WndProc = H::oHooked_WndProc;
		H::oHooked_WndProc = Fake_WndProc;
	}
	if (!H::fake_Present || !H::fake_Reset) {
		H::fake_Present = fake_gor.present_addr;
		H::fake_Reset = fake_gor.reset_addr;
	}

	//TODO: hook GetProcAddress for cheats that dynamicaly resolve functions

	//so that cheats hook our fake WndProc instead of the real one
	if (HookIAT(mod, "user32.dll", "GetWindowLongW", GetWindowLongW_Hooked))
		L::Debug("GetWindowLongW hooked");
	if (HookIAT(mod, "user32.dll", "GetWindowLongA", GetWindowLongA_Hooked))
		L::Debug("GetWindowLongA hooked");

	//emulate replacing the GWL_WNDPROC to stay on top
	if (HookIAT(mod, "user32.dll", "SetWindowLongW", SetWindowLongW_Hooked))
		L::Debug("SetWindowLongW hooked");
	if (HookIAT(mod, "user32.dll", "SetWindowLongA", SetWindowLongA_Hooked))
		L::Debug("SetWindowLongA hooked");

	//NOTE: if a cheat is parsing PE headers instead of using this api this won't work
	//TODO: find empty space in gameoverlayrenderer and put fake sigs there
	if (HookIAT(mod, "kernel32.dll", "K32GetModuleInformation", K32GetModuleInformation_Hooked))
		L::Debug("K32GetModuleInformation hooked");

	return TRUE;
}