#pragma once
#include "pch.h"

class BaseMod {
public:
	virtual void Init(void* base) = 0;
	virtual void Apply() = 0;
	virtual void Revert() = 0;
};

class BytePatch: public BaseMod {
public:
	void* base;
	DWORD offset;
	std::vector<BYTE> orig_bytes;
	std::vector<BYTE> bytes;
	bool applied;

	explicit BytePatch(DWORD offset, std::vector<BYTE> bytes);
	explicit BytePatch(DWORD offset, std::vector<BYTE> bytes, DWORD addr_offset, void* addr);
	virtual void Init(void* base);
	virtual void Apply();
	virtual void Revert();
};

class InstantBytePatch : public BytePatch {
public:
	using BytePatch::BytePatch;
	void Init(void* base);
	void Apply();
};

class CheatMod {
public:
	std::vector<BaseMod*> mods;
	std::string displayName;
	std::string name;
	explicit CheatMod(std::string name, std::string displayName, std::vector<BaseMod*> mods);
	explicit CheatMod(std::string displayName, std::vector<BaseMod*> mods);
	void Init(void* base, bool apply = false);
	void Apply();
	void Apply(bool apply);
	void Revert();
	bool applied = false;
};

class CheatModPack {
public:
	virtual const char* GetName() = 0;
	virtual std::vector<CheatMod*>& GetPatches() = 0;
	virtual std::vector<CheatMod*>& GetMods() = 0;
};