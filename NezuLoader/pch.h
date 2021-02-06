// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#define _CRT_SECURE_NO_WARNINGS
#define DIRECTINPUT_VERSION 0x0800
#define IMGUI_DEFINE_MATH_OPERATORS

// add headers that you want to pre-compile here
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

#include "Util.h"
#include "Structs.h"
#include "Interfaces.h"
#include "Memory.h"
#include "Menu.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#endif //PCH_H
