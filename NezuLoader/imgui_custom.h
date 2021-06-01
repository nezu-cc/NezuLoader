#pragma once
#include "imgui.h"

struct animatedbtn_t {
	float w = 0;
	float speed = 0;
	unsigned char mode = 0;
};

namespace ImGui {

	bool BeginChildWithTitle(const char* title, ImVec2 size = ImVec2(0, 0), ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	bool AnimatedMenuButton(const char* label, const ImVec2 size, bool selected, bool vertical = false);
	void ResetCustomAnimations();

}