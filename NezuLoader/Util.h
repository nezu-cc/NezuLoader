#pragma once

#include "pch.h"

constexpr float PI_F = 3.14159265358979f;

constexpr float deg2rad(float degrees) noexcept { return degrees * (PI_F / 180.0f); }
constexpr float rad2deg(float radians) noexcept { return radians * (180.0f / PI_F); }

#define RELEASE_TEXTURE(t) if (t) { t->Release(); t = NULL; }

#define WCHARUTF8(t) (std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>::wstring_convert().to_bytes((t)))
#define UTF8WCHAR(t) (std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>::wstring_convert().from_bytes((t)))

#ifdef UNICODE
#define TCHARUTF8(t) WCHARUTF8(t)
#define UTF8TCHAR(t) UTF8WCHAR(t)
#define TCHARWCHAR(t) (std::wstring(t))
#else
#define TCHARUTF8(t) (std::string(t))
#define UTF8TCHAR(t) (std::string(t))
#define TCHARWCHAR(t) UTF8WCHAR(t)
#endif //UNICODE

#define vector_contains(ve, va) (std::find(ve.begin(), ve.end(), va) != ve.end())

#define CLAMP(V, MN, MX) ((V) < (MN) ? (MN) : (V) > (MX) ? (MX) : (V))

class IUIEvent;
struct vec;
DWORD FindPattern(void* start, DWORD size, std::string pattern);
DWORD FindPattern(std::string moduleName, std::string pattern);
std::string BytesToPattern(byte * bytes, DWORD size);
std::string DllFilePicker(HWND owner);
uint64_t DecodeFriendCode(std::string friend_code);
DWORD relativeToAbsolute(DWORD address);
void invitePlayer(const uint64_t& id);
const char* RankString(int id);
void CreateOnlineLobby();
void hexDump(void* addr, int len);
IUIPanel* TryGetMenuPanel(bool retry, DWORD delay);
int GetMaxLobbySlotsForGameMode(const char* mode);
IUIEvent* CreatePanoramaEvent(const std::string & event_name, bool cache_only = false);
bool W2s(const vec& in, ImVec2 & out, bool floor = false);
std::vector<vec> convexHull2d(std::vector<vec> points);
