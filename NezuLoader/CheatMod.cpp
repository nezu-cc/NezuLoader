#include "pch.h"
#include "CheatMod.h"

BytePatch::BytePatch(DWORD offset, std::vector<BYTE> bytes) {
	this->offset = offset;
	this->bytes = bytes;
	applied = false;
}

BytePatch::BytePatch(DWORD offset, std::vector<BYTE> bytes, DWORD addr_offset, void* addr) : BytePatch(offset, bytes) {
	BYTE* dwAddr = (BYTE*)&addr;
	for (DWORD i = 0; i < sizeof(void*); i++) {
		this->bytes[addr_offset++] = *(dwAddr++);
	}
}

void BytePatch::Init(void* base) {
	this->base = base;
	orig_bytes.resize(bytes.size());
	memcpy(orig_bytes.data(), (void*)((DWORD)base + offset), bytes.size());
}

void BytePatch::Apply() {
	if(!applied)
		memcpy((void*)((DWORD)base + offset), bytes.data(), bytes.size());
	applied = true;
}

void BytePatch::Revert() {
	if(applied)
		memcpy((void*)((DWORD)base + offset), orig_bytes.data(), bytes.size());
	applied = false;
}

void InstantBytePatch::Init(void* base) {
	this->base = base;
	orig_bytes.resize(bytes.size());
}

void InstantBytePatch::Apply() {
	memcpy(orig_bytes.data(), (void*)((DWORD)base + offset), bytes.size());
	BytePatch::Apply();
}

CheatMod::CheatMod(std::string name, std::string displayName, std::vector<BaseMod*> mods) {
	this->name = name;
	this->displayName = displayName;
	this->mods = mods;
}

CheatMod::CheatMod(std::string displayName, std::vector<BaseMod*> mods) : CheatMod("", displayName, mods) { }

void CheatMod::Init(void* base, bool apply) {
	for (auto& mod : mods)
		mod->Init(base);
	if (apply) {
		for (auto& mod : mods)
			mod->Apply();
		applied = true;
	}
}

void CheatMod::Apply() {
	for (auto& mod : mods)
		mod->Apply();
	applied = true;
}

void CheatMod::Revert() {
	for (auto& mod : mods)
		mod->Revert();
	applied = false;
}

void CheatMod::Apply(bool apply) {
	apply ? Apply() : Revert();
}
