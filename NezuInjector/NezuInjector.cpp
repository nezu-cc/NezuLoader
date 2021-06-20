#include "pch.h"
#include "RobotoBold.h"
#include "icons.h"
#include "Injector.h"
#include "Loader.h"
#include "NezuVacInterface.h"
#include "resource.h"

//for debuging
//#define SHOW_DEMO

//cretes a fake alt+tab representation using only the used space
//doesn't look as nice as i thought it would, i'll still leave the code just in case
//#define USE_ICONIC_REPRESENTATION

//causes problems on diffrent mashines, i dont want to have a "works on my mashine" type problems.
//You can try it out but dont distribute binaries with it enabled
//#define USE_BLURBEHIND

#define TOOLTIP_SIZE 25.0f
#define G_SCALE 1.4f
#define WINDOW_TITLE "Nezu Injector"
#define WINDOW_TITLE_INJECTION "Loading..."
#define WINDOW_TITLE_ABOUT "About Nezu Injector"
#define WINDOW_TITLE_ACCOUNT "Credentials to use"
#ifdef _DEBUG
#define CFG_FILENAME L"NezuInjector.Debug.ini"
#else
#define CFG_FILENAME L"NezuInjector.ini"
#endif // _DEBUG


#ifdef SHOW_DEMO
int w = 1200, h = 1000;
#else
int w = (int)(315 * G_SCALE), h = (int)(150 * G_SCALE);
#endif // ShOW_DEMO

//globals
static LPDIRECT3D9 g_pD3D = NULL;
static LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS g_d3dpp = {};
static LPDIRECT3DTEXTURE9 g_pSettingsIcon = NULL;
static LPDIRECT3DTEXTURE9 g_pInjectorIcon = NULL;
#ifdef USE_ICONIC_REPRESENTATION
static HBITMAP g_hBitmap;
static HDC g_hdcMain = NULL;
#endif
static HANDLE g_hLoaderThread = NULL;
static bool g_bWindowActive = true;
static int g_iForcedFrames = 0;
static int g_iSelectedSandbox = 0;
static LPCWSTR g_cConfig_file = NULL;
static WCHAR g_cNezuFolder[MAX_PATH] = L"";

//forward definitions
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
HBITMAP CreatePreviewBitmap(HWND hWnd, int nWidth, int nHeight);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL GetMessageWithTimeout(MSG* msg, UINT to);
void HelpMarker(LPCSTR desc, const ImVec4& col, LPCSTR icon = "(?)");
static void MakeLink(LPCSTR text, LPCSTR link, const ImVec4& col = ImVec4(0.f, 0.f, 0.933f, 1.f));
static bool StartInjection();
static void ResetD3D9();

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

IProcess* proc = new StProcess();

#pragma comment( linker, "/subsystem:windows" )
//#pragma comment( linker, "/subsystem:console" )

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
int main(int ac, char** av) { WinMain(GetModuleHandle(NULL), NULL, (LPSTR)L"", 0); }
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    if (Sandboxie::Init()) {
        if (Sandboxie::IsInsideSandbox()) {
            settings.sandbox = Sandboxie::GetCurrentSandboxName();
            proc = new SbProcess(settings.sandbox);
        }
    }

    if (!U::SetDebugPrivilege(TRUE))
        printf("Failed to set debug privileges\n");

#pragma region

    g_cConfig_file = []() -> LPCWSTR {

        PWSTR appdata_dir = NULL;
        if (FAILED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &appdata_dir))) {
            CoTaskMemFree(appdata_dir);
            printf("Failed to load injector config (SHGetKnownFolderPath) 0x%X", GetLastError());
            return NULL;
        }

        LPWSTR filename = new WCHAR[MAX_PATH];
        if (!PathCombineW(filename, appdata_dir, L"Nezu")) {
            CoTaskMemFree(appdata_dir);
            delete[] filename;
            printf("Failed to load injector config (PathCombineW - 1) 0x%X", GetLastError());
            return NULL;
        }

        CoTaskMemFree(appdata_dir);

        if (!CreateDirectoryW(filename, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
            delete[] filename;
            printf("Failed to load injector config (CreateDirectoryW) 0x%X", GetLastError());
            return NULL;
        }

        wcscpy_s(g_cNezuFolder, filename);

        if (!PathCombineW(filename, filename, CFG_FILENAME)) {
            delete[] filename;
            printf("Failed to load injector config (PathCombineW - 2) 0x%X", GetLastError());
            return NULL;
        }

        return filename;

    }();

    if (g_cConfig_file) {
        std::ifstream nii(g_cConfig_file);
        if (nii.is_open()) {
            inipp::Ini<CHAR> ini;
            ini.parse(nii);
            inipp::get_value(ini.sections["NezuInjector"], "RestartSteam", settings.RestartSteam);
            inipp::get_value(ini.sections["NezuInjector"], "VacBypass", settings.VacBypass);
            int injectionMode = (int)settings.injectionMode;
            inipp::get_value(ini.sections["NezuInjector"], "injectionMode", injectionMode);
            settings.injectionMode = (InjectionMode)injectionMode;
            inipp::get_value(ini.sections["NezuInjector"], "dll", settings.dll);
            inipp::get_value(ini.sections["NezuInjector"], "AdvSettingsOpen", settings.AdvSettingsOpen);
            inipp::get_value(ini.sections["NezuInjector"], "CloseAfterLoad", settings.CloseAfterLoad);
            inipp::get_value(ini.sections["NezuInjector"], "UseCustomCredentials", settings.UseCustomCredentials);
            inipp::get_value(ini.sections["NezuInjector"], "CustomSteamArgs", settings.CustomSteamArgs);
            inipp::get_value(ini.sections["NezuInjector"], "AlwaysOnTop", settings.AlwaysOnTop);
            inipp::get_value(ini.sections["NezuInjector"], "CSGOConfig", settings.CSGOConfig);
            nii.close();
        }
        else {
            printf("Failed to load injector config (ifstream::open)");
        }
    }

