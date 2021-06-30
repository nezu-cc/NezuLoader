#include "pch.h"
#include "Util.h"

#define INRANGE(x,a,b)    (x >= a && x <= b)
#define getBits( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))

DWORD FindPattern(void* start, DWORD size, std::string pattern) {
	const char* pat = pattern.c_str();
	DWORD firstMatch = 0;
	DWORD rangeStart = (DWORD)start;
	if (!rangeStart)
		return NULL;
	DWORD rangeEnd = rangeStart + size;
	for (DWORD pCur = rangeStart; pCur < rangeEnd; pCur++) {
		if (!*pat) return firstMatch;
		if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == getByte(pat)) {
			if (!firstMatch) firstMatch = pCur;
			if (!pat[2]) return firstMatch;
			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?') pat += 3;
			else pat += 2;
		}
		else {
			pat = pattern.c_str();
			firstMatch = 0;
		}
	}
	return NULL;
}

DWORD FindPattern(std::string moduleName, std::string pattern) {
	DWORD rangeStart = (DWORD)GetModuleHandleA(moduleName.c_str());
	if (!rangeStart)
		return NULL;
	MODULEINFO miModInfo;
	GetModuleInformation(GetCurrentProcess(), (HMODULE)rangeStart, &miModInfo, sizeof(MODULEINFO));
	return FindPattern((void*)rangeStart, miModInfo.SizeOfImage, pattern);
}

//thanks hinnie from UC
std::string BytesToPattern(byte* bytes, DWORD size) {
	std::stringstream ida_pattern;
	ida_pattern << std::hex << std::setfill('0');
	for (size_t i = 0; i < size; i++) {
		const int32_t current_byte = bytes[i];
		//I don't need 0xFF to be wildcards, why would you?
		//if (current_byte != 255)
			ida_pattern << std::setw(2) << current_byte;
		//else
			//ida_pattern << std::setw(1) << "?";

		if (i != size - 1)
			ida_pattern << " ";
	}
	return ida_pattern.str();
}

typedef IUIEvent*(__cdecl* setup_event_t)(int(***)(void));

std::unordered_map<std::string, setup_event_t> panorama_event_constructors;

//again, thanks hinnie from UC
IUIEvent* CreatePanoramaEvent(const std::string& event_name, bool cache_only) {

	//try to find the event in cache first
	auto cache_entry = panorama_event_constructors.find(event_name);
	if (cache_entry != panorama_event_constructors.end()) {
		if (!cache_entry->second) //if we didn't find it last time assume we won't ever find it
			return 0;
		if (cache_only)
			return 0;
		return cache_entry->second(0);
	}

	// First of, we're going to find the event name in client.dll, so let's convert the name to an ida style pattern
	std::string ida_pattern = BytesToPattern((byte*)event_name.data(), event_name.size());
	uintptr_t string_in_memory = FindPattern("client.dll", ida_pattern);
	if (!string_in_memory) {
		panorama_event_constructors.insert(std::make_pair(event_name, (setup_event_t)0));
		return 0;
	}

	// Now, we're going to find the first reference to the string in client.dll
	byte bytes[4];
	memcpy(bytes, &string_in_memory, 4);
	ida_pattern = BytesToPattern(bytes, 4);
	uintptr_t string_reference = FindPattern("client.dll", ida_pattern);
	if (!string_reference) {
		panorama_event_constructors.insert(std::make_pair(event_name, (setup_event_t)0));
		return 0;
	}

	// The first function we find after the reference is the setup function of the event. To find it, we can search for the bytes "C7 45 DC"
	// from the string reference.
	uintptr_t setup_fn_ref_addr = FindPattern((void*)string_reference, 999, "C7 45 DC");
	if (!setup_fn_ref_addr) {
		panorama_event_constructors.insert(std::make_pair(event_name, (setup_event_t)0));
		return 0;
	}

	// To get the actual setup function address, we add 3 bytes (because of the C7 45 DC), and then dereference.
	setup_event_t setup_fn_addr = *(setup_event_t*)(setup_fn_ref_addr + 3);

	//add to cache
	panorama_event_constructors.insert(std::make_pair(event_name, setup_fn_addr));

	if (cache_only)
		return 0;

	// The only thing left to do is call the setup function. It will return the event that the function dispatch_event accepts.
	return setup_fn_addr(0);
}

