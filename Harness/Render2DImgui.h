#pragma once
// ImGui backend for the Render2D API. Selected by -DR2D_IMGUI from Render2D.h.
// Implements the exact same primitive/text surface as the software renderer, but
// emits into an ImGui ImDrawList (GPU-accelerated via the Win32+DX9 backend), so
// Hud.cpp / Menu.cpp / Scene.h render 1:1 with zero changes — only the backend
// differs. Forward-declares ImGui types so menu/HUD TUs don't need imgui.h.

#include <cstdint>
#include <string>
#include <algorithm>

struct ImDrawList;
struct ImFont;

// Wire fonts (body=11px, head=16px, word=22px) after the ImGui atlas is built.
void R2DImguiSetFonts(ImFont* body, ImFont* head, ImFont* word);

class Render2D {
public:
    Render2D(int w, int h) : W(w), H(h) {}

    int width() const { return W; }
    int height() const { return H; }

    // Per-frame: the draw list everything is emitted into.
    void beginFrame(ImDrawList* dl) { dl_ = dl; }

    // No-ops/stubs so Scene.h (software bg-cache path) still compiles unused.
    const uint8_t* data() const { return nullptr; }
    void Blit(const uint8_t*) {}
    void Clear(Color) {}
    void Blend(int, int, Color, float = 1.f) {}
    bool savePNG(const std::string&) const { return false; }

    void BoxFilled(float x, float y, float w, float h, Color c);
    void RoundedBox(float x, float y, float w, float h, float r, Color c);
    void RoundedBoxOutline(float x, float y, float w, float h, float r, float thick, Color c);
    void GradientBox(float x, float y, float w, float h, Color c1, Color c2,
                     bool vertical = true, float r = 0.f);
    void Line(float ax, float ay, float bx, float by, Color c, float thick = 1.f);
    void CircleFilled(float cx, float cy, float rad, Color c);
    void Circle(float cx, float cy, float rad, float thick, Color c);
    void GlowCircle(float cx, float cy, float rad, Color c);
    void Shadow(float x, float y, float w, float h, float r, float spread, Color c);
    void Triangle(float ax, float ay, float bx, float by, float cx, float cy, Color col);

    int  TextWidth(const std::string& s, int scale = 1) const;
    int  TextHeight(int scale = 1) const;
    void Text(float x, float y, const std::string& s, Color c, int scale = 1);

    void TextCentered(float cx, float y, const std::string& s, Color c, int scale = 1) {
        Text(cx - TextWidth(s, scale) * 0.5f, y, s, c, scale);
    }
    void TextMid(float cx, float cy, const std::string& s, Color c, int scale = 1) {
        Text(cx - TextWidth(s, scale) * 0.5f, cy - TextHeight(scale) * 0.5f, s, c, scale);
    }
    void TextLMid(float x, float cy, const std::string& s, Color c, int scale = 1) {
        Text(x, cy - TextHeight(scale) * 0.5f, s, c, scale);
    }
    void TextRMid(float xr, float cy, const std::string& s, Color c, int scale = 1) {
        Text(xr - TextWidth(s, scale), cy - TextHeight(scale) * 0.5f, s, c, scale);
    }

private:
    int W, H;
    ImDrawList* dl_ = nullptr;
};
