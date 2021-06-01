#include "pch.h"
#include "Hooks.h"

f_WndProc H::oHooked_WndProc;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall H::Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	{
		std::lock_guard<std::mutex> lk(load_lock_m);
		load_lock = true;
	}
	load_lock_cv.notify_all();


	if (uMsg == WM_KEYDOWN && wParam == VK_HOME) {
		Menu::opening = (Menu::open = !Menu::open) ? true : Menu::opening;
		if (!Menu::open)
			I::InputSystem->enableInput(true);//make sure input is enabled on menu close
	}

	if (Menu::open) {
		bool ret = ImGui_ImplWin32_WndProcHandler(M::Window, uMsg, wParam, lParam) || uMsg == WM_MOUSEMOVE || uMsg == WM_NCHITTEST || uMsg == WM_GETDLGCODE;

		bool want_kbd_state = ImGui::GetCurrentContext() && ImGui::GetIO().WantTextInput;
		static bool last_want_kbd_state = want_kbd_state;

		I::InputSystem->enableInput(!want_kbd_state);
		if (want_kbd_state != last_want_kbd_state)
			I::InputSystem->resetInputState();
		last_want_kbd_state = want_kbd_state;


		if (ret && !((uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST) && !ImGui::GetIO().WantCaptureKeyboard))
			return 1;
	}

	return oHooked_WndProc(hWnd, uMsg, wParam, lParam);
}