#pragma endregion config load
    
#pragma region
    if (g_cNezuFolder[0] != L'\0') {
        CHAR err_str[512] = "";
        WCHAR ntdll_path[MAX_PATH];
        PWSTR win_dir = NULL;
        if (FAILED(SHGetKnownFolderPath(FOLDERID_Windows, 0, NULL, &win_dir))) {
            CoTaskMemFree(win_dir);
            sprintf_s(err_str, "SHGetKnownFolderPath failed 0x%X", GetLastError());
            sym_ntdll_native.m_szError = std::string(err_str);
        }
        else {
            if (!PathCombineW(ntdll_path, win_dir, L"System32\\ntdll.dll")) {
                CoTaskMemFree(win_dir);
                sprintf_s(err_str, "PathCombineW failed 0x%X", GetLastError());
                sym_ntdll_native.m_szError = std::string(err_str);
            }
            else {
                CoTaskMemFree(win_dir);
                sym_ntdll_native_ret = std::async(std::launch::async, &SYMBOL_PARSER::Initialize, &sym_ntdll_native, ntdll_path, g_cNezuFolder, nullptr, false, false);
            }
        }
    }
#pragma endregion sym_ntdll init

#pragma region

    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(0);
    wc.lpszClassName = _T("NezuInjector");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICON1));
    wc.hIconSm = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICON1));
	RegisterClassEx(&wc);

	RECT rect;
	GetClientRect(GetDesktopWindow(), &rect);
	rect.left = (rect.right / 2) - (w / 2);
	rect.top = (rect.bottom / 2) - (h / 2);

    //create window
	HWND hwnd = CreateWindow(wc.lpszClassName, _T(WINDOW_TITLE), WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX, rect.left, rect.top, w * 2, h * 2, NULL, NULL, wc.hInstance, NULL);
//#ifndef SHOW_DEMO
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    if(settings.AlwaysOnTop)
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
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
#ifdef USE_ICONIC_REPRESENTATION
    BOOL fForceIconic = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_FORCE_ICONIC_REPRESENTATION, &fForceIconic, sizeof(fForceIconic));
    BOOL fHasIconicBitmap = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_HAS_ICONIC_BITMAP, &fHasIconicBitmap, sizeof(fHasIconicBitmap));
    g_hdcMain = CreateCompatibleDC(GetDC(hwnd));
#endif //USE_ICONIC_REPRESENTATION
#ifdef USE_BLURBEHIND
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);
#endif // USE_BLURBEHIND

#ifdef USE_ICONIC_REPRESENTATION
    BITMAPINFO bmi;
    ZeroMemory(&bmi.bmiHeader, sizeof(BITMAPINFOHEADER));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h * 2;  // Use a top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;

    HDC wndHdc = GetDC(hwnd);

    PBYTE pbDS = NULL;
    g_hBitmap = CreateDIBSection(g_hdcMain, &bmi, DIB_RGB_COLORS, (VOID**)&pbDS, NULL, NULL);
    if (!g_hBitmap) {
        return 1;
    }
    SelectObject(g_hdcMain, g_hBitmap);
#endif //USE_ICONIC_REPRESENTATION

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
    ImGuiIO& io = ImGui::GetIO();
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

    static const ImVec4 dark_accent = ImVec4(0.416f, 0.000f, 1.000f, 1.000f);
    static const ImVec4 light_accent = ImVec4(0.691f, 0.484f, 0.973f, 1.000f);
    static const ImVec4& color_disabled = ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled);
    static const ImVec4& color_danger = ImColor(200, 0, 0);
    static const ImVec4& color_danger_light = [](const ImVec4& c) {
        float h, s, v;
        ImGui::ColorConvertRGBtoHSV(c.x, c.y, c.z, h, s, v);
        s *= 0.8f;
        v *= 1.1f;//i'm bad at colors, ok...
        return ImColor::HSV(h, s, v);
    }(color_danger);
    static const ImVec4& color_info = ImColor(82, 179, 18);
    static const ImVec4& color_warning = ImColor(255, 135, 25);

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

