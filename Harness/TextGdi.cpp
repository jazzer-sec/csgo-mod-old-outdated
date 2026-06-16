#include "Render2D.h"
#ifdef _WIN32
// Windows text backend: renders antialiased glyphs with GDI using the system
// Verdana, so the .exe depends only on Windows system DLLs (no FreeType). Text
// is drawn white onto an offscreen DIB, then the coverage is alpha-blended into
// the Render2D framebuffer — same Text()/TextWidth() surface as the FreeType path.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <cmath>

namespace {

int pxForScale(int scale) { return scale <= 1 ? 11 : (scale == 2 ? 16 : 22); }

std::wstring to_utf16(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring w((size_t)n, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], n);
    return w;
}

struct GdiText {
    HDC dc = nullptr;
    HBITMAP bmp = nullptr;
    uint8_t* bits = nullptr;
    int W = 1200, H = 64;
    HFONT fonts[3] = {nullptr, nullptr, nullptr}; // 11 / 16 / 22 px
    bool ok = false;

    GdiText() {
        dc = CreateCompatibleDC(nullptr);
        if (!dc) return;
        BITMAPINFO bi = {};
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = W;
        bi.bmiHeader.biHeight = -H;       // top-down
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;
        bmp = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, (void**)&bits, nullptr, 0);
        if (!bmp) return;
        SelectObject(dc, bmp);
        const int sizes[3] = {11, 16, 22};
        for (int i = 0; i < 3; ++i)
            fonts[i] = CreateFontA(-sizes[i], 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Verdana");
        SetBkMode(dc, OPAQUE);
        SetBkColor(dc, RGB(0, 0, 0));
        SetTextColor(dc, RGB(255, 255, 255));
        ok = true;
    }
    HFONT fontFor(int scale) { return fonts[scale <= 1 ? 0 : (scale == 2 ? 1 : 2)]; }
};

GdiText& gt() { static GdiText g; return g; }

} // namespace

int Render2D::TextWidth(const std::string& s, int scale) const {
    GdiText& g = gt();
    if (!g.ok) return 0;
    SelectObject(g.dc, g.fontFor(scale));
    std::wstring w = to_utf16(s);
    SIZE sz;
    GetTextExtentPoint32W(g.dc, w.c_str(), (int)w.size(), &sz);
    return sz.cx;
}

int Render2D::TextHeight(int scale) const { return pxForScale(scale); }

void Render2D::Text(float x, float y, const std::string& s, Color c, int scale) {
    GdiText& g = gt();
    if (!g.ok || s.empty()) return;
    SelectObject(g.dc, g.fontFor(scale));
    std::wstring w = to_utf16(s);
    SIZE sz;
    GetTextExtentPoint32W(g.dc, w.c_str(), (int)w.size(), &sz);
    int tw = sz.cx, th = sz.cy;
    if (tw <= 0 || th <= 0) return;
    if (tw > g.W) tw = g.W;
    if (th > g.H) th = g.H;

    RECT rc = {0, 0, tw, th};
    ExtTextOutW(g.dc, 0, 0, ETO_OPAQUE, &rc, L"", 0, nullptr); // clear to black
    TextOutW(g.dc, 0, 0, w.c_str(), (int)w.size());
    GdiFlush();

    int ox = (int)std::lround(x), oy = (int)std::lround(y);
    for (int py = 0; py < th; ++py) {
        const uint8_t* row = g.bits + (size_t)py * g.W * 4;
        for (int pxs = 0; pxs < tw; ++pxs) {
            uint8_t cov = row[(size_t)pxs * 4]; // white text -> any channel = coverage
            if (cov) Blend(ox + pxs, oy + py, c, cov / 255.f);
        }
    }
}
#endif // _WIN32
