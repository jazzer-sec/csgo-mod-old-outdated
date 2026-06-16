// Standalone interactive build for Windows — pure Win32 + GDI, no SDL/FreeType,
// so this compiles to a single self-contained .exe that depends only on Windows
// system DLLs. Your friend can just double-click it. No game, no injection: the
// menu, HUD, and the real create_move pipeline run against a simulated scene.
//
// Controls:  INSERT toggles the menu,  left-click drives it,  ESC quits.
//
// Cross-build with MinGW:
//   x86_64-w64-mingw32-g++ -std=c++17 -O2 -mwindows App/main_win32.cpp \
//     Harness/Render2D.cpp Harness/TextGdi.cpp Harness/Hud.cpp Harness/Menu.cpp \
//     -lgdi32 -luser32 -static -static-libgcc -static-libstdc++ -o jazzhook.exe

#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include "Scene.h"

static const int W = 1280, H = 720;

static bool         g_running = true;
static Menu::Input  g_in;
static bool         g_toggle = false;

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM wp, LPARAM lp) {
    switch (m) {
    case WM_DESTROY:     g_running = false; PostQuitMessage(0); return 0;
    case WM_MOUSEMOVE:   g_in.mx = (float)(short)LOWORD(lp);
                         g_in.my = (float)(short)HIWORD(lp); return 0;
    case WM_LBUTTONDOWN: g_in.down = true;  SetCapture(h); return 0;
    case WM_LBUTTONUP:   g_in.down = false; ReleaseCapture(); return 0;
    case WM_KEYDOWN:
        if (wp == VK_ESCAPE) g_running = false;
        else if (wp == VK_INSERT) g_toggle = true;
        return 0;
    }
    return DefWindowProc(h, m, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "jazzhook_app";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassA(&wc);

    DWORD style = (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX);
    RECT rc = {0, 0, W, H};
    AdjustWindowRect(&rc, style, FALSE);
    HWND hwnd = CreateWindowA("jazzhook_app", "jazzhook - standalone (no game)",
        style, CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInst, nullptr);
    ShowWindow(hwnd, SW_SHOW);

    HDC wdc = GetDC(hwnd);
    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = W;
    bi.bmiHeader.biHeight = -H;        // top-down
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    Render2D r(W, H);
    AppState st;
    app_init(st);
    std::vector<uint8_t> bgra((size_t)W * H * 4);

    LARGE_INTEGER freq, prev;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&prev);
    bool prevDown = false;

    while (g_running) {
        g_toggle = false;
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        g_in.clicked = g_in.down && !prevDown; // left-button press edge
        prevDown = g_in.down;

        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        float dt = (float)(now.QuadPart - prev.QuadPart) / (float)freq.QuadPart;
        prev = now;
        if (dt > 0.1f) dt = 0.1f;

        app_frame(r, st, g_in, g_toggle, dt);

        // Render2D is RGBA; GDI 32-bit DIB is BGRA — swap R/B on the way out.
        const uint8_t* src = r.data();
        for (size_t i = 0; i < (size_t)W * H; ++i) {
            bgra[i * 4 + 0] = src[i * 4 + 2];
            bgra[i * 4 + 1] = src[i * 4 + 1];
            bgra[i * 4 + 2] = src[i * 4 + 0];
            bgra[i * 4 + 3] = 255;
        }
        StretchDIBits(wdc, 0, 0, W, H, 0, 0, W, H, bgra.data(), &bi, DIB_RGB_COLORS, SRCCOPY);
        Sleep(1);
    }

    ReleaseDC(hwnd, wdc);
    return 0;
}
