#include "pch.h"
#include "OTC3ModPack.h"

void OTC3ModPack::IntiMods(void* base) {
	Mod_RageBotLegitAA.Init(base);
	Mod_MenuResetPositionDisable.Init(base);
	watermark_cheat_name.Init(base);
	watermark_cheat_name.Apply();
}

void OTC3ModPack::InitPatches(void* base) {
	Patch_BombClassId.Init(base, true); 
	Patch_Watermark.Init(base, true);
}

const char* OTC3ModPack::GetName() {
	return "OTC3";
}

std::vector<CheatMod*>& OTC3ModPack::GetPatches() {
	return patches;
}

std::vector<CheatMod*>& OTC3ModPack::GetMods() {
	return mods;
}

OTC3ModPack::OTC3ModPack(void* base) {
	InitPatches(base);
	IntiMods(base);
}

/*

0x5C2D0 - menu fake angles section
0x5C280 - menu legit aa section
0x5C120 - rage bot master on getter func
0x10F7B0 - legit aa func
0x10D7BF - legit aa indicator arrow (ragebot check)

0x179280 - WndProc
0x829E0 - draw watermark

0xBC17B4 - legitbot
0xBC1E20 - ragebot
0xBC22E0 - legit aa
0xBC1F50 - rage aa
0xBC2D78 - bomb esp
0xBC2E20 - weapon esp
0xBC1FAC - rage aa manual dir

0x14FDC0 - c4 glow jump table

0x8477D - call lbot?
0x111FD0 - lbot

0xBC11E8 - menu object
0x179301 - reset menu position to center

*/