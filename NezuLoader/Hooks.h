#pragma once

typedef HRESULT(_stdcall* f_EndScene)(IDirect3DDevice9* pDevice);
typedef HRESULT(_stdcall* f_Reset)(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
typedef void(__thiscall* f_LockCursor)(void*);

namespace H {
	extern WNDPROC OldWindow;
	extern f_EndScene oHooked_EndScene;
	extern f_Reset oHooked_Reset;
	extern f_LockCursor oHooked_LockCursor;

	HRESULT __stdcall Hooked_EndScene(IDirect3DDevice9* pDevice);
	HRESULT __stdcall Hooked_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
	LRESULT __stdcall Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void __fastcall Hooked_LockCursor(void* thisPtr, void* edx);

	void ApplyHooks();
};

