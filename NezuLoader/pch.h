#ifndef PCH_H
#define PCH_H

#define _CRT_SECURE_NO_WARNINGS
#define DIRECTINPUT_VERSION 0x0800
#define IMGUI_DEFINE_MATH_OPERATORS
#define MM_INTERNAL
#define _USE_MATH_DEFINES

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
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <Shlobj.h>
#include <shlwapi.h>
#include <intrin.h>
#include <locale>
#include <codecvt>
#include <json.hpp>
#include <Crc32.h>
#include <Injector.h>
#include <regex>
#include <set>
#include <iomanip>
#include <stack>
#include <algorithm>

#pragma warning( push )
#pragma warning( disable : 6387 6451 6387 6011 28182 6255 26495 6031 26812 6031 26551 )
#include "imgui_custom.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include "imgui_memory_editor.h"
#pragma warning( pop )

#pragma warning( disable : 26812 )
#include "protobuf/pb_decode.h"
#include "protobuf/pb_encode.h"

using nlohmann::json;

#include "SDK/clzss.h"
#include "SDK/bitbuf.h"

#include "fnv.h"
#include "Log.h"
#include "Config.h"
#include "NetVarManager.h"
#include "CheatMod.h"
#include "Globals.h"
#include "Util.h"
#include "Memory.h"
#include "Structs.h"
#include "Interfaces.h"
#include "NezuLoader.h"
#include "Cheat.h"
#include "Menu.h"
#include "GameCoordinator.h"


#endif //PCH_H