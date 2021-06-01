#include "pch.h"
#include "imgui_custom.h"

std::unordered_map<ImGuiID, animatedbtn_t> animated_buttons;

bool ImGui::BeginChildWithTitle(const char* title, ImVec2 size, ImGuiWindowFlags flags) {
	if (ImGui::BeginChild((std::string("child_") + title).c_str(), size, true, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysUseWindowPadding | flags)) {
		if (ImGui::BeginMenuBar()) {
			ImGuiStyle style = ImGui::GetStyle();
			ImGui::SetCursorPosX((ImGui::GetContentRegionAvailWidth() + style.WindowPadding.x * 2 - ImGui::CalcTextSize(title, 0, true).x) / 2);
			ImGui::Text("%s", title);
		}
		ImGui::EndMenuBar();
		return true;
	}
	return false;
}

bool ImGui::AnimatedMenuButton(const char* label, const ImVec2 size, bool selected, bool vertical) {
    ImGuiID id = ImGui::GetID((std::string(label) + "_back").c_str());
    ImGuiStyle style = ImGui::GetStyle();
    animatedbtn_t* b = &animated_buttons[id];
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    const auto pos = ImGui::GetCurrentWindow()->DC.CursorPos;
    const ImRect rect(pos, pos + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(rect, id)) return false;
    bool hovered;
    bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, 0);
    if (pressed) ImGui::MarkItemEdited(id);
    float delta = ImGui::GetIO().DeltaTime;
    float sizevar = vertical ? size.y : size.x;
    if (hovered || selected) {
        if (b->mode == 100) {
            b->mode = 0;
            b->speed = 15;
        }
        if (b->mode != 2) {
            b->speed += delta * 50;
            b->w += b->speed * delta * 100;
        }
        if (b->w > sizevar) {
            b->w = sizevar;
            b->speed *= b->mode == 0 ? -.3f : 0;
            b->mode++;
        }
    }
    else {
        if (b->mode != 100) b->speed = 0;
        b->mode = 100;
        b->speed -= delta * 50;
        b->w += b->speed * delta * 100;
        if (b->w < 0) {
            b->speed = 0;
            b->w = 0;
        }
    }
    const ImRect rect2(pos, pos + (vertical ? ImVec2(size.x, b->w) : ImVec2(b->w, size.y)));
    //ImGui::RenderNavHighlight(rect, id);
    ImGui::RenderFrame(rect2.Min, rect2.Max, ImGui::GetColorU32(selected ? ImGuiCol_Button : ImGuiCol_ButtonHovered), false, style.FrameRounding);
    ImGui::RenderTextClipped(rect.Min + style.FramePadding, rect.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &rect);

    return pressed;
}

void ImGui::ResetCustomAnimations() {
    ImGui::GetIO().DeltaTime = 0.f;
    //reset so they have a nice animation when opening the menu
    for (auto& item : animated_buttons) {
        animatedbtn_t* b = &animated_buttons[item.first];
        b->mode = 0;
        b->speed = 0.f;
        b->w = 0.f;
    }
}