#include "pch.h"
#include "Memory.h"

namespace M {

	IDirect3DDevice9* d3d9Device;
	HWND Window;

	void FindAll() {

		d3d9Device = **(IDirect3DDevice9***)(FindPattern("shaderapidx9.dll", "A1 ? ? ? ? 50 8B 08 FF 51 0C") + 1);

	}

}
