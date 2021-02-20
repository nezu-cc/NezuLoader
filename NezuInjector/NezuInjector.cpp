#include "pch.h"
#include "RobotoBold.h"
#include "Injector.h"
#include "Loader.h"
#include "NezuVacInterface.h"

//for debuging
//#define SHOW_DEMO

//causes problems on diffrent mashines, i dont want to have a "works on my mashine" type problems.
//You can try it out but dont distribute binaries with it enabled
//#define USE_BLURBEHIND

#define TOOLTIP_SIZE 25.0f
#define G_SCALE 1.4f
#define WINDOW_TITLE "Nezu Injector"
#define WINDOW_TITLE_INJECTION "Loading..."

#ifdef SHOW_DEMO
int w = 1200, h = 1000;
#else
int w = (int)(315 * G_SCALE), h = (int)(150 * G_SCALE);
#endif // ShOW_DEMO

static LPDIRECT3D9 g_pD3D = NULL;
static LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS g_d3dpp = {};

static HANDLE g_hLoaderThread = NULL;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL GetMessageWithTimeout(MSG* msg, UINT to);
void HelpMarker(LPCSTR desc, const ImVec4& col, LPCSTR icon = "(?)");

struct t_NezuInjectorStatus {
public:
    bool SteamActive = false;
    bool SteamServiceActive = false;
    bool CsgoActive = false;
    std::string NezuVacError;
    HANDLE SteamHandle = NULL;
    DWORD NezuVacStatusAddress = NULL;
    NezuVac::NeuzStatus NezuVacStatus = { 0, 0, 0, (NezuVac::Error)0, 0 };
    double LastChecked = -1.0;//-1 to force update on first frame
} status;

NezuInjectorSettings settings;

bool window_active = true;
int forced_frames = 0;

int main() {

#pragma region window

    WNDCLASSEX wc;// = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("NezuInjector"), NULL };
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(0);
    wc.lpszClassName = _T("NezuInjector");
	RegisterClassEx(&wc);

	RECT rect;
	GetClientRect(GetDesktopWindow(), &rect);
	rect.left = (rect.right / 2) - (w / 2);
	rect.top = (rect.bottom / 2) - (h / 2);

    //create window
	HWND hwnd = CreateWindow(wc.lpszClassName, _T(WINDOW_TITLE), WS_POPUP | WS_VISIBLE | WS_SYSMENU, rect.left, rect.top, w * 2, h * 2, NULL, NULL, wc.hInstance, NULL);
//#ifndef SHOW_DEMO
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(20, 20, 20), 0, LWA_COLORKEY);
//#endif // !SHOW_DEMO

    //DWM stuff
#ifdef USE_BLURBEHIND
    DWM_BLURBEHIND bb = { 0 };
    bb.dwFlags = DWM_BB_ENABLE;
    bb.fEnable = TRUE;
    bb.hRgnBlur = NULL;
    DwmEnableBlurBehindWindow(hwnd, &bb);
