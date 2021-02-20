#ifndef PCH_H
#define PCH_H

#define _CRT_SECURE_NO_WARNINGS
#define DIRECTINPUT_VERSION 0x0800
#define IMGUI_DEFINE_MATH_OPERATORS

#include <windows.h>
#include <tlhelp32.h>
#include <limits.h>
#include <string.h>
#include <string>
#include <d3d9.h>
#include <dinput.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <tchar.h>
#include <malloc.h> 
#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <psapi.h>
#include <unordered_map>

#include "Globals.h"
#include "Util.h"
#include "Structs.h"
#include "Interfaces.h"
#include "Memory.h"
#include "Menu.h"

#pragma warning( push )
#pragma warning( disable : 6387 6451 6387 6011 28182 6255 26495 6031 26812 6031 26551 )
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#pragma warning( pop )

#endif //PCH_H