#pragma region
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        //if window inactive limit fps to 2
        if (!g_bWindowActive && g_iForcedFrames == 0 && GetMessageWithTimeout(&msg, 500)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            /*if (GetForegroundWindow() == hwnd) {
                g_bWindowActive = true;
            }*/
        }
        if (g_iForcedFrames > 0)
            g_iForcedFrames--;
#pragma endregion window messages handler

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
#ifndef USE_ICONIC_REPRESENTATION
        ImGui::GetIO().DisplaySize = ImVec2((float)(w * 2), (float)(h * 3));
#endif //USE_ICONIC_REPRESENTATION
        ImGui::NewFrame();

#pragma region

        if (status.LastChecked + 1.0 <= ImGui::GetTime()) {//update every second
            DWORD steam_pid = proc->FindProcess(_T("steam.exe"));
            status.SteamActive = steam_pid != 0;
            status.SteamServiceActive = proc->IsProcessOpen(_T("steamservice.exe"));
            status.CsgoActive = proc->IsProcessOpen(_T("csgo.exe"));
            if (steam_pid == 0) {
                CloseHandle(status.SteamHandle);
                status.SteamHandle = NULL;
                status.NezuVacStatusAddress = NULL;
                ZeroMemory(&status.NezuVacStatus, sizeof(status.NezuVacStatus));
                status.NezuVacError = "Steam not running";
            }
            else if (GetProcessId(status.SteamHandle) != steam_pid) {
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
                        if (status.NezuVacStatusAddress == NULL)
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
            if (Sandboxie::IsLoaded() && !Sandboxie::IsInsideSandbox()) {
                size_t old_box_count = Sandboxie::GetBoxes(false).size();
                auto new_boxes = Sandboxie::GetBoxes(true);
                size_t new_box_count = new_boxes.size();
                if (new_box_count == 0 && g_iSelectedSandbox != 0) {
                    g_iSelectedSandbox = 0;
                    settings.sandbox = "";
                }
                else if (new_box_count != old_box_count && g_iSelectedSandbox > 0) {
                    int box = g_iSelectedSandbox - 1;
                    if (box >= (int)new_box_count || new_boxes[box] != settings.sandbox) {
                        g_iSelectedSandbox = 0;//reset index
                        for (size_t i = 0; i < new_box_count; i++) {
                            if (new_boxes[i] == settings.sandbox) {
                                g_iSelectedSandbox = i + 1;
                                break;
                            }
                        }
                        if (g_iSelectedSandbox == 0) {//not found, sandbox has been deleted
                            settings.sandbox = "";
                        }
                    }
                }

            }
            else {
                g_iSelectedSandbox = 0;
                settings.sandbox = "";
            }
            status.LastChecked = ImGui::GetTime();

        }
#pragma endregion status update

#pragma region

        ImGui::SetNextWindowSize(ImVec2((float)w, (float)h), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
        ImGuiWindow* window_injection = ImGui::FindWindowByName(WINDOW_TITLE_INJECTION "###" WINDOW_TITLE_INJECTION);
        ImGuiWindow* window_about = ImGui::FindWindowByName(WINDOW_TITLE_ABOUT "###" WINDOW_TITLE_ABOUT);
        ImGuiWindow* window_account = ImGui::FindWindowByName(WINDOW_TITLE_ACCOUNT "###" WINDOW_TITLE_ACCOUNT);
        ImGuiWindow* window_main = ImGui::FindWindowByName(WINDOW_TITLE "###" WINDOW_TITLE);
        ImGuiWindow* window = window_account && window_account->WasActive ? window_account : window_about && window_about->WasActive ? window_about :
            window_injection && window_injection->WasActive ? window_injection : window_main;
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

#pragma endregion window position update

        char main_window_title[128] = WINDOW_TITLE;
        if (Sandboxie::IsLoaded()) {
            if (Sandboxie::IsInsideSandbox()) {
                const char* name = Sandboxie::GetCurrentSandboxName();
                sprintf_s(main_window_title, WINDOW_TITLE " (SB: %s)", name ? name : "Unknown Sandbox");
            }
            else if(settings.sandbox.size() > 0){
                sprintf_s(main_window_title, WINDOW_TITLE " (SB: %s)", settings.sandbox.c_str());
            }
        }

        bool open = true;
#ifdef SHOW_DEMO
        ImGui::Begin("###" WINDOW_TITLE, &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
#else
        ImGui::Begin("###" WINDOW_TITLE, &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
#endif // SHOW_DEMO
        {
            const float button_h = ImGui::GetFontSize() + (ImGui::GetStyle().FramePadding.y * 2.f);
            ImGui::Columns(2, 0, false);
            ImGui::Text("Load options:");
            ImGui::Checkbox("Restart Steam", &settings.RestartSteam);
            ImGui::SameLine(); HelpMarker("Ensures that no VAC modules are loaded before injecting VAC bypass.", color_disabled);
            if (settings.RestartSteam && status.NezuVacError.empty() && status.NezuVacStatus.initialized) {
                ImGui::SameLine(); HelpMarker("NezuVac already loaded, Steam restart isn't necessary.", color_warning, "(!)");
            }
            else if (!status.SteamActive) {
                ImGui::SameLine(); HelpMarker("Steam isn't running. The injector will start it automatically.", colors[ImGuiCol_Text], "(i)");
            }
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, (status.SteamServiceActive && !status.NezuVacStatus.initialized) && !settings.RestartSteam);
            ImGui::Checkbox("VAC bypass", &settings.VacBypass);
            if (ImGui::GetItemsFlags() & ImGuiItemFlags_Disabled
                && ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax())) {//ImGui::IsItemHovered dosn't work when item is disabled
                ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
            }
            ImGui::PopItemFlag();
            ImGui::SameLine(); HelpMarker("NezuVac: prevents VAC modules from loading, effectively disabling VAC.", color_disabled);
            if (Sandboxie::IsLoaded() && !settings.sandbox.empty() && !settings.VacBypass && !status.NezuVacStatus.initialized && !status.SteamServiceActive) {
                ImGui::SameLine(); HelpMarker("Running csgo in a sandbox without VAC bypass will cause VAC Errors!", color_warning, "(!)");
            }
            else if ((status.SteamServiceActive && !status.NezuVacStatus.initialized) && !settings.RestartSteam) {
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
                HelpMarker("CS:GO is not running, the injector will start it automatically.", ImGui::GetStyleColorVec4(ImGuiCol_Text), "CSGO");
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
                if (settings.UseCustomCredentials && (!status.SteamActive || settings.RestartSteam)) {
                    settings.Creds = std::make_pair("", "");//reset inputs
                    ImGui::OpenPopup(WINDOW_TITLE_ACCOUNT "###" WINDOW_TITLE_ACCOUNT);
                }
                else {
                    ImGui::OpenPopup(WINDOW_TITLE_INJECTION "###" WINDOW_TITLE_INJECTION);
                    StartInjection();
                }
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
                    if (settings.CloseAfterLoad && LoaderThreadExitCode == TRUE) {
                        PostQuitMessage(0);
                    }
                    CloseHandle(g_hLoaderThread);
                    g_hLoaderThread = NULL;
                }
            }

            bool open_injection_popup = false;
            ImGui::SetNextWindowPos(ImVec2(w / 2.0f, h / 2.0f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2((float)w, (float)h), ImGuiCond_Appearing);
            if (ImGui::BeginPopupModal(WINDOW_TITLE_ACCOUNT "###" WINDOW_TITLE_ACCOUNT, &open, ImGuiWindowFlags_NoResize)) {

                if (ImGui::BeginTable("login", 2)) {
                    ImGui::TableSetupColumn(NULL, ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn(NULL, ImGuiTableColumnFlags_WidthStretch);

                    static std::string xd;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); {
                        ImGui::Text("Login:");
                    }
                    ImGui::TableSetColumnIndex(1); {
                        ImGui::PushItemWidth(-FLT_MIN);
                        ImGui::InputText("##username", &settings.Creds.first);
                        if (ImGui::IsWindowAppearing())
                            ImGui::SetKeyboardFocusHere(-1);
                    }

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); {
                        ImGui::Text("Password:");
                    }
                    ImGui::TableSetColumnIndex(1); {
                        if (ImGui::InputText("##password", &settings.Creds.second, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue)) {
                            if (!settings.Creds.first.empty() && !settings.Creds.second.empty()) {
                                ImGui::CloseCurrentPopup();
                                open_injection_popup = true;
                                StartInjection();
                            }
                            else {
                                ImGui::SetKeyboardFocusHere(-1);
                            }
                        }
                    }

                    ImGui::EndTable();
                }
                ImGui::Text("");
                ImGui::SameLine(ImGui::GetContentRegionMax().x - (200 + 100 + style.ItemSpacing.x));
                ImGui::PushStyleColor(ImGuiCol_Button, color_danger);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color_danger_light);
                if (ImGui::Button("Cancel", ImVec2(100, 0))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::PopStyleColor(2);
                ImGui::SameLine();
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, settings.Creds.first.empty() || settings.Creds.second.empty());
                if (ImGui::Button("Load", ImVec2(200, 0))) {
                    ImGui::CloseCurrentPopup();
                    open_injection_popup = true;
                    StartInjection();
                }
                if (ImGui::GetItemsFlags() & ImGuiItemFlags_Disabled && ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax())) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
                }
                ImGui::PopItemFlag();

                ImGui::EndPopup();
            }
            if (open_injection_popup) //has to be outside BeginPopupModal
                ImGui::OpenPopup(WINDOW_TITLE_INJECTION "###" WINDOW_TITLE_INJECTION);

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
                    case MessageType::Debug: ImGui::TextColored(color_disabled, "[*]"); break;
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
                {
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
                            //I know, I know, I'll do something beter later(if I don't forget)
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
                }
                ImGui::EndPopup();
            }

            ImGui::SetNextWindowPos(ImVec2(w / 2.0f, h / 2.0f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2((float)w, (float)h), ImGuiCond_Appearing);
            bool open = true;
            if (ImGui::BeginPopupModal(WINDOW_TITLE_ABOUT "###" WINDOW_TITLE_ABOUT, &open, ImGuiWindowFlags_NoResize)) {
                ImGui::Text("Made by nezu (aka dumbasPL)");
                ImGui::SameLine(); MakeLink("GitHub", "https://github.com/dumbasPL/NezuLoader", color_info);

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 1));
                ImGui::Text("Credits:");
                ImGui::Text("ocornut - for Dear ImGui");
                ImGui::Text("Tsuda Kageyu - for MinHook");
                ImGui::Text("zyhp - for VAC3 Inhibitor");
                ImGui::Text("mcmtroffaes - for IniPP");
                ImGui::PopStyleVar();

                ImGui::EndPopup();
            }