#endif // USE_BLURBEHIND
    BOOL fForceIconic = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_FORCE_ICONIC_REPRESENTATION, &fForceIconic, sizeof(fForceIconic));
    BOOL fHasIconicBitmap = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_HAS_ICONIC_BITMAP, &fHasIconicBitmap, sizeof(fHasIconicBitmap));

    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!g_pD3D) {
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0) {
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        g_pD3D->Release(); 
        g_pD3D = NULL;
        if (g_pd3dDevice) { 
            g_pd3dDevice->Release(); 
            g_pd3dDevice = NULL;
        }
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

#pragma endregion window and dx init

#pragma region font

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.IniFilename = NULL;
    io.Fonts->Clear();
    ImFontConfig font_cfg;
    font_cfg.PixelSnapH = false;
    font_cfg.OversampleH = 3;
    font_cfg.OversampleV = 1;
    font_cfg.FontDataOwnedByAtlas = false;
    strcpy(font_cfg.Name, "Roboto Bold");
    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
    for (int i = 0; i < builder.UsedChars.Size; i++) //add all ranges form the font
        builder.UsedChars[i] = 0xFFFFFFFF; //all the bits
    builder.UsedChars[0] &= ~(1 << 0); //trying to add the '\0' character will cause the font creation to fail so we remove it 
    builder.BuildRanges(&ranges);
    io.Fonts->AddFontFromMemoryCompressedTTF(RobotoBold_compressed_data, RobotoBold_compressed_size, 15.0f * G_SCALE, &font_cfg, ranges.Data);
    io.Fonts->Build();

#pragma endregion imgui font creation

#pragma region styles

    // green
    // ImVec4 dark_accent  = ImVec4(0.00f, 0.70f, 0.16f, 1.00f); 
    // ImVec4 light_accent = ImVec4(0.50f, 1.00f, 0.00f, 1.00f);

    // orange
    //ImVec4 dark_accent = ImVec4(0.85f, 0.37f, 0.00f, 1.00f);
    //ImVec4 light_accent = ImVec4(1.00f, 0.63f, 0.00f, 1.00f);

    // purple
    ImVec4 dark_accent = ImVec4(0.416f, 0.000f, 1.000f, 1.000f);
    ImVec4 light_accent = ImVec4(0.691f, 0.484f, 0.973f, 1.000f);

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
    style.WindowRounding = 7.5;
    style.ChildRounding = style.PopupRounding = style.ScrollbarRounding = style.GrabRounding = style.TabRounding = 4;
    style.ScaleAllSizes(G_SCALE);

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

    //override to avoid transparency issues
    colors[ImGuiCol_ChildBg] = ImVec4(0.20f, 0.21f, 0.27f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.42f, 0.00f, 1.00f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.16f, 0.20f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.40f, 0.45f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.60f, 0.64f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.22f, 0.23f, 0.29f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.f, 0.0f, 0.0f, 0.00f);

#pragma endregion imgui styles

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    L::SetHwndToNotify(hwnd);

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT) {

        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        //if window inactive limit fps to 2
        if (!window_active && forced_frames == 0 && GetMessageWithTimeout(&msg, 500)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            /*if (GetForegroundWindow() == hwnd) {
                window_active = true;
            }*/
        }
        if (forced_frames > 0)
            forced_frames--;

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (status.LastChecked + 1.0 <= ImGui::GetTime()) {//update every second
            DWORD steam_pid = U::FindProcess(_T("steam.exe"));
            status.SteamActive = steam_pid != 0;
            status.SteamServiceActive = U::IsProcessOpen(_T("steamservice.exe"));
            status.CsgoActive = U::IsProcessOpen(_T("csgo.exe"));
            if (steam_pid == 0) {
                CloseHandle(status.SteamHandle);
                status.SteamHandle = NULL;
                status.NezuVacStatusAddress = NULL;
                ZeroMemory(&status.NezuVacStatus, sizeof(status.NezuVacStatus));
                status.NezuVacError = "Steam not running";
            }
            else if(GetProcessId(status.SteamHandle) != steam_pid) {
                CloseHandle(status.SteamHandle);
                status.SteamHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, steam_pid);
                if (!status.SteamHandle)
                    status.NezuVacError = "Unable to open Steam process";
                else status.NezuVacError = "";
                status.NezuVacStatusAddress = NULL;
                ZeroMemory(&status.NezuVacStatus, sizeof(status.NezuVacStatus));
            }
            if (status.NezuVacStatusAddress == NULL && status.SteamHandle != NULL) {
                HMODULE hNezuVac = NULL;
                if (U::FindRemoteDll(status.SteamHandle, _T("NezuVac.dll"), &hNezuVac)) {
                    if (hNezuVac) {
                        status.NezuVacStatusAddress = NezuVac::FindStatusSection(status.SteamHandle, hNezuVac);
                        if(status.NezuVacStatusAddress == NULL)
                            status.NezuVacError = "Unable to find NezuVac status section";
                    }
                    else 
                        status.NezuVacError = "Not loaded";
                }
                else 
                    status.NezuVacError = "Error while searching fror NezuVac.dll";
            }
            if (status.NezuVacStatusAddress != NULL && status.SteamHandle) {
                status.NezuVacStatus = NezuVac::GetStatus(status.SteamHandle, status.NezuVacStatusAddress);
                if (status.NezuVacStatus.magic != 'nezu') 
                    status.NezuVacError = "Invalid magic";
                else
                    status.NezuVacError = "";
            }
            status.LastChecked = ImGui::GetTime();

        }

        ImGui::SetNextWindowSize(ImVec2((float)w, (float)h), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
        ImGuiWindow* window_injection = ImGui::FindWindowByName(WINDOW_TITLE_INJECTION "###" WINDOW_TITLE_INJECTION);
        ImGuiWindow* window = window_injection && window_injection->WasActive ? window_injection : ImGui::FindWindowByName(WINDOW_TITLE);
        static ImVec2 last_window_pos = ImVec2();
        if (window && ImGui::GetCurrentContext()->MovingWindow == NULL)//not moving anny windows
            last_window_pos = window->Pos;
        if (window && (window->Pos.x != last_window_pos.x || window->Pos.y != last_window_pos.y)) {
            ImVec2 delta = window->Pos - last_window_pos;
            RECT rect;
            GetWindowRect(hwnd, &rect);
            SetWindowPos(hwnd, NULL, rect.left + (LONG)delta.x, rect.top + (LONG)delta.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            ImGui::SetWindowPos(window, last_window_pos, ImGuiCond_Always);
        }
        bool open = true;
#ifdef SHOW_DEMO
        ImGui::Begin(WINDOW_TITLE, &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
#else
        ImGui::Begin(WINDOW_TITLE, &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
#endif // SHOW_DEMO
        {
            static const ImVec4& color_disabled = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
            static const ImVec4& color_danger = ImColor(200, 0, 0);
            static const ImVec4& color_danger_light = [](const ImVec4& c){
                float h, s, v;
                ImGui::ColorConvertRGBtoHSV(c.x, c.y, c.z, h, s, v);
                s *= 0.8f;
                v *= 1.1f;//i'm bad at colors, ok...
                return ImColor::HSV(h, s, v);
            }(color_danger);
            static const ImVec4& color_info = ImColor(82, 179, 18);
            static const ImVec4& color_warning = ImColor(255, 135, 25);

            const float button_h = ImGui::GetFontSize() + (ImGui::GetStyle().FramePadding.y * 2.f);
            ImGui::Columns(2, 0, false);
            ImGui::Text("Load options:");
            ImGui::Checkbox("Restart Steam", &settings.RestartSteam);
            ImGui::SameLine(); HelpMarker("Ensures that no vac modules are loaded before injecting VAC bypass.", color_disabled);
            if (settings.RestartSteam && status.NezuVacError.empty() && status.NezuVacStatus.initialized) {
                ImGui::SameLine(); HelpMarker("NezuVac already loaded, Steam restart isn't necessary.", color_warning, "(!)");
            }
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, status.SteamServiceActive && !settings.RestartSteam);
            ImGui::Checkbox("VAC bypass", &settings.VacBypass);
            if (ImGui::GetItemsFlags() & ImGuiItemFlags_Disabled 
                && ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax())) {//ImGui::IsItemHovered dosn't work when item is disabled
                ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed); 
            }
            ImGui::PopItemFlag();
            ImGui::SameLine(); HelpMarker("NezuVac: prevents VAC modules from loading, effectively disabling VAC.", color_disabled);
            if (status.SteamServiceActive && !settings.RestartSteam) {
                ImGui::SameLine(); HelpMarker("Steam Client Service is running!\nVAC bypass can't be injected without restarting Steam.", color_danger, "(!)");
                settings.VacBypass = false;
            }
            else {
                std::string text = "NezuVac:\n";
                if (!status.NezuVacError.empty())
                    text += "Error: " + status.NezuVacError;
                else {
                    text += "Status: " + (status.NezuVacStatus.initialized ? "Loaded" : std::string("Error - ") + NezuVac::GetErrorString(status.NezuVacStatus.error));
                    if (status.NezuVacStatus.initialized)
                        text += "\nNumber of blocked VAC modules: " + std::to_string(status.NezuVacStatus.num_blocked_modules);
                }
                ImGui::SameLine(); HelpMarker(text.c_str(), status.NezuVacStatus.initialized ? color_info : color_danger, "VAC");
            }
            if (settings.injectionMode == InjectionMode::NezuVacOnly && !settings.VacBypass) {
                settings.injectionMode = InjectionMode::Dll;
            }
            ImGui::Text("Injection mode:");
            ImGui::SameLine(); HelpMarker("NezuLoader - Injects NezuLoader into CS:GO (If you select a DLL it will be loaded using NezuLoader afterwards)\n"
                "Standard - Clasic LoadLibrary injector (with trusted mode bypass)\n"
                "Manual map - Use only if you know what you're doing!\n"
                "NezuVac - Only injects VAC bypass into Steam", color_disabled);
            ImGui::SameLine();
            if (status.CsgoActive) {
                if (settings.RestartSteam)
                    HelpMarker("CS:GO is running, it will be closed and reopened while restarting Steam", color_warning, "CSGO");
                else
                    HelpMarker("CS:GO is already running, it will not be restarted", color_info, "CSGO");
            }
            else
                HelpMarker("CS:GO is not running, the injector will start it automatically", ImGui::GetStyleColorVec4(ImGuiCol_Text), "CSGO");
            float last_left_y = ImGui::GetCursorPosY();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
            static const char* modes_str[] = { "NezuLoader", "Standard", "Manual map", "NezuVac" };
            if (ImGui::Combo("##injmde", (int*)&settings.injectionMode, modes_str, IM_ARRAYSIZE(modes_str)) && settings.injectionMode == InjectionMode::NezuVacOnly) {
                settings.VacBypass = true;
            }
            float last_left_h = ImGui::GetCurrentWindow()->DC.LastItemRect.GetHeight();
            ImGui::NextColumn();
            const bool dll_selected = !settings.dll.empty() && settings.injectionMode != InjectionMode::NezuVacOnly;
            const bool can_select = settings.injectionMode != InjectionMode::NezuVacOnly;
            if (ImGui::ButtonEx("Select DLL", ImVec2(dll_selected ? -button_h : -1, last_left_h), can_select ? ImGuiButtonFlags_None : ImGuiButtonFlags_Disabled))
                settings.dll = U::DllFilePicker(hwnd);
            if (!can_select && ImGui::IsItemHovered()) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * (TOOLTIP_SIZE * .52f));
                ImGui::TextUnformatted("This injection mode dosn't support DLL injection into CS:GO.");
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
            if (dll_selected) {
                ImGui::SameLine();
                if (ImGui::CloseButton(ImGui::GetCurrentWindow()->GetID("dll_clear_btn"), ImGui::GetCursorPos() - ImVec2(ImGui::GetStyle().FramePadding.x, 0)))
                    settings.dll.clear();
                const ImVec2 curs = ImGui::GetCursorPos();
                ImGui::ItemSize(ImRect(curs.x, curs.y, curs.x + button_h, curs.y + button_h));
            }

            {//scope this to mke things go out of scope quicker
                float normal_scale = ImGui::GetCurrentWindow()->FontWindowScale;

                CHAR selected_dll_text[MAX_PATH + 50];
                sprintf_s(selected_dll_text, "Selected DLL: %s", settings.dll.size() > 0 && settings.injectionMode != InjectionMode::NezuVacOnly ? settings.dll.c_str() : "<none>");
                const ImVec2 selected_dll_size = ImGui::CalcTextSize(selected_dll_text, NULL, false, ImGui::CalcWrapWidthForPos(ImGui::GetCursorPos(), 0));
                const float space_for_text = last_left_y - ImGui::GetCursorPos().y;
                if (selected_dll_size.y > space_for_text) {
                    //not perfect but good enough for me.
                    const float space_for_text_fraction = space_for_text / (selected_dll_size.y - ((selected_dll_size.y - space_for_text) / 2.5f)); 
                    ImGui::SetWindowFontScale(normal_scale * space_for_text_fraction);
                }
                ImGui::TextWrapped("%s", selected_dll_text);
                ImGui::SetWindowFontScale(normal_scale);
            }
            ImGui::SetCursorPosY(last_left_y);
            bool can_inject = settings.injectionMode == InjectionMode::NezuLoader || settings.injectionMode == InjectionMode::NezuVacOnly || settings.dll.size() > 0;
            if (ImGui::ButtonEx("Load", ImVec2(-1, last_left_h), can_inject ? ImGuiButtonFlags_None : ImGuiButtonFlags_Disabled)) {
                L::Clear();
                ImGui::OpenPopup(WINDOW_TITLE_INJECTION "###" WINDOW_TITLE_INJECTION);
                g_hLoaderThread = CreateThread(0, 0, LoaderThread, &settings, 0, 0);
                if (g_hLoaderThread == NULL)
                    L::Error("Failed to start loader thread");
            }
            if (!can_inject && ImGui::IsItemHovered()) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * (TOOLTIP_SIZE * .5f));
                ImGui::TextUnformatted("Select a DLL first or select a different injection mode");
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
            ImGui::Columns();


            if (g_hLoaderThread != NULL) {
                DWORD LoaderThreadExitCode = 0;
                if (!GetExitCodeThread(g_hLoaderThread, &LoaderThreadExitCode)) {
                    CloseHandle(g_hLoaderThread);
                    g_hLoaderThread = NULL;
                }
                if (LoaderThreadExitCode != STILL_ACTIVE) {
                    CloseHandle(g_hLoaderThread);
                    g_hLoaderThread = NULL;
                }
            }

            const char* LoaderPopupTitle = NULL;
            if (g_hLoaderThread != NULL)
                LoaderPopupTitle = WINDOW_TITLE_INJECTION "###" WINDOW_TITLE_INJECTION;
            else 
                LoaderPopupTitle = WINDOW_TITLE "###" WINDOW_TITLE_INJECTION;

            ImGui::SetNextWindowPos(ImVec2(w / 2.0f, h / 2.0f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2((float)w, (float)h), ImGuiCond_Appearing);

            if (ImGui::BeginPopupModal(LoaderPopupTitle, NULL, ImGuiWindowFlags_NoResize)) {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 1));
                L::lock.lock();
                for (size_t i = 0; i < L::messages.size(); i++) {
                    auto& message = L::messages[i];
                    switch (std::get<0>(message))
                    {
                    case MessageType::Debug: ImGui::TextColored(ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled), "[*]"); break;
                    case MessageType::Info: ImGui::TextColored(color_info, "[+]"); break;
                    case MessageType::Warning: ImGui::TextColored(color_warning, "[!]"); break;
                    case MessageType::Error: ImGui::TextColored(color_danger, "[-]"); break;
                    default: ImGui::Text("[?]"); break;
                    }
                    
                    ImGui::SameLine();
                    ImGui::TextWrapped("%s", std::get<1>(message).c_str());
                }
                L::lock.unlock();
                ImGui::PopStyleVar();

                //AutoScroll
                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f);

                //custom close button
                ImGuiStyle& style = ImGui::GetStyle();
                ImGuiWindow* cur_wnd = ImGui::GetCurrentWindow();

                const bool lt_running = g_hLoaderThread != NULL;
                const char* cb_text = lt_running ? "Cancel" : "Close";

                ImRect orig_rect = cur_wnd->ClipRect;
                cur_wnd->ClipRect = cur_wnd->OuterRectClipped;
                cur_wnd->DrawList->PushClipRectFullScreen();
                ImRect tb_rect = cur_wnd->TitleBarRect();
                const ImVec2 label_size = ImGui::CalcTextSize(cb_text, NULL, true);
                cur_wnd->DC.CursorPos = ImVec2(tb_rect.Max.x - (label_size.x + style.FramePadding.x * 3.0f), tb_rect.Min.y + style.FramePadding.y);
                ImGui::PushStyleColor(ImGuiCol_Button, color_danger);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color_danger_light);
                if (ImGui::SmallButton(cb_text)) {
                    if (lt_running) {
                        //I know, I know, i'll do something beter later
                        if (!TerminateThread(g_hLoaderThread, 2)) {
                            L::Error("Failed to terminate loader thread");
                        }
                        CloseHandle(g_hLoaderThread);
                        g_hLoaderThread = NULL;
                        L::Warning("Loader thread terminated manualy!");
                    }
                    else {
                        ImGui::CloseCurrentPopup();
                    }
                }
                ImGui::PopStyleColor(2);
                cur_wnd->DrawList->PopClipRect();
                cur_wnd->ClipRect = orig_rect;
               ImGui::EndPopup();
            }
        }
        ImGui::End();
        if (!open)
            SendMessage(hwnd, WM_DESTROY, NULL, NULL);
