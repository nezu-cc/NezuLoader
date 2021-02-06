#include "pch.h"
#include "Hooks.h"

WNDPROC H::OldWindow;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall H::Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_KEYDOWN && wParam == VK_HOME) Menu::open = !Menu::open;

	if (Menu::open) {
		bool ret = ImGui_ImplWin32_WndProcHandler(M::Window, uMsg, wParam, lParam) || uMsg == WM_MOUSEMOVE || uMsg == WM_NCHITTEST || uMsg == WM_GETDLGCODE;
		if (ret && !((uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST) && !ImGui::GetIO().WantCaptureKeyboard))
			return 1;
	}

	return CallWindowProc(OldWindow, hWnd, uMsg, wParam, lParam);
}