#pragma region
            ImGuiWindow* cur_wnd = ImGui::GetCurrentWindow();
            float font_size = ImGui::GetFontSize();
            ImRect orig_rect = cur_wnd->ClipRect;
            cur_wnd->ClipRect = cur_wnd->OuterRectClipped;
            cur_wnd->DrawList->PushClipRectFullScreen();
            ImRect tb_rect = cur_wnd->TitleBarRect();

            //icon
            {
                const ImVec2 pos(tb_rect.Min.x + 3, tb_rect.Min.y + 3);
                const ImRect bb(pos, pos + ImVec2(23, 23));
                cur_wnd->DrawList->AddImage(g_pInjectorIcon, bb.Min, bb.Max);
            }

            //title
            {
                const ImVec2 pos(tb_rect.Min.x + 3 + 23 + style.ItemSpacing.x, tb_rect.Min.y + style.FramePadding.y);
                const ImRect bb(pos, ImVec2(tb_rect.Max.x - style.FramePadding.x - (font_size * 4.0f + style.FramePadding.x * 2.0f), pos.y + font_size));
                ImGui::RenderTextClipped(bb.Min, bb.Max, main_window_title, NULL, NULL, ImVec2(0.0f, 0.5f), &bb);
            }

            //minimize button
            {
                const ImVec2 pos(tb_rect.Max.x - (font_size * 2.0f + style.FramePadding.x * 1.5f), tb_rect.Min.y + style.FramePadding.y);
                const ImRect bb(pos, pos + ImVec2(font_size, font_size));
                ImGuiID id = cur_wnd->GetID("minimize_btn");
                if (ImGui::ItemAdd(bb, id)) {
                    bool hovered, held;
                    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
                    ImU32 col = ImGui::GetColorU32(held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered);
                    if (hovered)
                        cur_wnd->DrawList->AddCircleFilled(bb.GetCenter(), ImMax(2.0f, font_size * 0.5f + 1.0f), col, 12);
                    float cross_extent = font_size * 0.5f * 0.7071f - 1.0f;
                    ImU32 cross_col = ImGui::GetColorU32(ImGuiCol_Text);
                    ImVec2 center = bb.GetCenter() - ImVec2(0.5f, 0.5f);
                    cur_wnd->DrawList->AddLine(center + ImVec2(-cross_extent, 0), center + ImVec2(+cross_extent, 0), cross_col, 1.0f);
                    if (pressed) {
                        ShowWindow(hwnd, SW_MINIMIZE);
                    }
                }
            }

            //about button
            {
                const char* ab_text = "?";
                const ImVec2 pos(tb_rect.Max.x - (font_size * 3.0f + style.FramePadding.x * 2.0f), tb_rect.Min.y + style.FramePadding.y);
                const ImRect bb(pos, pos + ImVec2(font_size, font_size));
                ImGuiID id = cur_wnd->GetID("about_btn");
                if (ImGui::ItemAdd(bb, id)) {
                    bool hovered, held;
                    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
                    ImU32 col = ImGui::GetColorU32(held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered);
                    if (hovered)
                        cur_wnd->DrawList->AddCircleFilled(bb.GetCenter(), ImMax(2.0f, font_size * 0.5f + 1.0f), col, 12);
                    ImGui::RenderTextClipped(bb.Min, bb.Max, ab_text, NULL, NULL, ImVec2(0.5f, 0.5f), &bb);
                    if (pressed) {
                        ImGui::OpenPopup(WINDOW_TITLE_ABOUT "###" WINDOW_TITLE_ABOUT);
                    }
                }
            }

            //settings button
            if (g_pSettingsIcon) {
                const ImVec2 pos(tb_rect.Max.x - (font_size * 4.0f + style.FramePadding.x * 2.5f), tb_rect.Min.y + style.FramePadding.y);
                const ImRect bb(pos, pos + ImVec2(font_size, font_size));
                ImGuiID id = cur_wnd->GetID("settings_btn");
                if (ImGui::ItemAdd(bb, id)) {
                    bool hovered, held;
                    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
                    ImU32 col = ImGui::GetColorU32(held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered);
                    if (hovered)
                        cur_wnd->DrawList->AddCircleFilled(bb.GetCenter(), ImMax(2.0f, font_size * 0.5f + 1.0f), col, 12);
                    const ImVec2 img_offset(style.FramePadding.y, style.FramePadding.y);
                    cur_wnd->DrawList->AddImage(g_pSettingsIcon, bb.Min + img_offset, bb.Max - img_offset);
                    if (pressed) {
                        settings.AdvSettingsOpen ^= true;
                    }
                }
            }

            cur_wnd->DrawList->PopClipRect();
            cur_wnd->ClipRect = orig_rect;
