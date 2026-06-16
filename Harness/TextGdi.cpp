#include "Render2D.h"
#ifdef _WIN32
// Windows text backend: antialiased system Verdana via GDI, so the .exe needs no
// FreeType DLL. Each (size, codepoint) glyph is rasterized once into a cache and
// then blitted from CPU memory every frame — no per-frame GDI calls / GdiFlush,
// which is what kept the framerate down.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
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

struct CGlyph { int w = 0, h = 0, adv = 0; std::vector<uint8_t> cov; };

struct GdiText {
    HDC dc = nullptr;
    HBITMAP bmp = nullptr;
    uint8_t* bits = nullptr;
    int W = 256, H = 64;
    HFONT fonts[3] = {nullptr, nullptr, nullptr};   // 11 / 16 / 22 px
    int cellH[3] = {11, 16, 22};
    int curIdx = -1;
    std::unordered_map<uint64_t, CGlyph> glyphs;
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
        for (int i = 0; i < 3; ++i) {
            fonts[i] = CreateFontA(-sizes[i], 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Verdana");
            SelectObject(dc, fonts[i]);
            TEXTMETRICA tm;
            if (GetTextMetricsA(dc, &tm)) cellH[i] = tm.tmHeight;
        }
        SetBkMode(dc, OPAQUE);
        SetBkColor(dc, RGB(0, 0, 0));
        SetTextColor(dc, RGB(255, 255, 255));
        ok = true;
    }

    int idxForScale(int scale) { return scale <= 1 ? 0 : (scale == 2 ? 1 : 2); }

    const CGlyph* glyph(int idx, uint32_t cp) {
        uint64_t key = ((uint64_t)idx << 32) | cp;
        auto it = glyphs.find(key);
        if (it != glyphs.end()) return &it->second;

        if (curIdx != idx) { SelectObject(dc, fonts[idx]); curIdx = idx; }
        wchar_t wc = (wchar_t)cp;
        SIZE sz;
        GetTextExtentPoint32W(dc, &wc, 1, &sz);
        int adv = sz.cx, h = cellH[idx];
        if (adv <= 0) adv = 1;
        int cw = adv < W ? adv : W;
        int ch = h < H ? h : H;

        RECT rc = {0, 0, cw, ch};
        ExtTextOutW(dc, 0, 0, ETO_OPAQUE, &rc, L"", 0, nullptr); // clear to black
        TextOutW(dc, 0, 0, &wc, 1);
        GdiFlush();

        CGlyph g;
        g.w = cw; g.h = ch; g.adv = adv;
        g.cov.resize((size_t)cw * ch);
        for (int py = 0; py < ch; ++py) {
            const uint8_t* row = bits + (size_t)py * W * 4;
            for (int pxs = 0; pxs < cw; ++pxs)
                g.cov[(size_t)py * cw + pxs] = row[(size_t)pxs * 4]; // white -> coverage
        }
        return &glyphs.emplace(key, std::move(g)).first->second;
    }
};

GdiText& gt() { static GdiText g; return g; }

} // namespace

int Render2D::TextWidth(const std::string& s, int scale) const {
    GdiText& g = gt();
    if (!g.ok) return 0;
    int idx = g.idxForScale(scale);
    long w = 0;
    for (wchar_t wc : to_utf16(s)) {
        const CGlyph* cg = g.glyph(idx, (uint32_t)wc);
        if (cg) w += cg->adv;
    }
    return (int)w;
}

int Render2D::TextHeight(int scale) const { return pxForScale(scale); }

void Render2D::Text(float x, float y, const std::string& s, Color c, int scale) {
    GdiText& g = gt();
    if (!g.ok || s.empty()) return;
    int idx = g.idxForScale(scale);
    int ox = (int)std::lround(x), oy = (int)std::lround(y);
    float pen = 0.f;
    for (wchar_t wc : to_utf16(s)) {
        const CGlyph* cg = g.glyph(idx, (uint32_t)wc);
        if (!cg) continue;
        int gx = ox + (int)std::lround(pen);
        for (int py = 0; py < cg->h; ++py)
            for (int pxs = 0; pxs < cg->w; ++pxs) {
                uint8_t cov = cg->cov[(size_t)py * cg->w + pxs];
                if (cov) Blend(gx + pxs, oy + py, c, cov / 255.f);
            }
        pen += cg->adv;
    }
}
#endif // _WIN32
