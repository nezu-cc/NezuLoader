#include "pch.h"
#include "Menu.h"
#include "RobotoBold.h"
#include "Hooks.h"
#include "icons.h"

bool Menu::open = true;
bool Menu::opening = true;
Textures Menu::textures;
int Menu::selectedTab = 2;

static ImVec4 color_dark_accent;
static ImVec4 color_light_accent;
static ImVec4 color_disabled;
static ImVec4 color_danger;
static ImVec4 color_danger_light;
static ImVec4 color_info;
static ImVec4 color_warning;

void Menu::init(IDirect3DDevice9* pDevice) {

	ImGui::CreateContext();

	ImGui_ImplWin32_Init(M::Window);
	ImGui_ImplDX9_Init(pDevice);

    auto& style = ImGui::GetStyle();
    style.WindowPadding = { 6,6 };
    style.FramePadding = { 6,3 };
    style.CellPadding = { 6,3 };
    style.ItemSpacing = { 6,6 };
    style.ItemInnerSpacing = { 6,6 };
    style.ScrollbarSize = 8;
    style.GrabMinSize = 8;
    style.WindowBorderSize = style.PopupBorderSize = 1;
    style.FrameBorderSize = style.ChildBorderSize = style.TabBorderSize = 0;
    style.WindowRounding = 10;
    style.ChildRounding = style.PopupRounding = style.ScrollbarRounding = style.GrabRounding = style.TabRounding = 4;

    ImVec4 dark_accent = color_dark_accent = ImVec4(0.416f, 0.000f, 1.000f, 1.000f);
    ImVec4 light_accent = color_light_accent = ImVec4(0.691f, 0.484f, 0.973f, 1.000f);
    color_disabled = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
    color_danger = ImColor(200, 0, 0);
    color_danger_light = [](const ImVec4& c) {
        float h, s, v;
        ImGui::ColorConvertRGBtoHSV(c.x, c.y, c.z, h, s, v);
        s *= 0.8f;
        v *= 1.1f;//i'm bad at colors, ok...
        return ImColor::HSV(h, s, v);
    }(color_danger);
    color_info = ImColor(82, 179, 18);
    color_warning = ImColor(255, 135, 25);

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(0.89f, 0.89f, 0.92f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.38f, 0.45f, 0.64f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.20f, 0.21f, 0.27f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.20f, 0.21f, 0.27f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.173f, 0.184f, 0.235f, 1.00f);
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

    //override
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
    colors[ImGuiCol_Border] = ImVec4(0.42f, 0.00f, 1.00f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.17f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.40f, 0.45f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.60f, 0.64f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.22f, 0.23f, 0.29f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.f, 0.0f, 0.0f, 0.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    auto& io = ImGui::GetIO();
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.IniFilename = NULL;
    io.Fonts->Clear();
    ImFontConfig font_cfg;
    font_cfg.PixelSnapH = true;
    font_cfg.OversampleH = 1;
    font_cfg.OversampleV = 1;
    font_cfg.FontDataOwnedByAtlas = false;
    strcpy(font_cfg.Name, "Roboto Bold");
    io.Fonts->AddFontFromMemoryCompressedTTF(RobotoBold_compressed_data, RobotoBold_compressed_size, 15.0f, &font_cfg);

}

void Menu::uninit() {

    RELEASE_TEXTURE(textures.reload);
    RELEASE_TEXTURE(textures.player_avatar);
    //ImGui_ImplDX9_Shutdown(); // this will destroy the device, don't do it
    ImGui_ImplDX9_InvalidateDeviceObjects(); // just release rextures
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

}

void Menu::Draw(IDirect3DDevice9* pDevice) {
    if (opening) {
        opening = false;
        ImGui::ResetCustomAnimations();
    }
    if (!textures.reload) {
        if (!CreateReloadIcon(pDevice, &textures.reload))
            textures.reload = NULL;
    }

    ImGui::SetColorEditOptions(ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoOptions);

    //main window
    const ImVec2 wnd_size = ImVec2(500, 400);
    ImGui::SetNextWindowSize(wnd_size, ImGuiCond_Appearing);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::SetNextWindowPos(ImGui::GetIO().DisplaySize / 2, ImGuiCond_Once, ImVec2(0.5, 0.5));
    ImGui::Begin("Nezu Loader", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize); {
        ImGui::PopStyleVar();//ImGuiStyleVar_WindowPadding
        
        ImGuiWindow* main_wnd = ImGui::GetCurrentWindow();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::Columns(2, 0, false);
        ImGui::SetColumnWidth(-1, 130);
        //ImGui::GetCurrentWindow()->DC.CurrentColumns->Flags |= ImGuiColumnsFlags_NoResize;
        static const char* menus[] = { "Loader", "Mods", "Tools"/*, "Players" */};
        const ImVec2 size(ImGui::GetContentRegionAvailWidth(), 40);
        for (int i = 0; i < IM_ARRAYSIZE(menus); i++)
            if (ImGui::AnimatedMenuButton(menus[i], size, selectedTab == i)) selectedTab = i;
        ImGui::PopStyleVar();//ImGuiStyleVar_ItemSpacing

        //restore window padding
        main_wnd->WindowPadding = ImGui::GetStyle().WindowPadding;
        ImGui::Indent(main_wnd->WindowPadding.x);
        main_wnd->ContentRegionRect.Min.x += main_wnd->WindowPadding.x;
        main_wnd->ContentRegionRect.Max.x -= main_wnd->WindowPadding.x;
        main_wnd->WorkRect.Min.x += main_wnd->WindowPadding.x;
        main_wnd->WorkRect.Max.x -= main_wnd->WindowPadding.x;
        main_wnd->DC.CursorPos.y += ImGui::GetStyle().ItemSpacing.y;

        //vertical line
        main_wnd->DrawList->AddLine(ImVec2(main_wnd->ClipRect.Max.x - 1, main_wnd->ClipRect.Min.y), main_wnd->ClipRect.Max + ImVec2(-1, 0), ImGui::GetColorU32(ImGuiCol_Separator));

        const ImVec2 old_wnd_pad = ImGui::GetStyle().WindowPadding;
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 6));
        if (ImGui::BeginChildWithTitle("Configs")) {
            ImGui::PushItemWidth(-1);
            ImGui::GetCurrentWindow()->DC.CursorPos.y -= main_wnd->WindowPadding.y;
            static unsigned int selected_config_index = [](){
                if (!Cfg::global.default_config.empty()) {
                    auto it = std::find(Cfg::configs.begin(), Cfg::configs.end(), Cfg::global.default_config);
                    if (it != Cfg::configs.end())
                        return it - Cfg::configs.begin();
                }
                return 0;
            }();
            static std::string configname = Cfg::configs.size() > 0 ? Cfg::configs[selected_config_index] : "";
            static float list_size = 30;
            const ImVec2 avail = ImGui::GetContentRegionAvail();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
            if (ImGui::ListBoxHeader("##configfiles", ImVec2(avail.x, avail.y - list_size))) {
                ImGuiListClipper clipper(Cfg::configs.size(), ImGui::GetTextLineHeightWithSpacing());
                while (clipper.Step()) {
                    for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                        const bool item_selected = i == selected_config_index;
                        ImGui::PushID(i);
                        bool is_def = Cfg::configs[i] == Cfg::global.default_config;
                        if (is_def)
                            ImGui::PushStyleColor(ImGuiCol_Text, color_info);
                        if (ImGui::Selectable(Cfg::configs[i].c_str(), item_selected)) {
                            selected_config_index = i;
                            if (selected_config_index >= 0 && selected_config_index < (int)Cfg::configs.size())
                                configname = Cfg::configs[selected_config_index];
                        }
                        if (is_def)
                            ImGui::PopStyleColor();
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, old_wnd_pad);
                        if (ImGui::BeginPopupContextItem("item context menu")) {
                            if (ImGui::Selectable("Set as default")) {
                                Cfg::global.default_config = Cfg::configs[i];
                                Cfg::SaveGlobalConfig();
                            }
                            ImGui::EndPopup();
                        }
                        ImGui::PopStyleVar();
                        if (item_selected) 
                            ImGui::SetItemDefaultFocus();
                        ImGui::PopID();
                    }
                }
                ImGui::ListBoxFooter();
            }
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            list_size = 0;
            ImGuiStyle style = ImGui::GetStyle();
            ImGui::InputText("##selectedconfigname", &configname);
            list_size += ImGui::GetItemRectSize().y + style.ItemSpacing.y;

            G::debug |= configname == "enable debug";

            static float buttonH = 20;
            if (ImGui::Button("Load", ImVec2(ImGui::GetContentRegionAvailWidth() - (textures.reload != NULL ? buttonH + style.ItemSpacing.x : 0), 0))) {
                if (selected_config_index < Cfg::configs.size())
                    Cfg::LoadConfig(Cfg::configs[selected_config_index]);
            }

            ImGui::SameLine();
            buttonH = ImGui::GetCurrentWindow()->DC.CurrLineSize.y;
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
            if (ImGui::ImageButton(textures.reload, ImVec2(buttonH, buttonH) - ImVec2(3, 3) * 2))
                Cfg::RefreshList();
            ImGui::PopStyleVar();
            list_size += ImGui::GetItemRectSize().y + style.ItemSpacing.y;

            if (ImGui::Button("Save", ImVec2(ImGui::GetContentRegionAvailWidth() / 2, 0))) {
                Cfg::SaveConfig(configname);
                Cfg::RefreshList();
            }

            ImGui::SameLine();
            if (ImGui::Button("Delete", ImVec2(ImGui::GetContentRegionAvailWidth(), 0))) {
                Cfg::DeleteConfig(configname);
                configname = "";
                Cfg::RefreshList();
            }
            list_size += ImGui::GetItemRectSize().y + style.ItemSpacing.y;

            ImGui::PopItemWidth();
        }
        else {
            ImGui::PopStyleColor();
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();

        //restore window padding again
        main_wnd->WindowPadding = ImGui::GetStyle().WindowPadding;
        ImGui::Indent(main_wnd->WindowPadding.x);
        main_wnd->WorkRect.Min.x -= main_wnd->WindowPadding.x;
        main_wnd->WorkRect.Max.x += main_wnd->WindowPadding.x;
        main_wnd->ContentRegionRect.Min.x -= main_wnd->WindowPadding.x;
        main_wnd->ContentRegionRect.Max.x += main_wnd->WindowPadding.x;

        ImGui::NextColumn();

        main_wnd->DC.CursorPos.x -= 1;
        main_wnd->DC.Indent.x -= 1;
        main_wnd->DC.CursorPos.y += main_wnd->WindowPadding.y;
        
        switch (selectedTab) {
        case 0:
            DrawLoaderTab(pDevice);
            break;
        case 1:
            DrawModsTab(pDevice);
            break;
        case 2:
            DrawToolsTab(pDevice);
            break;
        default:
            ImGui::Text("WIP");
            break;
        }

        ImGui::Columns();
        ImGui::End();
    }
    
    if (G::debug) {
        ImGui::SetNextWindowPos(ImGui::GetIO().DisplaySize, ImGuiCond_Appearing, ImVec2(1, 1));
        ImGui::Begin("Debug", NULL, ImGuiWindowFlags_AlwaysAutoResize); {
            if (ImGui::Button("unload"))
                G::unload = true;
            if (ImGui::Button("aim botz"))
                I::Engine->ClientCmd_Unrestricted("map workshop\\243702660\\aim_botz");
            if (ImGui::Button("Debug Magic :0")) {
            }

            ImGui::End();
        }

        ImGui::SetNextWindowPos(ImGui::GetIO().DisplaySize* ImVec2(0.5, 0), ImGuiCond_Appearing, ImVec2(0.5, 0));
        ImGui::SetNextWindowCollapsed(true, ImGuiCond_Appearing);
        ImGui::ShowDemoWindow();
    }

}