#pragma endregion title bar

        }
        ImGui::End();
        if (!open)
            SendMessage(hwnd, WM_DESTROY, NULL, NULL);

        static float h_extra = ImGui::GetFrameHeightWithSpacing();

        if (settings.AdvSettingsOpen && (!window_injection || !window_injection->WasActive) && (!window_about || !window_about->WasActive) && (!window_account || !window_account->WasActive)) {


            ImGui::SetNextWindowSize(ImVec2((float)w, (float)h + h_extra), ImGuiCond_Appearing);
            ImGui::SetNextWindowPos(ImVec2(0, (float)h), ImGuiCond_Once);

            if (ImGui::Begin("Advanced Settings", &settings.AdvSettingsOpen,
#ifdef SHOW_DEMO
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar)) {
#else
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove)) {
#endif // SHOW_DEMO
                //static int
                const auto& boxes = Sandboxie::GetBoxes(false);
                static float sandbox_text_size = ImGui::CalcTextSize("Sandboxie").x;
                ImGui::PushItemWidth(-(sandbox_text_size + style.ItemInnerSpacing.x * 2));
                bool changed = ImGui::Combo("###Sandbox", &g_iSelectedSandbox, [](void* data, int idx, const char** out_text) {
                    if (idx == 0) {
                        *out_text = Sandboxie::IsInsideSandbox() ? Sandboxie::GetCurrentSandboxName() : "No sandboxing (ignore sandboxes)";
                        return true;
                    }
                    auto data_boxes = (const std::vector<std::string>*)data;
                    *out_text = (*data_boxes)[idx - 1].c_str();
                    return true;
                }, (void*)&boxes, (int)boxes.size() + 1, 10);
                if (changed && !Sandboxie::IsInsideSandbox()) {
                    settings.sandbox = g_iSelectedSandbox > 0 ? boxes[g_iSelectedSandbox - 1] : "";
                    if (g_iSelectedSandbox > 0)
                        proc = new SbProcess(settings.sandbox);
                    else
                        proc = new StProcess();
                    status.LastChecked = -1;// force status update
                }
                ImGui::PopItemWidth();
                ImGui::SameLine(); 
                if (Sandboxie::IsLoaded())
                    HelpMarker((std::string("Loaded\nVersion: ") + Sandboxie::GetVersion() +
                        (Sandboxie::IsInsideSandbox() ? "\nWarning: running inside of a sandbox,\ninteractions outside of this sandbox imposible!" : "")
                        ).c_str(), Sandboxie::IsInsideSandbox() ? color_warning : color_info, "Sandboxie");
                else
                    HelpMarker("Sandboxie not installed", color_danger, "Sandboxie");

                ImGui::Checkbox("Use different account when (re)starting steam", &settings.UseCustomCredentials);
                ImGui::Checkbox("Close after successfull load", &settings.CloseAfterLoad); 
                ImGui::SameLine();
                if (ImGui::Checkbox("Always on top", &settings.AlwaysOnTop))
                    SetWindowPos(hwnd, settings.AlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4));
                ImGui::Text("Additional steam launch parameters:");
                ImGui::PushItemWidth(-1);
                ImGui::InputText("##steamargs", &settings.CustomSteamArgs);
                ImGui::PopItemWidth();
                ImGui::PopStyleVar();

                ImGui::AlignTextToFramePadding();
                ImGui::Text("CS:GO Config:");
                ImGui::SameLine();
                HelpMarker("CS:GO will use this directory to load/save configuration\nWorks only when (re)starting steam", 
                    status.SteamActive && !settings.RestartSteam ? color_warning : color_disabled);
                ImGui::SameLine();
                static float btn_width = 100; //some start value
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4));
                ImGui::PushItemWidth(-btn_width);
                ImGui::InputText("##csgo_cfg", &settings.CSGOConfig, ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_NoMarkEdited);
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (settings.CSGOConfig.empty()) {
                    if (ImGui::Button("Select"))
                        settings.CSGOConfig = U::DirectoryPicker(hwnd);
                }
                else {
                    if (ImGui::Button("Clear"))
                        settings.CSGOConfig.clear();
                }
                btn_width = ImGui::GetItemRectSize().x + 1;
                ImGui::PopStyleVar();

            }
            ImGui::End();
        }

        bool download_in_progress = sym_ntdll_native_ret.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready;
        if (download_in_progress || (!sym_ntdll_native_ret.get() && !sym_ntdll_native.m_szError.empty())) {
            ImGui::SetNextWindowSize(ImVec2((float)w, (float)h), ImGuiCond_Appearing);
            ImGui::SetNextWindowPos(ImVec2(0, (float)(settings.AdvSettingsOpen ? (2 * h) + h_extra : h)), ImGuiCond_Always);

            if (ImGui::Begin("Symbol download", NULL,
#ifdef SHOW_DEMO
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar)) {
#else
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove)) {
#endif // SHOW_DEMO

                if (download_in_progress) {
                    ImGui::Text("Downloading ntdll.dll symbols from Microsoft...");
                    ImGui::ProgressBar(sym_ntdll_native.GetDownloadProgress());
                    ImGui::PushStyleColor(ImGuiCol_Button, color_danger);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color_danger_light);
                    if (ImGui::Button("Cancel", ImVec2(-1, 0)))
                        sym_ntdll_native.Interrupt();
                    ImGui::PopStyleColor(2);
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Text, color_danger);
                    ImGui::TextWrapped("Download error: %s", sym_ntdll_native.m_szError.c_str());
                    ImGui::PopStyleColor();
                    ImGui::PushStyleColor(ImGuiCol_Button, color_danger);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color_danger_light);
                    if (ImGui::Button("Close", ImVec2(-1, 0)))
                        sym_ntdll_native.m_szError = "";
                    ImGui::PopStyleColor(2);
                }

            }
            ImGui::End();
        }

