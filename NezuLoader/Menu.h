#pragma once

struct animatedbtn_t {
	float w = 0;
	float speed = 0;
	unsigned char mode = 0;
};

namespace Menu {

	extern bool open;

	void init(IDirect3DDevice9* pDevice);
	void uninit();
	void Draw();

};