DWORD relativeToAbsolute(DWORD address) {
	return (DWORD)(address + 4 + *(DWORD*)address);
}

std::string DllFilePicker(HWND owner) {
	TCHAR filename[MAX_PATH];
	OPENFILENAME ofn;
	ZeroMemory(&filename, sizeof(filename));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = owner;
	ofn.lpstrFilter = _T("DLL Files\0*.dll\0All Fils\0*.*\0");
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = _T("Select Dll to inject");
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn))
		return TCHARUTF8(filename);
	return "";
}

//stolen from https://stackoverflow.com/questions/216823/
// trim from start (in place)
static inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
		}));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s) {
	ltrim(s);
	rtrim(s);
}

// trim from start (copying)
static inline std::string ltrim_copy(std::string s) {
	ltrim(s);
	return s;
}

// trim from end (copying)
static inline std::string rtrim_copy(std::string s) {
	rtrim(s);
	return s;
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s) {
	trim(s);
	return s;
}

//based on https://github.com/emily33901/js-csfriendcode/blob/master/index.js

std::unordered_map<char, uint64_t> ralnum = {
	{'A', 0 }, {'B', 1 }, {'C', 2 }, {'D', 3 }, {'E', 4 },
	{'F', 5 }, {'G', 6 }, {'H', 7 }, {'J', 8 }, {'K', 9 },
	{'L', 10 }, {'M', 11 }, {'N', 12 }, {'P', 13 }, {'Q', 14 },
	{'R', 15 }, {'S', 16 }, {'T', 17 }, {'U', 18 }, {'V', 19 },
	{'W', 20 }, {'X', 21 }, {'Y', 22 }, {'Z', 23 }, {'2', 24 },
	{'3', 25 }, {'4', 26 }, {'5', 27 }, {'6', 28 }, {'8', 30 },
	{'9', 31 },
};

uint64_t DecodeFriendCode(std::string friend_code) {

	if (friend_code.substr(0, 5) != "AAAA-")
		friend_code = "AAAA-" + friend_code;

	if (friend_code.size() != 15 || friend_code.at(4) != '-' || friend_code.at(10) != '-')
		return 0;

	uint64_t res = 0;
	for (size_t i = 0, j = 0; j < 15; j++) {
		if (j == 4 || j == 10)
			continue;
		res |= ralnum[friend_code[j]] << (5 * i);
		i++;
	}

	uint64_t val = 0;
	for (size_t i = 0; i < 8; i++) {
		val <<= 8;
		val |= res & 0xFF;
		res >>= 8;
	}

	uint64_t id = 0;
	for (size_t i = 0; i < 8; i++) {
		val >>= 1;
		uint64_t id_nibble = val & 0xF;
		val >>= 4;

		id <<= 4;
		id |= id_nibble;
	}

	return id | 0x110000100000000;
}

void invitePlayer(const uint64_t& id) { // yes, pasted XD (thanks uc)
	typedef void(__thiscall* f_InvitePlayer)(uintptr_t, uint64_t);
	static const auto this_ = *(uintptr_t**)(FindPattern("client.dll", "83 3D ? ? ? ? ? 74 5E 8B 0D ? ? ? ? 68") + 2);
	static const auto fn = (f_InvitePlayer)FindPattern("client.dll", "55 8B EC 83 E4 F8 83 EC 1C 53 56 57 8B F1 FF 15 ? ? ? ? 8B");
	if (*this_)
		fn(*this_, id);
}

const char* RankString(int id) {
	switch (id)
	{
	case 0: return "Unranked";
	case 1:	return "Silver I";
	case 2:	return "Silver 2";
	case 3:	return "Silver III";
	case 4:	return "Silver IV";
	case 5:	return "Silver Elite";
	case 6:	return "Silver Elite Master";
	case 7:	return "Gold Nova I";
	case 8:	return "Gold Nova II";
	case 9:	return "Gold Nova III";
	case 10: return "Gold Nova Master";
	case 11: return "Master Guardian I";
	case 12: return "Master Guardian II";
	case 13: return "Master Guardian Elite";
	case 14: return "Distinguished Master Guardian";
	case 15: return "Legendary Eagle";
	case 16: return "Legendary Eagle Master";
	case 17: return "Supreme Master First Class";
	case 18: return "Global Elite";
	default: return "Unknown";
	}
}