#ifdef SHOW_DEMO
        ImGui::ShowDemoWindow();
#endif // SHOW_DEMO

#pragma region
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

#ifdef USE_ICONIC_REPRESENTATION
        if (!IsIconic(hwnd))
            BitBlt(g_hdcMain, 0, 0, w, h * (g_bSettingsOpen ? 2 : 1), wndHdc, 0, 0, SRCCOPY);
        DwmInvalidateIconicBitmaps(hwnd);
#else
#ifndef SHOW_DEMO
        ImVec2 max_draw(0, 0);
        static ImVec2 max_draw_last(0, 0);
        for (ImGuiWindow* window : ImGui::GetCurrentContext()->Windows) {
            if (!window->WasActive)
                continue;
            if (window->Flags & ImGuiWindowFlags_Tooltip) {
                max_draw = ImGui::GetIO().DisplaySize;
                break;
            }
            ImVec2 max = window->Rect().Max;
            if (max.x > max_draw.x)
                max_draw.x = max.x;
            if (max.y > max_draw.y)
                max_draw.y = max.y;
        }
        if (max_draw.x != max_draw_last.x || max_draw.y != max_draw_last.y) {
            if (SetWindowPos(hwnd, NULL, 0, 0, (int)max_draw.x, (int)max_draw.y, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE))
                max_draw_last = max_draw;
        }
