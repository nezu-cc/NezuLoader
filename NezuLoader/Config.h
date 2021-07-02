#pragma once

#define save_val_t(j, obj, name) j[#name] = obj.name;
#define save_val_direct(j, obj) j[#obj] = obj;

#define read_val_t(j, obj, name) if(!j[#name].is_null()) { decltype(obj.name) tmp = j[#name]; obj.name = tmp; }
#define read_val_def(j, obj, name, def) if(!j[#name].is_null()) { decltype(obj.name) tmp = j[#name]; obj.name = tmp; } else { obj.name = def; }
#define read_val_direct(j, obj) if(!j[#obj].is_null()) { decltype(obj) tmp = j[#obj]; obj = tmp; }
#define read_val_full(j, key, val, def) if(!j[key].is_null()) { decltype(val) tmp = j[key]; val = tmp; } else { val = def; }

#define def_val(struct_name, type, name, def) type name = def;
#define def_val_struct(parrent, name) struct name ## _t {
#define def_val_struct_end(parrent, name) } name;

#define save_val(struct_name, type, name, def) save_val_t(struct_name, struct_name##_s, name)
#define save_val_struct(parrent, name) { json name; auto& name##_s = parrent##_s.name;
#define save_val_struct_end(parrent, name) save_val_direct(parrent, name) }

#define read_val(struct_name, type, name, def) read_val_def(struct_name, struct_name##_s, name, def)
#define read_val_struct(parrent, name) { json name; read_val_direct(parrent, name); auto& name##_s = parrent##_s.name;
#define read_val_struct_end(parrent, name) }

#define cfg_struct_start(cfg_mode) cfg_mode##_val_struct
#define cfg_struct_end(cfg_mode) cfg_mode##_val_struct_end
#define cfg_val(cfg_mode) cfg_mode##_val

//why? so i can change one line and it will auto-update the struct, save and load code
//hard to read? feel free to refactor it
#define full_config(cfg_mode)\
	cfg_struct_start(cfg_mode)(root, lobby)\
		cfg_struct_start(cfg_mode)(lobby, fake)\
			cfg_val(cfg_mode)(fake, bool, fake_prime, false)\
		cfg_struct_end(cfg_mode)(lobby, fake)\
		cfg_struct_start(cfg_mode)(lobby, auto_queue)\
			cfg_val(cfg_mode)(auto_queue, bool, enabled, false)\
			cfg_val(cfg_mode)(auto_queue, int, min_players, 0)\
			cfg_val(cfg_mode)(auto_queue, bool, auto_disconnect, false)\
			cfg_val(cfg_mode)(auto_queue, bool, auto_accept, false)\
			cfg_val(cfg_mode)(auto_queue, bool, detect_auto_accept, false)\
		cfg_struct_end(cfg_mode)(lobby, auto_queue)\
		cfg_struct_start(cfg_mode)(lobby, auto_invite)\
			cfg_val(cfg_mode)(auto_invite, bool, enabled, false)\
			cfg_val(cfg_mode)(auto_invite, std::vector<uint64_t>, steam_ids, std::vector<uint64_t>())\
			cfg_val(cfg_mode)(auto_invite, bool, auto_join, false)\
			cfg_struct_start(cfg_mode)(auto_invite, mass_invite)\
				cfg_val(cfg_mode)(mass_invite, bool, prime, false)\
				cfg_val(cfg_mode)(mass_invite, LobbyGameType, game_type, LobbyGameType::COMP)\
				cfg_val(cfg_mode)(mass_invite, int, avg_rank, 10)\
			cfg_struct_end(cfg_mode)(auto_invite, mass_invite)\
		cfg_struct_end(cfg_mode)(lobby, auto_invite)\
	cfg_struct_end(cfg_mode)(root, lobby)\
	cfg_struct_start(cfg_mode)(root, misc)\
		cfg_val(cfg_mode)(misc, bool, ow_reveal, false)\
		cfg_val(cfg_mode)(misc, bool, disable_auto_mute, false)\
		cfg_val(cfg_mode)(misc, bool, crasher, false)\
		cfg_val(cfg_mode)(misc, int, crasher_strength, 500)\
		cfg_val(cfg_mode)(misc, float, crasher_max_time, 20)\
		cfg_val(cfg_mode)(misc, bool, crasher_fix, false)\
		cfg_val(cfg_mode)(misc, bool, crasher_auto, false)\
		cfg_val(cfg_mode)(misc, bool, sv_pure_bypass, false)\
	cfg_struct_end(cfg_mode)(root, misc)\
	cfg_struct_start(cfg_mode)(root, visuals)\
		cfg_val(cfg_mode)(visuals, bool, remove_grass, false)\
		cfg_val(cfg_mode)(visuals, bool, flame_rainbow, false)\
		cfg_val(cfg_mode)(visuals, bool, flame_esp, false)\
		cfg_val(cfg_mode)(visuals, bool, flame_no_smoke, false)\
		cfg_val(cfg_mode)(visuals, ImColor, flame_esp_self, ImColor(106, 0, 255, 127))\
		cfg_val(cfg_mode)(visuals, ImColor, flame_esp_team, ImColor(0, 255, 0, 127))\
		cfg_val(cfg_mode)(visuals, ImColor, flame_esp_enemy, ImColor(255, 0, 0, 127))\
	cfg_struct_end(cfg_mode)(root, visuals)


void to_json(json & j, const ImColor & v);
void from_json(const json & j, ImColor & v);

enum class LobbyGameType {
	COMP = 0,
	WIGMAN,
	DANGER_ZONE,
};

namespace Cfg {

	struct cfg_global {
		std::vector<std::string> recent_dlls;
		std::string default_config;
	};

	struct cfg {
		full_config(def)
	};

	extern std::vector<std::string> configs;

	extern cfg_global global;
	extern cfg c;

	LPCWSTR GetPath(LPCSTR cfg_name, bool global = false);

	void RefreshList();
	bool LoadCurrentMods();
	bool LoadConfig(std::string name);
	bool SaveConfig(std::string name);
	bool DeleteConfig(std::string name);

	bool LoadGlobalConfig();
	bool SaveGlobalConfig();
};

