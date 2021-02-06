#include "pch.h"
#include "Menu.h"
#include "imgui/RobotoBold.h"

bool Menu::open = true;

void Menu::init(IDirect3DDevice9* pDevice) {

	ImGui::CreateContext();

	ImGui_ImplWin32_Init(M::Window);
	ImGui_ImplDX9_Init(pDevice);

    // green
    // ImVec4 dark_accent  = ImVec4(0.00f, 0.70f, 0.16f, 1.00f); 
    // ImVec4 light_accent = ImVec4(0.50f, 1.00f, 0.00f, 1.00f);

    // orange
    ImVec4 dark_accent = ImVec4(0.85f, 0.37f, 0.00f, 1.00f);
    ImVec4 light_accent = ImVec4(1.00f, 0.63f, 0.00f, 1.00f);

    // purple
    // ImVec4 dark_accent = ImVec4(0.416f, 0.000f, 1.000f, 1.000f);
    // ImVec4 light_accent = ImVec4(0.691f, 0.484f, 0.973f, 1.000f);

    auto& style = ImGui::GetStyle();
    style.WindowPadding = { 6,6 };
    style.FramePadding = { 6,3 };
    style.CellPadding = { 6,3 };
    style.ItemSpacing = { 6,6 };
    style.ItemInnerSpacing = { 6,6 };
    style.ScrollbarSize = 16;
    style.GrabMinSize = 8;
    style.WindowBorderSize = style.ChildBorderSize = style.PopupBorderSize = style.TabBorderSize = 0;
    style.FrameBorderSize = 1;
    style.WindowRounding = style.ChildRounding = style.PopupRounding = style.ScrollbarRounding = style.GrabRounding = style.TabRounding = 4;


    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(0.89f, 0.89f, 0.92f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.38f, 0.45f, 0.64f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.20f, 0.21f, 0.27f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.20f, 0.21f, 0.27f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.173f, 0.184f, 0.235f, 1.000f);
    colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.06f);
    colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.02f);
    colors[ImGuiCol_FrameBgHovered] = light_accent;
    colors[ImGuiCol_FrameBgActive] = light_accent;
    colors[ImGuiCol_TitleBg] = dark_accent;
    colors[ImGuiCol_TitleBgActive] = dark_accent;
    colors[ImGuiCol_TitleBgCollapsed] = dark_accent;
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.21f, 0.27f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.89f, 0.89f, 0.93f, 0.27f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.89f, 0.89f, 0.93f, 0.55f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    colors[ImGuiCol_CheckMark] = dark_accent;
    colors[ImGuiCol_SliderGrab] = dark_accent;
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    colors[ImGuiCol_Button] = dark_accent;
    colors[ImGuiCol_ButtonHovered] = light_accent;
    colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    colors[ImGuiCol_Header] = dark_accent;
    colors[ImGuiCol_HeaderHovered] = light_accent;
    colors[ImGuiCol_HeaderActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    colors[ImGuiCol_Separator] = dark_accent;
    colors[ImGuiCol_SeparatorHovered] = light_accent;
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = dark_accent;
    colors[ImGuiCol_ResizeGripHovered] = light_accent;
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(1.00f, 1.00f, 1.00f, 0.02f);
    colors[ImGuiCol_TabHovered] = light_accent;
    colors[ImGuiCol_TabActive] = dark_accent;
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    //colors[ImGuiCol_DockingPreview] = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);
    //colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    colors[ImGuiCol_PlotLines] = light_accent;
    colors[ImGuiCol_PlotLinesHovered] = light_accent;
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

    auto& io = ImGui::GetIO();
    io.Fonts->Clear();
    ImFontConfig font_cfg;
    font_cfg.PixelSnapH = true;
    font_cfg.OversampleH = 1;
    font_cfg.OversampleV = 1;
    font_cfg.FontDataOwnedByAtlas = false;
    strcpy(font_cfg.Name, "Roboto Bold");
    io.Fonts->AddFontFromMemoryCompressedTTF(RobotoBold_compressed_data, RobotoBold_compressed_size, 15.0f, &font_cfg);

}
