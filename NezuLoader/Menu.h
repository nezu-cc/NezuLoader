#pragma once

typedef struct textures_t {
	LPDIRECT3DTEXTURE9 reload = NULL;
	LPDIRECT3DTEXTURE9 player_avatar = NULL;
} Textures;

extern const unsigned int reload_icon_compressed_size;
extern const unsigned int reload_icon_compressed_data[];

static void HelpMarker(LPCSTR desc, const ImVec4& col, LPCSTR icon = "(?)");

namespace Menu {

	extern bool open;
	extern bool opening;
	extern Textures textures;
	extern int selectedTab;

	void init(IDirect3DDevice9* pDevice);
	void uninit();
	void Draw(IDirect3DDevice9* pDevice);
	void DrawLoaderTab(IDirect3DDevice9* pDevice);
	void DrawModsTab(IDirect3DDevice9* pDevice);
	void DrawToolsTab(IDirect3DDevice9* pDevice);

};