#endif // !SHOW_DEMO
#endif //USE_ICONIC_REPRESENTATION

        HRESULT cooperativeLevel = g_pd3dDevice->TestCooperativeLevel();
        if (result != D3D_OK && (cooperativeLevel == D3DERR_DEVICENOTRESET || cooperativeLevel == D3DERR_DEVICELOST) ) {
            ResetD3D9();
        } 
        else if (result != D3D_OK) {
            char msg[100] = "";
            sprintf_s(msg, "Present: %d - %d", HRESULT_CODE(result), HRESULT_CODE(g_pd3dDevice->TestCooperativeLevel()));
            MessageBoxA(hwnd, msg, "Error", MB_ICONERROR);
        }
#pragma endregion render

    }

#pragma region

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    g_pd3dDevice->Release(); 
    g_pd3dDevice = NULL;
    g_pD3D->Release(); 
    g_pD3D = NULL;

#ifdef USE_ICONIC_REPRESENTATION
    ReleaseDC(hwnd, wndHdc);
    DeleteDC(g_hdcMain);
    DeleteObject(g_hBitmap);
#endif

    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

#pragma endregion cleanup

    return 0;
}

#pragma region

#ifdef USE_ICONIC_REPRESENTATION
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

            SetStretchBltMode(hdcMem, COLORONCOLOR);
            StretchBlt(hdcMem, 0, 0, nWidth, nHeight, g_hdcMain, 0, 0, w, h * (g_bSettingsOpen ? 2 : 1), SRCCOPY);
        }
        DeleteDC(hdcMem);
    }
    return hbm;
}
#endif

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_ACTIVATE:
        g_bWindowActive = wParam > 0;
        break;
    case WM_USER:
        g_iForcedFrames = wParam;
        break;
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetD3D9();
        }
        return 0;