void Menu::DrawLoaderTab(IDirect3DDevice9* pDevice) {

    ImGuiStyle& style = ImGui::GetStyle();

    const ImVec2 size1 = ImVec2((500 - 130) / 2, 400 / 2 - ImGui::GetCurrentWindow()->TitleBarRect().GetHeight() / 2) - (style.WindowPadding + style.ItemSpacing / 2);
    constexpr float window_offset = 20.0f;
    if (ImGui::BeginChildWithTitle("Injector", size1 + ImVec2(window_offset, 0))) {
        static int selected_file = 0;
        ImGui::Text("Recent files:");
        static float button_size = 10.0f;//some inital value
        if (ImGui::ListBoxHeader("##recent_files", ImGui::GetContentRegionAvail() - ImVec2(0, button_size))) {
            ImGuiListClipper clipper(Cfg::global.recent_dlls.size(), ImGui::GetTextLineHeightWithSpacing());
            int id_to_remove = -1;
            while (clipper.Step()) {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                    ImGui::PushID(i);
                    auto& path = Cfg::global.recent_dlls[i];
                    std::string filename(path.substr(path.rfind("\\") + 1));
                    if (ImGui::Selectable(filename.c_str(), i == selected_file))
                        selected_file = i;
                    if (ImGui::BeginPopupContextItem("item context menu")) {
                        if (ImGui::Selectable("Remove from recent"))
                            id_to_remove = i;
                        ImGui::EndPopup();
                    }
                    ImGui::PopID();
                }
            }
            ImGui::ListBoxFooter();
            if (id_to_remove != -1) {
                Cfg::global.recent_dlls.erase(Cfg::global.recent_dlls.begin() + id_to_remove);
                Cfg::SaveGlobalConfig();
            }
        }
        ImGui::PushItemWidth(-1);

        //FIXME: log
        if (selected_file < (int)Cfg::global.recent_dlls.size() && selected_file >= 0) {
            if (ImGui::Button("Load Selected")) {
                std::string path = Cfg::global.recent_dlls[selected_file];
                Cfg::global.recent_dlls.erase(Cfg::global.recent_dlls.begin() + selected_file);
                Cfg::global.recent_dlls.insert(Cfg::global.recent_dlls.begin(), path);
                selected_file = 0;
                Cfg::SaveGlobalConfig();
                Inject(UTF8WCHAR(path).c_str());
            }
            ImGui::SameLine();
        }
        else selected_file = 0;
        if (ImGui::Button("Load New", ImVec2(-1, 0))) {
            std::string selected_file_path = DllFilePicker(M::Window);
            if (selected_file_path.size() > 0) {
                if (Inject(UTF8WCHAR(selected_file_path).c_str()) == TRUE) {
                    auto itr = std::find(Cfg::global.recent_dlls.begin(), Cfg::global.recent_dlls.end(), selected_file_path);
                    if (itr != Cfg::global.recent_dlls.end())
                        Cfg::global.recent_dlls.erase(itr);
                    Cfg::global.recent_dlls.insert(Cfg::global.recent_dlls.begin(), selected_file_path);
                    selected_file = 0;
                    Cfg::SaveGlobalConfig();
                }
            }
        }
        button_size = ImGui::GetItemRectSize().y + style.ItemSpacing.y;
        ImGui::PopItemWidth();
    }
    ImGui::EndChild();

    ImGui::SameLine();
    if (ImGui::BeginChildWithTitle("Injector status", size1 - ImVec2(window_offset, 0))) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 2));
        static const char* statuses[] = { "Error", "NotLoaded", "Mapping" , "Mapped" };
        ImGui::TextWrapped("Load status: %s", statuses[(int)NezuLoader::cheat->LoadState + 1]);
        ImGui::TextWrapped("Name: %s", NezuLoader::cheat->name);
        ImGui::TextWrapped("Mod: %s", NezuLoader::cheat->mod ? NezuLoader::cheat->mod->GetName() : "None");
        ImGui::TextWrapped("CRC32: 0x%X", NezuLoader::cheat->crc32);
        ImGui::TextWrapped("File: %s", NezuLoader::cheat->filepath.c_str());
        ImGui::PopStyleVar();
    }
    ImGui::EndChild();

    if (ImGui::BeginChildWithTitle("Log", ImVec2(size1.x * 2 + style.ItemSpacing.x, size1.y), ImGuiWindowFlags_None)) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

        //if (copy_to_clipboard) //TODO: add log coppy
            //ImGui::LogToClipboard();

        size_t count = L::messages.size();
        size_t start = 0;
        if (count > 500) {
            ImGui::TextColored(color_warning, "[!] only showing last 500 messages");
            start = count - 500;
        }
        for (size_t i = start; i < count; i++) {
            auto& message = L::messages[i];
            switch (std::get<0>(message)) {
                case MessageType::Debug: ImGui::TextColored(color_disabled, "[*]"); break;
                case MessageType::Info: ImGui::TextColored(color_info, "[+]"); break;
                case MessageType::Warning: ImGui::TextColored(color_warning, "[!]"); break;
                case MessageType::Error: ImGui::TextColored(color_danger, "[-]"); break;
                default: ImGui::Text("[?]"); break;
            }
            ImGui::SameLine();
            ImGui::TextWrapped("%s", std::get<1>(message).c_str());
        }

        //if (copy_to_clipboard)
            //ImGui::LogFinish();

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::PopStyleVar();
    }
    ImGui::EndChild();

}

