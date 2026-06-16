// jazzhook standalone — Dear ImGui (Win32 + DirectX 9) host.
//
// Renders the EXACT same Hud/Menu/Scene as the software build, but through an
// ImGui ImDrawList on a GPU backend (selected via -DR2D_IMGUI). No game, no
// injection — the menu, HUD, and create_move pipeline run against a simulated
// scene. Controls: INSERT toggles the menu, left-click drives it, ESC quits.
//
// Cross-build (MinGW):
//   x86_64-w64-mingw32-g++ -std=c++17 -O2 -mwindows -DR2D_IMGUI \
//     -Ithird_party/imgui -Ithird_party/imgui/backends -Isrc \
//     ImguiApp/main_dx9.cpp Harness/Render2DImgui.cpp Harness/Hud.cpp Harness/Menu.cpp \
//     third_party/imgui/imgui.cpp third_party/imgui/imgui_draw.cpp \
//     third_party/imgui/imgui_tables.cpp third_party/imgui/imgui_widgets.cpp \
//     third_party/imgui/backends/imgui_impl_win32.cpp \
//     third_party/imgui/backends/imgui_impl_dx9.cpp \
//     -ld3d9 -lgdi32 -ldwmapi -limm32 -static -static-libgcc -static-libstdc++ \
//     -o jazzhook_imgui.exe

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx9.h"
#include <d3d9.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../App/Scene.h"   // Render2D(imgui) + Hud + Menu + sim

static const int W = 1280, H = 720;

static LPDIRECT3D9        g_pD3D = nullptr;
static LPDIRECT3DDEVICE9  g_pd3dDevice = nullptr;
static D3DPRESENT_PARAMETERS g_d3dpp = {};

static bool        g_toggleMenu = false;
static bool        g_running = true;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static bool CreateDeviceD3D(HWND hWnd) {
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr) return false;
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // vsync
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
            D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;
    return true;
}

static void CleanupDeviceD3D() {
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    switch (msg) {
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) g_running = false;
        else if (wParam == VK_INSERT) g_toggleMenu = true;
        return 0;
    case WM_DESTROY:
        g_running = false; PostQuitMessage(0); return 0;
    }
    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

static void LoadFonts() {
    ImGuiIO& io = ImGui::GetIO();
    static ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder b;
    b.AddRanges(io.Fonts->GetGlyphRangesDefault());
    b.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
    b.AddChar((ImWchar)0x2192); // → arrow used in the hit log
    b.BuildRanges(&ranges);

    const char* paths[] = {
        "C:\\Windows\\Fonts\\verdana.ttf",
        "C:\\Windows\\Fonts\\tahoma.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
    };
    ImFont* body = nullptr; const char* chosen = nullptr;
    for (const char* p : paths) {
        body = io.Fonts->AddFontFromFileTTF(p, 11.f, nullptr, ranges.Data);
        if (body) { chosen = p; break; }
    }
    ImFont *head = nullptr, *word = nullptr, *title = nullptr;
    if (chosen) {
        head  = io.Fonts->AddFontFromFileTTF(chosen, 16.f, nullptr, ranges.Data);
        word  = io.Fonts->AddFontFromFileTTF(chosen, 22.f, nullptr, ranges.Data);
        title = io.Fonts->AddFontFromFileTTF(chosen, 38.f, nullptr, ranges.Data);
    } else {
        body = head = word = title = io.Fonts->AddFontDefault();
    }
    R2DImguiSetFonts(body, head, word, title);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), CS_CLASSDC, WndProc, 0L, 0L,
        hInst, nullptr, LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr,
        "jazzhook_imgui", nullptr };
    RegisterClassExA(&wc);

    DWORD style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
    RECT rc = { 0, 0, W, H };
    AdjustWindowRect(&rc, style, FALSE);
    HWND hwnd = CreateWindowA("jazzhook_imgui", "jazzhook - standalone (imgui, no game)",
        style, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInst, nullptr);

    if (!CreateDeviceD3D(hwnd)) { CleanupDeviceD3D(); UnregisterClassA(wc.lpszClassName, hInst); return 1; }
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    LoadFonts();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    Render2D r(W, H);
    AppState st;
    app_init(st);
    st.splash.start();   // play the on-inject splash on launch

    while (g_running) {
        MSG msg;
        while (PeekMessageA(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        if (!g_running) break;

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        float dt = io.DeltaTime > 0.f ? io.DeltaTime : 1.f / 60.f;
        if (g_toggleMenu) { st.menuTarget = !st.menuTarget; g_toggleMenu = false; }
        st.open.set(st.menuTarget ? 1.f : 0.f);
        st.open.step(dt);
        app_sim(st, dt);

        ImDrawList* bg = ImGui::GetBackgroundDrawList();
        r.beginFrame(bg);
        Hud::DrawBackdrop(r);
        Hud::DrawWorld(r);
        Hud::DrawOverlays(r);
        app_status(r, st);

        if (st.open.cur > 0.001f) {
            float a = st.open.cur < 1.f ? st.open.cur : 1.f;
            r.BoxFilled(0, 0, (float)W, (float)H, Color(0, 0, 0, (int)(120 * a)));
            Menu::Input in;
            in.mx = io.MousePos.x; in.my = io.MousePos.y;
            in.down = io.MouseDown[0]; in.clicked = io.MouseClicked[0];
            st.menu.input = in;
            st.menu.frameDt = dt;
            st.menu.Draw(r, st.open.cur);
        }

        // on-inject splash over everything; menu button replays it
        if (st.menu.replaySplash) { st.splash.start(); st.menu.replaySplash = false; }
        st.splash.update(dt);
        st.splash.draw(r);

        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
            D3DCOLOR_RGBA(10, 11, 13, 255), 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT res = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
        if (res == D3DERR_DEVICELOST &&
            g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
            ImGui_ImplDX9_InvalidateDeviceObjects();
            g_pd3dDevice->Reset(&g_d3dpp);
            ImGui_ImplDX9_CreateDeviceObjects();
        }
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClassA(wc.lpszClassName, hInst);
    return 0;
}