#ifdef USE_ICONIC_REPRESENTATION
    case WM_DWMSENDICONICLIVEPREVIEWBITMAP:
    {
        HBITMAP hBitmap = CreatePreviewBitmap(hWnd, w, h * (g_bSettingsOpen ? 2 : 1));
        if (hBitmap) {
            POINT ptOffset = { 0 };
            DwmSetIconicLivePreviewBitmap(hWnd, hBitmap, NULL, NULL);
            DeleteObject(hBitmap);
        }
    }
    break;
    case WM_DWMSENDICONICTHUMBNAIL:
    {
        HBITMAP hBitmap = CreatePreviewBitmap(hWnd, HIWORD(lParam), LOWORD(lParam));
        if (hBitmap) {
            DwmSetIconicThumbnail(hWnd, hBitmap, 0);
            DeleteObject(hBitmap);
        }
    }
    break;
#endif //USE_ICONIC_REPRESENTATION
    case WM_DESTROY:
        if (g_cConfig_file) {
            std::ofstream nii(g_cConfig_file, std::ofstream::out | std::ofstream::trunc);
            if (nii.is_open()) {
                inipp::Ini<CHAR> ini;
                inipp::set_value(ini.sections["NezuInjector"], "RestartSteam", settings.RestartSteam);
                inipp::set_value(ini.sections["NezuInjector"], "VacBypass", settings.VacBypass);
                inipp::set_value(ini.sections["NezuInjector"], "injectionMode", (int)settings.injectionMode);
                inipp::set_value(ini.sections["NezuInjector"], "dll", settings.dll);
                inipp::set_value(ini.sections["NezuInjector"], "AdvSettingsOpen", settings.AdvSettingsOpen);
                inipp::set_value(ini.sections["NezuInjector"], "CloseAfterLoad", settings.CloseAfterLoad);
                inipp::set_value(ini.sections["NezuInjector"], "UseCustomCredentials", settings.UseCustomCredentials);
                inipp::set_value(ini.sections["NezuInjector"], "CustomSteamArgs", settings.CustomSteamArgs);
                inipp::set_value(ini.sections["NezuInjector"], "AlwaysOnTop", settings.AlwaysOnTop);
                inipp::set_value(ini.sections["NezuInjector"], "CSGOConfig", settings.CSGOConfig);
                ini.generate(nii);
                nii.close();
            }
            else printf("Failed to save injector config (ofstream::open)");
        }
        else printf("Failed to save injector config (config_file is NULL)");
        sym_ntdll_native.Interrupt();
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

#pragma endregion Window stuff

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

static void MakeLink(LPCSTR text, LPCSTR link, const ImVec4& col) {
    ImGui::TextColored(col, "%s", text);
    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    if(ImGui::IsItemClicked())
        ShellExecuteA(NULL, "open", link, NULL, NULL, SW_SHOWNORMAL);
}

static bool StartInjection() {
    if (!Injector::RtlInsertInvertedFunctionTable) {
        DWORD RVA = 0;
        if (sym_ntdll_native.GetSymbolAddress("RtlInsertInvertedFunctionTable", RVA)) {
            HINSTANCE hNTDLL = GetModuleHandle(TEXT("ntdll.dll"));
            if (hNTDLL)
                Injector::RtlInsertInvertedFunctionTable = (f_RtlInsertInvertedFunctionTable)((DWORD)hNTDLL + RVA);
        }
    }
    if (!Injector::RtlInsertInvertedFunctionTable && (settings.injectionMode == InjectionMode::NezuLoader || settings.injectionMode == InjectionMode::DllManual))
        L::Warning("ntdll.dll symbols not loaded, exceptions will not work inside mapped DLL");
    g_hLoaderThread = CreateThread(0, 0, LoaderThread, &settings, 0, 0);
    if (g_hLoaderThread == NULL) {
        L::Error("Failed to start loader thread");
        return false;
    }
    return true;
}

static void ResetD3D9() {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    if (g_pSettingsIcon) {
        g_pSettingsIcon->Release();
        g_pSettingsIcon = NULL;
    }
    if (g_pInjectorIcon) {
        g_pInjectorIcon->Release();
        g_pInjectorIcon = NULL;
    }
    g_pd3dDevice->Reset(&g_d3dpp);
    if (!g_pSettingsIcon && !CreateSettingsIcon(g_pd3dDevice, &g_pSettingsIcon))
        g_pSettingsIcon = NULL;
    if (!g_pInjectorIcon && !CreateInjectorIcon(g_pd3dDevice, &g_pInjectorIcon))
        g_pInjectorIcon = NULL;
    ImGui_ImplDX9_CreateDeviceObjects();
}