void Menu::DrawModsTab(IDirect3DDevice9* pDevice) {
    ImGuiStyle& style = ImGui::GetStyle();

    if (NezuLoader::cheat->mod) {
        const ImVec2 size1 = ImVec2((500 - 130) / 2, 400 / 2 - ImGui::GetCurrentWindow()->TitleBarRect().GetHeight() / 2) - style.WindowPadding;
        if (ImGui::BeginChildWithTitle(NezuLoader::cheat->mod->GetName(), ImVec2(size1.x * 2, size1.y * 2))) {
            const ImVec2 size2 = ImGui::GetContentRegionAvail() * ImVec2(1.0f, 0.5f) - ImVec2(0, style.ItemSpacing.y / 2);
            if (ImGui::BeginChildWithTitle("Mods", size2)) {
                for (CheatMod* mod : NezuLoader::cheat->mod->GetMods()) {
                    bool val = mod->applied;
                    if (ImGui::Checkbox(mod->displayName.c_str(), &val))
                        mod->Apply(val);
                }
            }
            ImGui::EndChild();

            if (ImGui::BeginChildWithTitle("Patches", size2)) {
                for (CheatMod* patch : NezuLoader::cheat->mod->GetPatches()) {
                    ImGui::TextColored(patch->applied ? color_info : color_danger, "%s", patch->displayName.c_str());
                }
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();
    }
    else {
        ImGui::Text("Load a supported cheat to enabled mods");
        ImGui::Text("Supported cheats:");
        ImGui::BulletText("OTC3 (onetap crack v3)");
    }

}

void Menu::DrawToolsTab(IDirect3DDevice9* pDevice) {

    if (ImGui::BeginTabBar("ToolCat")) {
        if (ImGui::BeginTabItem("Lobby & matchmaking")) {
            ImGui::GetCurrentWindow()->DC.CursorPos.y -= ImGui::GetStyle().ItemSpacing.y;//no padding from top
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
            if (ImGui::BeginChild("###tools_tab_cont", ImGui::GetContentRegionAvail())) {
                ImGui::PopStyleColor();
                ImGui::GetCurrentWindow()->DC.CursorPos.y += ImGui::GetStyle().ItemSpacing.y;//add padding back

                const float avail_w = ImGui::GetContentRegionAvail().x - 2;

                static std::vector<CSteamID> panding_invites;

                MatchSession* match_session = I::IMatchFramework->get_match_session();
                CSteamID lobby_id;
                if (match_session) {
                    lobby_id.SetFromUint64(match_session->get_lobby_id());
                }

                {//auto_queue
                    static float w_heigth = 0;
                    ImGui::BeginChildWithTitle("Auto queue", ImVec2(avail_w, w_heigth)); {
                        ImGui::Checkbox("Enabled", &Cfg::c.lobby.auto_queue.enabled);
                        ImGui::SliderInt("Min players", &Cfg::c.lobby.auto_queue.min_players, 0, 5, Cfg::c.lobby.auto_queue.min_players == 0 ? "Full lobby" : "%d");
                        ImGui::Checkbox("Auto disconnect", &Cfg::c.lobby.auto_queue.auto_disconnect);
                        ImGui::Checkbox("Auto accept", &Cfg::c.lobby.auto_queue.auto_accept);
                        ImGui::Checkbox("Detect auto accept", &Cfg::c.lobby.auto_queue.detect_auto_accept);
                        if (Cfg::c.lobby.auto_queue.detect_auto_accept && !Cfg::c.lobby.auto_queue.auto_accept) {
                            ImGui::SameLine(); HelpMarker("Auto accept detection requires auto accept to be enabled!", color_danger, "(!!!)");
                        }
                    }
                    w_heigth = ImGui::GetCursorPos().y;
                    ImGui::EndChild();
                }
                {//auto_invite
                    static float w_heigth = 0;
                    static float last_table_h;
                    float table_h = 0;
                    ImGui::BeginChildWithTitle("Auto invite", ImVec2(avail_w, w_heigth)); {
                        size_t num_players_in_lobby = lobby_id.IsLobby() ? I::SteamMatchmaking->GetNumLobbyMembers(lobby_id) : 0;
                        std::vector<CSteamID> players_in_lobby;
                        for (size_t i = 0; i < num_players_in_lobby; i++) {
                            players_in_lobby.push_back(I::SteamMatchmaking->GetLobbyMemberByIndex(lobby_id, i));
                        }

                        auto& ids = Cfg::c.lobby.auto_invite.steam_ids;
                        const float table_start_y = ImGui::GetCursorPos().y;
                        if (ImGui::BeginTable("auto_invite_steam_ids", 2, ImGuiTableFlags_Borders)) {
                            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoResize);
                            ImGui::TableSetupColumn("actions", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize);
                            ImGui::TableHeadersRow();
                            int i = 0;
                            for (uint64_t id : ids) {
                                ImGui::PushID(i);
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                CSteamID steam_id(id);
                                bool in_lobby = vector_contains(players_in_lobby, steam_id);
                                I::SteamFriends->RequestUserInformation(steam_id, true);
                                const char* name = I::SteamFriends->GetFriendPersonaName(steam_id);
                                if (in_lobby)
                                    ImGui::PushStyleColor(ImGuiCol_Text, color_info);
                                ImGui::Text(name);
                                if (in_lobby)
                                    ImGui::PopStyleColor();
                                ImGui::TableSetColumnIndex(1);
                                if (ImGui::SmallButton("Invite")) {
                                    CreateOnlineLobby();
                                    invitePlayer(steam_id.ConvertToUint64());
                                }
                                ImGui::SameLine();
                                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
                                if (ImGui::SmallButton("X")) {
                                    ids.erase(ids.begin() + i);
                                }
                                ImGui::PopStyleVar();
                                ImGui::PopID();
                                i++;
                            }
                            ImGui::EndTable();
                        }
                        table_h = ImGui::GetCursorPos().y - table_start_y;
                        if (table_h != 0)
                            last_table_h = table_h;

                        static std::string add_text = "";
                        static const std::regex friend_code_regex("(?:[ABCDEFGHJKLMNPQRSTUVWXYZ23456789]{4}-)?[ABCDEFGHJKLMNPQRSTUVWXYZ23456789]{5}-[ABCDEFGHJKLMNPQRSTUVWXYZ23456789]{4}");
                        static const std::regex steam_id_regex("76\\d{15}");
                        std::smatch f_match;
                        std::regex_search(add_text, f_match, friend_code_regex);
                        std::smatch i_match;
                        std::regex_search(add_text, i_match, steam_id_regex);
                        const bool matched = f_match.size() == 1 || i_match.size() == 1;
                        ImGui::PushStyleColor(ImGuiCol_Text, matched ? color_info : color_danger);
                        ImGui::InputText("##add_auto_invite", &add_text, ImGuiInputTextFlags_AutoSelectAll);
                        ImGui::PopStyleColor();
                        ImGui::SameLine();
                        if (ImGui::Button("Add")) {
                            uint64_t id = 0;
                            if (f_match.size() == 1)
                                id = DecodeFriendCode(f_match[0]);
                            else if (i_match.size() == 1)
                                id = std::stoull(i_match[0]);

                            if (id != 0) {
                                add_text = "";
                                if (!vector_contains(ids, id))//add only if it dosn't exist;
                                    ids.push_back(id);
                            }
                        }
                        ImGui::SameLine();
                        HelpMarker("This can be either CS:GO friend code or steamID64", color_disabled);
                        if (ImGui::Button("Invite missing", ImVec2(-1, 0))) {
                            CreateOnlineLobby();
                            for (uint64_t id : ids) {
                                CSteamID steam_id(id);
                                if (!vector_contains(players_in_lobby, steam_id)) {
                                    invitePlayer(steam_id.ConvertToUint64());
                                }
                            }
                        }
                        ImGui::Checkbox("Invite all after auto disconnect", &Cfg::c.lobby.auto_invite.enabled);
                        ImGui::Checkbox("Auto accept all lobby invites", &Cfg::c.lobby.auto_invite.auto_join);
                    }
                    w_heigth = ImGui::GetCursorPos().y + (table_h == 0 ? last_table_h : 0);
                    ImGui::EndChild();
                }
                {//mass invite
                    static float w_heigth = 0;
                    ImGui::BeginChildWithTitle("Mass invite", ImVec2(avail_w, w_heigth)); {
                        auto& settings = Cfg::c.lobby.auto_invite.mass_invite;
                        ImGui::Text("Search settings:");
                        ImGui::Checkbox("Prime", &settings.prime);
                        ImGui::RadioButton("Competetive", (int*)&settings.game_type, (int)LobbyGameType::COMP); ImGui::SameLine();
                        ImGui::RadioButton("Wingman", (int*)&settings.game_type, (int)LobbyGameType::WIGMAN); ImGui::SameLine();
                        ImGui::RadioButton("Danger Zone", (int*)&settings.game_type, (int)LobbyGameType::DANGER_ZONE);
                        ImGui::SliderInt("Average rank", (int*)&settings.avg_rank, 0, 18, RankString(settings.avg_rank));
                        if (!GC::invite_all) {
                            if (ImGui::Button("Search & invite", ImVec2(-1, 0)))
                                GC::InviteAll();
                        }
                        else {
                            ImGui::PushStyleColor(ImGuiCol_Button, color_danger);
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color_danger_light);
                            if (ImGui::Button("Cancel", ImVec2(-1, 0)))
                                GC::invite_all = false;
                            ImGui::PopStyleColor(2);
                        }
                    }
                    w_heigth = ImGui::GetCursorPos().y;
                    ImGui::EndChild();
                }
                {//fake
                    static float w_heigth = 0;
                    ImGui::BeginChildWithTitle("Fake", ImVec2(avail_w, w_heigth)); {
                        ImGui::Checkbox("Fake prime", &Cfg::c.lobby.fake.fake_prime);
                    }
                    w_heigth = ImGui::GetCursorPos().y;
                    ImGui::EndChild();
                }

                if (G::debug) {
                    static float w_heigth = 0;
                    ImGui::BeginChildWithTitle("Debug", ImVec2(avail_w, w_heigth)); {
                        ImGui::Text("Lobby ID: %llu", lobby_id.ConvertToUint64());
                        if (lobby_id.IsLobby()) {
                            ImGui::Text("Cur Players(steam): %d", I::SteamMatchmaking->GetNumLobbyMembers(lobby_id));
                            ImGui::Text("Max Players(steam): %d", I::SteamMatchmaking->GetLobbyMemberLimit(lobby_id));
                        }
                        if (match_session) {
                            KeyValues* settings = match_session->get_session_settings();
                            const char* gamemode = settings->GetString("game/mode");
                            ImGui::Text("game/mode: \"%s\"", gamemode);
                            ImGui::Text("Max slots(game/mode): %d", GetMaxLobbySlotsForGameMode(gamemode));
                        }
                        if (*M::UiComponent_PartyList) {
                            ImGui::Text("Cur Players(UiComponent_PartyList): %d", (*M::UiComponent_PartyList)->GetCount());
                        }
                        if (*M::UiComponent_PartyBrowser) {
                            for (auto& invite : G::PendingInvites) {
                                ImGui::Text("Inv %llu: %d %llu", invite.first, (*M::UiComponent_PartyBrowser)->HasBeenInvited(invite.first),
                                    (int64_t)(invite.second + 10000) - (int64_t)GetTickCount64());
                            }
                        }
                    }
                    if (ImGui::Button("Accept match")) {
                        IUIEvent* e = CreatePanoramaEvent("MatchAssistedAccept");
                        I::PanoramaUIEngine->AccessUIEngine()->DispatchEvent(e);
                    }
                    w_heigth = ImGui::GetCursorPos().y;
                    ImGui::EndChild();
                }

                ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(0, ImGui::GetStyle().WindowPadding.y / 2));//fix broken padding
            }
            else ImGui::PopStyleColor();
            ImGui::EndChild();
            
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Misc")) {
            

            ImGui::GetCurrentWindow()->DC.CursorPos.y -= ImGui::GetStyle().ItemSpacing.y;//no padding from top
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
            if (ImGui::BeginChild("###tools_tab_cont", ImGui::GetContentRegionAvail())) {
                ImGui::PopStyleColor();
                ImGui::GetCurrentWindow()->DC.CursorPos.y += ImGui::GetStyle().ItemSpacing.y;//add padding back

                const float avail_w = ImGui::GetContentRegionAvail().x - 2;

                {//demo
                    static float w_heigth = 0;
                    ImGui::BeginChildWithTitle("Demo", ImVec2(avail_w, w_heigth)); {
                        ImGui::Checkbox("Overwatch revealer", &Cfg::c.misc.ow_reveal);
                    }
                    w_heigth = ImGui::GetCursorPos().y;
                    ImGui::EndChild();
                }

                {//automation
                    static float w_heigth = 0;
                    ImGui::BeginChildWithTitle("Automation", ImVec2(avail_w, w_heigth)); {
                        ImGui::Checkbox("Auto unmute all players", &Cfg::c.misc.disable_auto_mute);
                    }
                    w_heigth = ImGui::GetCursorPos().y;
                    ImGui::EndChild();
                }

                {//exploits
                    static float w_heigth = 0;
                    ImGui::BeginChildWithTitle("Exploits", ImVec2(avail_w, w_heigth)); {
                        static float start_time;
                        float time = (float)ImGui::GetTime();
                        if (ImGui::Checkbox("Crasher", &Cfg::c.misc.crasher))
                            start_time = time;
                        if (I::Engine->IsInGame() && Cfg::c.misc.crasher) {
                            ImGui::SameLine();
                            ImGui::Text("%.1f", time - start_time);
                            if (time - start_time > Cfg::c.misc.crasher_max_time)
                                Cfg::c.misc.crasher = false;
                        }
                        ImGui::SliderInt("Crasher strength", &Cfg::c.misc.crasher_strength, 1, 1000);
                        ImGui::SliderFloat("Crasher max time", &Cfg::c.misc.crasher_max_time, 1, 60, "%.1f");
                        if (Cfg::c.misc.crasher)
                            Cfg::c.misc.crasher_fix = true;
                        ImGui::Checkbox("Crasher fix", &Cfg::c.misc.crasher_fix);
                    }
                    w_heigth = ImGui::GetCursorPos().y;
                    ImGui::EndChild();
                }
                
                if (G::debug) {
                    static float w_heigth = 0;
                    ImGui::BeginChildWithTitle("Debug", ImVec2(avail_w, w_heigth)); {
                        static std::string js_textbox = "";
                        ImGui::Text("Execute javascript");
                        ImGui::InputText("js context", &G::ExecuteJsContext);
                        ImGui::InputTextMultiline("##js_run", &js_textbox, ImVec2(-1, 0));
                        if (ImGui::Button("Run", ImVec2(-1, 0)))
                            G::ExecuteJs = js_textbox;
                    }
                    w_heigth = ImGui::GetCursorPos().y;
                    ImGui::EndChild();
                }

                ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(0, ImGui::GetStyle().WindowPadding.y / 2));//fix broken padding
            }
            else ImGui::PopStyleColor();
            ImGui::EndChild();

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Visual")) {
            ImGui::GetCurrentWindow()->DC.CursorPos.y -= ImGui::GetStyle().ItemSpacing.y;//no padding from top
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
            if (ImGui::BeginChild("###tools_tab_cont", ImGui::GetContentRegionAvail())) {
                ImGui::PopStyleColor();
                ImGui::GetCurrentWindow()->DC.CursorPos.y += ImGui::GetStyle().ItemSpacing.y;//add padding back

                const float avail_w = ImGui::GetContentRegionAvail().x - 2;

                {//removals
                    static float w_heigth = 0;
                    ImGui::BeginChildWithTitle("Removals", ImVec2(avail_w, w_heigth)); {
                        ImGui::Checkbox("Remove grass", &Cfg::c.visuals.remove_grass);
                    }
                    w_heigth = ImGui::GetCursorPos().y;
                    ImGui::EndChild();
                }

                {//fire
                    static float w_heigth = 0;
                    ImGui::BeginChildWithTitle("Fire", ImVec2(avail_w, w_heigth)); {
                        ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs;
                        ImGui::Checkbox("Flame esp", &Cfg::c.visuals.flame_esp);
                        ImGui::SameLine(); ImGui::ColorEdit4("team", (float*)&Cfg::c.visuals.flame_esp_team.Value, flags);
                        ImGui::SameLine(); ImGui::ColorEdit4("enemy", (float*)&Cfg::c.visuals.flame_esp_enemy.Value, flags);
                        ImGui::SameLine(); ImGui::ColorEdit4("self", (float*)&Cfg::c.visuals.flame_esp_self.Value, flags);
                        ImGui::Checkbox("Rainbow flames", &Cfg::c.visuals.flame_rainbow);
                        ImGui::Checkbox("Remove smoke from flames", &Cfg::c.visuals.flame_no_smoke);
                    }
                    w_heigth = ImGui::GetCursorPos().y;
                    ImGui::EndChild();
                }

                ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(0, ImGui::GetStyle().WindowPadding.y / 2));//fix broken padding
            }
            else ImGui::PopStyleColor();
            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        if (G::debug && ImGui::BeginTabItem("DEBUG")) {
            ImGui::GetCurrentWindow()->DC.CursorPos.y -= ImGui::GetStyle().ItemSpacing.y;//no padding from top
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
            if (ImGui::BeginChild("###tools_tab_cont", ImGui::GetContentRegionAvail())) {
                ImGui::PopStyleColor();
                ImGui::GetCurrentWindow()->DC.CursorPos.y += ImGui::GetStyle().ItemSpacing.y;//add padding back

                const float avail_w = ImGui::GetContentRegionAvail().x - 2;

                {//memory
                    static float w_heigth = 0;
                    ImGui::BeginChildWithTitle("Memory", ImVec2(avail_w, w_heigth)); {

                        static MemoryEditor mem_edit_1;
                        mem_edit_1.ReadOnly = true;
                        static BYTE data[0x100];

                        static char AddrInputBuf[32] = "0";
                        if (ImGui::InputText("addr", AddrInputBuf, IM_ARRAYSIZE(AddrInputBuf), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue))
                        {
                            size_t goto_addr;
                            if (sscanf(AddrInputBuf, "%IX", &goto_addr) == 1) {
                                if (!IsBadReadPtr((void*)goto_addr, sizeof(data))) {
                                    memcpy(data, (void*)goto_addr, sizeof(data));
                                }
                            }
                        }

                        mem_edit_1.DrawWindow("MEM", data, sizeof(data));

                    }
                    w_heigth = ImGui::GetCursorPos().y;
                    ImGui::EndChild();
                }

                ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(0, ImGui::GetStyle().WindowPadding.y / 2));//fix broken padding
            }
            else ImGui::PopStyleColor();
            ImGui::EndChild();

            ImGui::EndTabItem();
        }


        ImGui::EndTabBar();
    }

}

static void HelpMarker(LPCSTR desc, const ImVec4& col, LPCSTR icon) {
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::Text(icon);
    ImGui::PopStyleColor(); //ImGuiCol_Text
    if (ImGui::IsItemHovered()) {
        ImGui::PushStyleColor(ImGuiCol_Border, col);
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
        ImGui::PopStyleColor();//ImGuiCol_Border
    }
}