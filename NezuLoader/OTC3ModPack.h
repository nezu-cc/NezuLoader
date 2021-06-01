#pragma once
#include "CheatMod.h"

class OTC3ModPack : public CheatModPack {
private:
	char watermark_text[0x100] = "otc3 (NezuLoader)";

	//legit aa while rageboting
	BytePatch menu_aa_ragebot_getter = BytePatch(0x5C120, { 0xB0, 0x00, 0xC3 });// mov al, 0; ret
	BytePatch menu_aa_fake_angles = BytePatch(0x5C2D7, { 0xEB });// je -> jmp
	BytePatch menu_aa_legit = BytePatch(0x5C287, { 0x90, 0x90 });// nop x2
	BytePatch legitbot_run_lbot_check = BytePatch(0x111FE0, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });// nop x6
	BytePatch legitbot_run_rbot_check = BytePatch(0x111FED, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });// nop x6
	BytePatch legit_aa_run_lbot_check = BytePatch(0x10F7BD, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });// nop x6
	BytePatch legit_aa_run_rbot_check = BytePatch(0x10F7CA, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });// nop x6
	BytePatch legit_aa_indicator_run_lbot_check = BytePatch(0x10D7B9, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });// nop x6
	BytePatch legit_aa_indicator_run_rbot_check = BytePatch(0x10D7C6, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });// nop x6
	BytePatch legit_aa_indicator_run_visuals = BytePatch(0x9A995, { 0x90, 0x90 });// nop x2
	BytePatch legit_aa_keybinds_visuals = BytePatch(0x81C78, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });// nop x6
	BytePatch rage_aa_run_cfg_check = BytePatch(0x79CFF, { 0x39, 0xC0, 0x90, 0x90, 0x90, 0x90, 0x90 });// cmp eax, eax; nop x5
	BytePatch rage_aa_manual_dir_disable = BytePatch(0x775B2, { 0x90, 0x90 });// nop x2
	CheatMod Mod_RageBotLegitAA = CheatMod("legit_aa", "Legit anti-aim while ragebot active", {
		&menu_aa_ragebot_getter, &menu_aa_fake_angles, &menu_aa_legit,          // aa menu
		&legitbot_run_lbot_check, &legitbot_run_rbot_check,                     // legit bot
		&legit_aa_run_lbot_check, &legit_aa_run_rbot_check,                     // legit aa
		&rage_aa_run_cfg_check, &rage_aa_manual_dir_disable,                    // rage aa
		&legit_aa_indicator_run_lbot_check, &legit_aa_indicator_run_rbot_check, // legit aa indicator
		&legit_aa_indicator_run_visuals, &legit_aa_keybinds_visuals,            // visuals
		});

	//don't reset menu position
	BytePatch menu_reset_position = BytePatch(0x179301, { 0xEB });// je -> jmp
	CheatMod Mod_MenuResetPositionDisable = CheatMod("menu_pos", "Don't reset onetap menu position", { &menu_reset_position });

	//bomb class id moved from 0x80 to 0x81
	BytePatch bomb_esp_max_defualt_case = BytePatch(0x8BF6D, { 0x57 });// 0x56 -> 0x57
	BytePatch bomb_esp_jumptable = BytePatch(0x8C11E, { 0x05, 0x04 });// shift 0x04 plus 1 in jump table
	BytePatch bomb_glow_jumptable = BytePatch(0x14FDBF, { 0x04, 0x03 });// shift 0x03 plus 1 in jump table
	CheatMod Patch_BombClassId = CheatMod("Bomb esp fix", {
		&bomb_esp_max_defualt_case,
		&bomb_esp_jumptable,
		&bomb_glow_jumptable
		});

	BytePatch watermark_cheat_name = BytePatch(0x82AC3, { 0xBA, 0x69, 0x69, 0x69, 0x69, 0x90, 0x90 }, 1, &watermark_text); // mox edx, &watermark_text; nop x2
	CheatMod Patch_Watermark = CheatMod("Watermark", { &watermark_cheat_name });

	std::vector<CheatMod*> patches = { &Patch_BombClassId, &Patch_Watermark };
	std::vector<CheatMod*> mods = { &Mod_RageBotLegitAA, &Mod_MenuResetPositionDisable };

	void IntiMods(void* base);
	void InitPatches(void* base);

public:

	explicit OTC3ModPack(void* base);
	virtual const char* GetName();
	virtual std::vector<CheatMod*>& GetPatches();
	virtual std::vector<CheatMod*>& GetMods();

};