void CreateOnlineLobby() {
	M::CreateSession();
	M::CreateSessionUI();
	MatchSession* match_session = I::IMatchFramework->get_match_session();
	if (match_session && match_session->get_lobby_id() == 0) {
		KeyValues* keyValues = new KeyValues();
		M::InitKeyValues(keyValues, "MakeOnline");
		match_session->command(keyValues);
	}
}

//pasted from SO
void hexDump(void* addr, int len) {
	int i;
	unsigned char buff[17];
	unsigned char* pc = (unsigned char*)addr;
	if (len == 0) {
		printf("  ZERO LENGTH\n");
		return;
	}
	if (len < 0) {
		printf("  NEGATIVE LENGTH: %i\n", len);
		return;
	}
	for (i = 0; i < len; i++) {
		if ((i % 16) == 0) {
			if (i != 0) printf("  %s\n", buff);
			printf("  %04x ", i);
		}
		printf(" %02x", pc[i]);
		if ((pc[i] < 0x20) || (pc[i] > 0x7e)) buff[i % 16] = '.';
		else buff[i % 16] = pc[i];
		buff[(i % 16) + 1] = '\0';
	}
	while ((i % 16) != 0) {
		printf("   ");
		i++;
	}
	printf("  %s\n", buff);
}

IUIPanel* TryGetMenuPanel(bool retry, DWORD delay) {
	IUIPanel* panel = NULL;
	do {
		panel = I::PanoramaUIEngine->AccessUIEngine()->GetLastDispatchedEventTargetPanel();
		if (I::PanoramaUIEngine->AccessUIEngine()->IsValidPanelPointer(panel)) {
			do {
				if (!strcmp(panel->GetID(), "MainMenuContainerPanel")) return panel;
			} while ((panel = panel->GetParent()) != NULL && I::PanoramaUIEngine->AccessUIEngine()->IsValidPanelPointer(panel));
		}
		else panel = NULL;
		if (retry && delay) Sleep(delay);
	} while (retry && (panel == NULL || !I::PanoramaUIEngine->AccessUIEngine()->IsValidPanelPointer(panel)));
	return NULL;
}

// code.pbin/panorama/scripts/common/sessionutil.js -> _GetMaxLobbySlotsForGameMode
int GetMaxLobbySlotsForGameMode(const char* mode) {
	switch (fnv::hashRuntime(mode))
	{
	case fnv::hash("scrimcomp2v2"):
	case fnv::hash("cooperative"):
	case fnv::hash("coopmission"):
	case fnv::hash("survival"):
		return 2;
	default:
		return 5;
	}
}

bool W2s(const vec& in, ImVec2& out, bool floor) {
	const auto& matrix = G::viewMatrix;

	const auto w = matrix._41 * in.x + matrix._42 * in.y + matrix._43 * in.z + matrix._44;
	if (w < 0.001f)
		return false;

	out = ImGui::GetIO().DisplaySize / 2.0f;
	out.x *= 1.0f + (matrix._11 * in.x + matrix._12 * in.y + matrix._13 * in.z + matrix._14) / w;
	out.y *= 1.0f - (matrix._21 * in.x + matrix._22 * in.y + matrix._23 * in.z + matrix._24) / w;
	if (floor)
		out = ImFloor(out);
	return true;
}

