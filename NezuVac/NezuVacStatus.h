#pragma once

#define NEZUVACSTATUS_SECTION_NAME ".nezuvac"
#define NEZUVACSTATUS_SECTION_NAME_1 ".nezuvac$1"

namespace NezuVac {

	enum class Error {
		Success = 0,
		NotLoaded,
		MHInit,
		FindPattern_loading,
		FindPattern_calling,
		CreateHook_loading,
		CreateHook_calling,
		ApplyHook
	};

	inline LPCSTR GetErrorString(Error error) {
		switch (error)
		{
		case Error::Success: return "Success";
		case Error::NotLoaded: return "NotLoaded";
		case Error::MHInit: return "MHInit";
		case Error::FindPattern_loading: return "FindPattern_loading";
		case Error::FindPattern_calling: return "FindPattern_calling";
		case Error::CreateHook_loading: return "CreateHook_loading";
		case Error::CreateHook_calling: return "CreateHook_calling";
		case Error::ApplyHook: return "ApplyHook";
		default: return "Unknown";
		}
	}

	typedef struct NezuStatus_t {
		DWORD magic = 'nezu';
		BOOL loaded = FALSE;
		BOOL initialized = FALSE;
		Error error = Error::NotLoaded;
		int num_blocked_modules = 0;
	} NeuzStatus;

}