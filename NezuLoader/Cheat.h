#pragma once
class Cheat {
public:

	enum class LoadState {
		Error = -1,
		NotLoaded = 0,
		Mapping,
		Mapped,
	};

	LoadState LoadState = LoadState::NotLoaded;
	CheatModPack* mod = NULL;
	DWORD crc32 = 0;
	const char* name = "unknown";
	std::string filepath;

};