std::vector<vec> convexHull2d(std::vector<vec> points) {

	static auto orientation = [](vec p, vec q, vec r) -> int {
		float val = (q.y - p.y) * (r.x - q.x) -
			(q.x - p.x) * (r.y - q.y);
		if (val == 0) return 0;
		return (val > 0) ? 1 : 2;
	};

	static auto distSq = [](vec p1, vec p2) -> float {
		return (p1.x - p2.x) * (p1.x - p2.x) +
			(p1.y - p2.y) * (p1.y - p2.y);
	};

	static auto nextToTop = [](auto& S) -> vec {
		vec p = S.top();
		S.pop();
		vec res = S.top();
		S.push(p);
		return res;
	};

	float ymin = points[0].y;
	int min = 0;
	for (size_t i = 1; i < points.size(); i++) {
		float y = points[i].y;
		if ((y < ymin) || (ymin == y &&
			points[i].x < points[min].x))
			ymin = points[i].y, min = i;
	}

	std::swap(points[0], points[min]);

	static vec p0;
	p0 = points[0];
	qsort(&points[1], points.size() - 1, sizeof(vec), [](const void* vp1, const void* vp2) {
		vec* p1 = (vec*)vp1;
		vec* p2 = (vec*)vp2;
		int o = orientation(p0, *p1, *p2);
		if (o == 0)
			return (distSq(p0, *p2) >= distSq(p0, *p1)) ? -1 : 1;
		return (o == 2) ? -1 : 1;
	});

	int m = 1;
	for (size_t i = 1; i < points.size(); i++) {
		while (i < points.size() - 1 && orientation(p0, points[i], points[i + 1]) == 0)
			i++;
		points[m] = points[i];
		m++; 
	}

	if (m < 3) return points;

	std::stack<vec, std::vector<vec>> S;
	S.push(points[0]);
	S.push(points[1]);
	S.push(points[2]);

	for (int i = 3; i < m; i++) {
		while (S.size() > 1 && orientation(nextToTop(S), S.top(), points[i]) != 2)
			S.pop();
		S.push(points[i]);
	}

	vec* end = &S.top() + 1;
	vec* begin = end - S.size();

	return std::vector<vec>(begin, end);
}

unsigned int __stdcall ScanMemory(void* patt2) {
	//__try 
	{
		std::string* patt = (std::string*)patt2;
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		L::Info("[+] MinAddress: 0x%X", (DWORD)si.lpMinimumApplicationAddress);
		L::Info("[+] MaxAddress: 0x%X", (DWORD)si.lpMaximumApplicationAddress);

		/* walk process addresses */
		LPVOID lpMem = 0;
		MEMORY_BASIC_INFORMATION mbi;
		while (lpMem < si.lpMaximumApplicationAddress)
		{
			int r = VirtualQueryEx(
				GetCurrentProcess(),
				lpMem, //A pointer to the base address of the region of pages to be queried.
				//A pointer to a MEMORY_BASIC_INFORMATION structure
				// in which information about the specified page range is returned.
				&mbi,
				sizeof(MEMORY_BASIC_INFORMATION));

			if (r == 0) {
				L::Error("VirtualQueryEx faield");
				return 1;
			}
			if (mbi.Protect & PAGE_GUARD) {
				//L::Warning("Skipping guarded page");
				goto end;
			}
			if (mbi.Protect & PAGE_EXECUTE) {
				//L::Warning("Skipping guarded page");
				goto end;
			}

			if (mbi.Protect & PAGE_READONLY || mbi.Protect & PAGE_READWRITE) {
				//printf("%X\n", (DWORD)mbi.BaseAddress);
				const char* pat = patt->c_str();
				DWORD firstMatch = 0;
				DWORD rangeEnd = (DWORD)mbi.BaseAddress + mbi.RegionSize;
				for (DWORD pCur = (DWORD)mbi.BaseAddress; pCur < rangeEnd; pCur++) {
					if (*(CHAR*)pCur == *pat) {
						if (!firstMatch) firstMatch = pCur;
						if (!pat[1]) {
							L::Info("Found pattern at 0x%X", firstMatch);
							pat = patt->c_str();
							firstMatch = 0;
							continue;
						}
						else pat += 1;
					}
					else {
						pat = patt->c_str();
						firstMatch = 0;
					}
				}
			}
			//L::Debug("[+] mbi.RegionSize 0x%X - 0x%X", (DWORD)mbi.BaseAddress, mbi.RegionSize);
		end:
			lpMem = (LPVOID)((DWORD)mbi.BaseAddress + (DWORD)mbi.RegionSize);
		}
	}
	L::Info("Done");
	//__except (EXCEPTION_EXECUTE_HANDLER) {
	//    L::Debug("Caught exception code: 0x%X", GetExceptionCode());
	//    return 1;
	//}
	return 0;
};