#ifdef SHOW_DEMO
        ImGui::ShowDemoWindow();
#endif // SHOW_DEMO

        ImGui::EndFrame();

        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

        //areas that have LWA_COLORKEY color will be transparent and click-through but have no way to alpha blend
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(20, 20, 20, 255), 1.0f, 0);

#ifdef USE_BLURBEHIND
        //remove color and set 0 lapha from behind all windows to allow for transparency using the alpha chanel
        //these areas won't be click through but will alpha blend correctly with things behind our window (makes anti-aliased rounded corners look nice)
        ImGuiContext& ctx = *ImGui::GetCurrentContext();
        for (int i = 0; i < ctx.Windows.Size; i++) {
            ImGuiWindow* window = ctx.Windows[i];
            if (!window->WasActive) 
                continue;
            //const ImVec2 margin = ImVec2(1, 1);
            const ImVec2 min = window->Pos;// -margin;
            const ImVec2 max = window->Pos + window->Size;// +margin;
            D3DRECT rect = { (LONG)min.x, (LONG)min.y, (LONG)max.x, (LONG)max.y };
            g_pd3dDevice->Clear(1, &rect, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 0), 0, 0);
        }
#endif // USE_BLURBEHIND

        if (g_pd3dDevice->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }

        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        DwmInvalidateIconicBitmaps(hwnd);

        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
            ImGui_ImplDX9_InvalidateDeviceObjects();
            g_pd3dDevice->Reset(&g_d3dpp);
            ImGui_ImplDX9_CreateDeviceObjects();
        }

    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    g_pd3dDevice->Release(); 
    g_pd3dDevice = NULL;
    g_pD3D->Release(); 
    g_pD3D = NULL;

    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}


