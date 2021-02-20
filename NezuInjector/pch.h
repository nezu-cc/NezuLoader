#ifndef PCH_H
#define PCH_H

#define _CRT_SECURE_NO_WARNINGS
#define DIRECTINPUT_VERSION 0x0800
#define IMGUI_DEFINE_MATH_OPERATORS
#define MMNOJOY

#include <Windows.h>
#include <d3d9.h>
#include <tlhelp32.h>
#include <limits.h>
#include <string.h>
#include <string>
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
#include <shobjidl.h> 
#include <locale>
#include <codecvt>
#include <fstream>
#include <dwmapi.h>
#include <Shlobj.h>
#include <shlwapi.h>
#include <mutex>

#pragma warning( push )
#pragma warning( disable : 6387 6451 6387 6011 28182 6255 26495 6031 26812 6031 26551 )
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#pragma warning( pop )

#include "LoaderLog.h"
#include "Undocumented.h"
#include "Util.h"

#endif //PCH_H