#include "Render2D.h"
#ifdef R2D_IMGUI
// ImGui implementation of the Render2D primitive/text API (see Render2DImgui.h).
#include "imgui.h"
#include <cfloat>
#include <cmath>

namespace {
ImFont* g_body = nullptr; // 11px
ImFont* g_head = nullptr; // 16px
ImFont* g_word = nullptr; // 22px

inline ImU32 col(Color c) { return IM_COL32(c.r, c.g, c.b, c.a); }
inline ImVec2 v(float x, float y) { return ImVec2(x, y); }
int pxForScale(int s) { return s <= 1 ? 11 : (s == 2 ? 16 : 22); }
ImFont* fontForScale(int s) { return s <= 1 ? g_body : (s == 2 ? g_head : g_word); }
}

void R2DImguiSetFonts(ImFont* body, ImFont* head, ImFont* word) {
    g_body = body; g_head = head; g_word = word;
}

void Render2D::BoxFilled(float x, float y, float w, float h, Color c) {
    if (dl_) dl_->AddRectFilled(v(x, y), v(x + w, y + h), col(c));
}

void Render2D::RoundedBox(float x, float y, float w, float h, float r, Color c) {
    if (dl_) dl_->AddRectFilled(v(x, y), v(x + w, y + h), col(c), r);
}

void Render2D::RoundedBoxOutline(float x, float y, float w, float h, float r, float thick, Color c) {
    if (dl_) dl_->AddRect(v(x, y), v(x + w, y + h), col(c), r, 0, thick);
}

void Render2D::GradientBox(float x, float y, float w, float h, Color c1, Color c2,
                           bool vertical, float r) {
    if (!dl_) return;
    ImU32 a = col(c1), b = col(c2);
    // AddRectFilledMultiColor: upper-left, upper-right, bottom-right, bottom-left
    if (vertical)
        dl_->AddRectFilledMultiColor(v(x, y), v(x + w, y + h), a, a, b, b);
    else
        dl_->AddRectFilledMultiColor(v(x, y), v(x + w, y + h), a, b, b, a);
    (void)r; // ImGui multicolor rects are square-cornered; rounding ignored
}

void Render2D::Line(float ax, float ay, float bx, float by, Color c, float thick) {
    if (dl_) dl_->AddLine(v(ax, ay), v(bx, by), col(c), thick);
}

void Render2D::CircleFilled(float cx, float cy, float rad, Color c) {
    if (dl_) dl_->AddCircleFilled(v(cx, cy), rad, col(c));
}

void Render2D::Circle(float cx, float cy, float rad, float thick, Color c) {
    if (dl_) dl_->AddCircle(v(cx, cy), rad, col(c), 0, thick);
}

void Render2D::GlowCircle(float cx, float cy, float rad, Color c) {
    if (!dl_) return;
    // soft-ish: a few stacked translucent discs
    for (int i = 3; i >= 1; --i) {
        Color g = c; g.a = (uint8_t)(c.a / (i + 1));
        dl_->AddCircleFilled(v(cx, cy), rad * i / 3.f, col(g));
    }
}

void Render2D::Shadow(float x, float y, float w, float h, float r, float spread, Color c) {
    if (!dl_) return;
    // approximate a soft drop shadow with a few expanding translucent rounded rects
    int layers = 4;
    for (int i = layers; i >= 1; --i) {
        float e = spread * i / layers;
        Color s = c; s.a = (uint8_t)(c.a / (layers + 1));
        dl_->AddRectFilled(v(x - e, y - e), v(x + w + e, y + h + e), col(s), r + e);
    }
}

void Render2D::Triangle(float ax, float ay, float bx, float by, float cx, float cy, Color color) {
    if (dl_) dl_->AddTriangleFilled(v(ax, ay), v(bx, by), v(cx, cy), col(color));
}

int Render2D::TextWidth(const std::string& s, int scale) const {
    ImFont* f = fontForScale(scale);
    if (!f || s.empty()) return 0;
    ImVec2 sz = f->CalcTextSizeA((float)pxForScale(scale), FLT_MAX, 0.f,
                                 s.c_str(), s.c_str() + s.size());
    return (int)(sz.x + 0.5f);
}

int Render2D::TextHeight(int scale) const { return pxForScale(scale); }

void Render2D::Text(float x, float y, const std::string& s, Color c, int scale) {
    if (!dl_ || s.empty()) return;
    ImFont* f = fontForScale(scale);
    dl_->AddText(f, (float)pxForScale(scale), v(x, y), col(c),
                 s.c_str(), s.c_str() + s.size());
}
#endif // R2D_IMGUI