//the actual window is bigger than what's visible.
//We want only the visible window in alt+tab,win+tab,taskbar preview
HBITMAP CreatePreviewBitmap(HWND hWnd, int nWidth, int nHeight) {
    HBITMAP hbm = NULL;
    HDC hdcMem = CreateCompatibleDC(GetDC(hWnd));
    if (hdcMem != NULL) {
        BITMAPINFO bmi;
        ZeroMemory(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = nWidth;
        bmi.bmiHeader.biHeight = -nHeight;  // Use a top-down DIB
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;

        PBYTE pbDS = NULL;
        hbm = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, (VOID**)&pbDS, NULL, NULL);
        if (hbm) {
            SelectObject(hdcMem, hbm);

            RECT r = { 0, 0, nWidth, nHeight };
            FillRect(hdcMem, &r, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));

            HDC wndHdc = GetDC(hWnd);

            SetStretchBltMode(hdcMem, COLORONCOLOR);
            StretchBlt(hdcMem, 0, 0, nWidth, nHeight, wndHdc, 0, 0, w, h, SRCCOPY);

            ReleaseDC(hWnd, wndHdc);
        }
        DeleteDC(hdcMem);
    }
    return hbm;
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_ACTIVATE:
        window_active = wParam > 0;
        break;
    case WM_USER:
        forced_frames = wParam;
        break;
    case WM_DWMSENDICONICLIVEPREVIEWBITMAP:
    {
        HBITMAP hbm = CreatePreviewBitmap(hWnd, w, h);
        if (hbm) {
            POINT ptOffset = { 0 };
            DwmSetIconicLivePreviewBitmap(hWnd, hbm, &ptOffset, 0);
            DeleteObject(hbm);
        }
    }
    break;
    case WM_DWMSENDICONICTHUMBNAIL:
    {
        HBITMAP hbm = CreatePreviewBitmap(hWnd, HIWORD(lParam), LOWORD(lParam));
        if (hbm) {
            DwmSetIconicThumbnail(hWnd, hbm, 0);
            DeleteObject(hbm);
        }
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

BOOL GetMessageWithTimeout(MSG* msg, UINT to) {
    BOOL res;
    UINT_PTR timerId = SetTimer(NULL, NULL, to, NULL);
    res = GetMessage(msg, NULL, 0, 0);
    KillTimer(NULL, timerId);
    if (!res)
        return FALSE;
    if (msg->message == WM_TIMER && msg->hwnd == NULL && msg->wParam == timerId)
        return FALSE;
    return TRUE;
}


static void HelpMarker(LPCSTR desc, const ImVec4& col, LPCSTR icon) {
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::Text(icon);
    ImGui::PopStyleColor(); //ImGuiCol_Text
    if (ImGui::IsItemHovered()) {
        ImGui::PushStyleColor(ImGuiCol_Border, col);
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * TOOLTIP_SIZE);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
        ImGui::PopStyleColor();//ImGuiCol_Border
    }
}