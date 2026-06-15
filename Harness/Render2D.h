#pragma once
// Render2D — software-rasterized 2D primitives with an API named to match
// ArcticTech's CRender (BoxFilled / GradientBox / Line / Circle / Text / ...).
// In the real mod this same call surface is backed by DirectX 9; here it is a
// CPU framebuffer so the harness can dump PNG previews on any machine.
//
// The whole point: the menu/HUD drawing code below is written against THIS API,
// so porting into the mod means swapping the backend, not rewriting the UI.

#include <cstdint>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

struct Color {
    uint8_t r = 0, g = 0, b = 0, a = 255;
    Color() = default;
    Color(int r_, int g_, int b_, int a_ = 255)
        : r((uint8_t)r_), g((uint8_t)g_), b((uint8_t)b_), a((uint8_t)a_) {}

    Color withA(int na) const { return Color(r, g, b, na); }
    Color withAf(float f) const { return Color(r, g, b, (int)std::round(a * std::clamp(f, 0.f, 1.f))); }

    static Color lerp(const Color& x, const Color& y, float t) {
        t = std::clamp(t, 0.f, 1.f);
        return Color(
            (int)std::round(x.r + (y.r - x.r) * t),
            (int)std::round(x.g + (y.g - x.g) * t),
            (int)std::round(x.b + (y.b - x.b) * t),
            (int)std::round(x.a + (y.a - x.a) * t));
    }
};

class Render2D {
public:
    Render2D(int w, int h) : W(w), H(h), px((size_t)w * h * 4, 0) {}

    int width() const { return W; }
    int height() const { return H; }
    const uint8_t* data() const { return px.data(); }

    void Clear(Color c) {
        for (int i = 0; i < W * H; ++i) setRaw(i, c);
    }

    // Single pixel, alpha-blended, with coverage (for AA).
    void Blend(int x, int y, Color c, float cov = 1.f) {
        if (x < 0 || y < 0 || x >= W || y >= H) return;
        cov = std::clamp(cov, 0.f, 1.f);
        float a = (c.a / 255.f) * cov;
        if (a <= 0.f) return;
        size_t i = ((size_t)y * W + x) * 4;
        px[i + 0] = (uint8_t)std::round(px[i + 0] + (c.r - px[i + 0]) * a);
        px[i + 1] = (uint8_t)std::round(px[i + 1] + (c.g - px[i + 1]) * a);
        px[i + 2] = (uint8_t)std::round(px[i + 2] + (c.b - px[i + 2]) * a);
        px[i + 3] = 255;
    }

    void BoxFilled(float x, float y, float w, float h, Color c) {
        RoundedBox(x, y, w, h, 0.f, c);
    }

    // Anti-aliased rounded rectangle via signed distance field.
    void RoundedBox(float x, float y, float w, float h, float r, Color c) {
        r = std::min(r, std::min(w, h) * 0.5f);
        int x0 = (int)std::floor(x) - 1, y0 = (int)std::floor(y) - 1;
        int x1 = (int)std::ceil(x + w) + 1, y1 = (int)std::ceil(y + h) + 1;
        float cx = x + w * 0.5f, cy = y + h * 0.5f;
        float hx = w * 0.5f, hy = h * 0.5f;
        for (int py = y0; py <= y1; ++py)
            for (int px2 = x0; px2 <= x1; ++px2) {
                float d = sdRound(px2 + 0.5f, py + 0.5f, cx, cy, hx, hy, r);
                float cov = std::clamp(0.5f - d, 0.f, 1.f);
                if (cov > 0.f) Blend(px2, py, c, cov);
            }
    }

    void RoundedBoxOutline(float x, float y, float w, float h, float r, float thick, Color c) {
        r = std::min(r, std::min(w, h) * 0.5f);
        int x0 = (int)std::floor(x) - 1, y0 = (int)std::floor(y) - 1;
        int x1 = (int)std::ceil(x + w) + 1, y1 = (int)std::ceil(y + h) + 1;
        float cx = x + w * 0.5f, cy = y + h * 0.5f;
        float hx = w * 0.5f, hy = h * 0.5f;
        for (int py = y0; py <= y1; ++py)
            for (int px2 = x0; px2 <= x1; ++px2) {
                float d = std::fabs(sdRound(px2 + 0.5f, py + 0.5f, cx, cy, hx, hy, r));
                float cov = std::clamp(thick * 0.5f + 0.5f - d, 0.f, 1.f);
                if (cov > 0.f) Blend(px2, py, c, cov);
            }
    }

    // Vertical (default) or horizontal two-stop gradient, rounded.
    void GradientBox(float x, float y, float w, float h, Color c1, Color c2,
                     bool vertical = true, float r = 0.f) {
        r = std::min(r, std::min(w, h) * 0.5f);
        int x0 = (int)std::floor(x) - 1, y0 = (int)std::floor(y) - 1;
        int x1 = (int)std::ceil(x + w) + 1, y1 = (int)std::ceil(y + h) + 1;
        float cx = x + w * 0.5f, cy = y + h * 0.5f;
        float hx = w * 0.5f, hy = h * 0.5f;
        for (int py = y0; py <= y1; ++py)
            for (int px2 = x0; px2 <= x1; ++px2) {
                float d = sdRound(px2 + 0.5f, py + 0.5f, cx, cy, hx, hy, r);
                float cov = std::clamp(0.5f - d, 0.f, 1.f);
                if (cov <= 0.f) continue;
                float t = vertical ? (py - y) / h : (px2 - x) / w;
                Blend(px2, py, Color::lerp(c1, c2, std::clamp(t, 0.f, 1.f)), cov);
            }
    }

    // Anti-aliased thick line (segment SDF).
    void Line(float ax, float ay, float bx, float by, Color c, float thick = 1.f) {
        int x0 = (int)std::floor(std::min(ax, bx) - thick - 1);
        int x1 = (int)std::ceil(std::max(ax, bx) + thick + 1);
        int y0 = (int)std::floor(std::min(ay, by) - thick - 1);
        int y1 = (int)std::ceil(std::max(ay, by) + thick + 1);
        for (int py = y0; py <= y1; ++py)
            for (int px2 = x0; px2 <= x1; ++px2) {
                float d = sdSeg(px2 + 0.5f, py + 0.5f, ax, ay, bx, by);
                float cov = std::clamp(thick * 0.5f + 0.5f - d, 0.f, 1.f);
                if (cov > 0.f) Blend(px2, py, c, cov);
            }
    }

    void CircleFilled(float cx, float cy, float rad, Color c) {
        int x0 = (int)std::floor(cx - rad - 1), x1 = (int)std::ceil(cx + rad + 1);
        int y0 = (int)std::floor(cy - rad - 1), y1 = (int)std::ceil(cy + rad + 1);
        for (int py = y0; py <= y1; ++py)
            for (int px2 = x0; px2 <= x1; ++px2) {
                float d = std::hypot(px2 + 0.5f - cx, py + 0.5f - cy) - rad;
                float cov = std::clamp(0.5f - d, 0.f, 1.f);
                if (cov > 0.f) Blend(px2, py, c, cov);
            }
    }

    void Circle(float cx, float cy, float rad, float thick, Color c) {
        int x0 = (int)std::floor(cx - rad - thick - 1), x1 = (int)std::ceil(cx + rad + thick + 1);
        int y0 = (int)std::floor(cy - rad - thick - 1), y1 = (int)std::ceil(cy + rad + thick + 1);
        for (int py = y0; py <= y1; ++py)
            for (int px2 = x0; px2 <= x1; ++px2) {
                float d = std::fabs(std::hypot(px2 + 0.5f - cx, py + 0.5f - cy) - rad);
                float cov = std::clamp(thick * 0.5f + 0.5f - d, 0.f, 1.f);
                if (cov > 0.f) Blend(px2, py, c, cov);
            }
    }

    // Soft radial glow (used for accent halos / glow circles).
    void GlowCircle(float cx, float cy, float rad, Color c) {
        int x0 = (int)std::floor(cx - rad - 1), x1 = (int)std::ceil(cx + rad + 1);
        int y0 = (int)std::floor(cy - rad - 1), y1 = (int)std::ceil(cy + rad + 1);
        for (int py = y0; py <= y1; ++py)
            for (int px2 = x0; px2 <= x1; ++px2) {
                float dist = std::hypot(px2 + 0.5f - cx, py + 0.5f - cy);
                if (dist > rad) continue;
                float f = 1.f - dist / rad;
                Blend(px2, py, c, f * f);
            }
    }

    // Soft drop shadow for a rounded panel.
    void Shadow(float x, float y, float w, float h, float r, float spread, Color c) {
        int x0 = (int)std::floor(x - spread - 1), x1 = (int)std::ceil(x + w + spread + 1);
        int y0 = (int)std::floor(y - spread - 1), y1 = (int)std::ceil(y + h + spread + 1);
        float cx = x + w * 0.5f, cy = y + h * 0.5f, hx = w * 0.5f, hy = h * 0.5f;
        for (int py = y0; py <= y1; ++py)
            for (int px2 = x0; px2 <= x1; ++px2) {
                float d = sdRound(px2 + 0.5f, py + 0.5f, cx, cy, hx, hy, r);
                if (d <= 0.f) continue;
                float f = 1.f - std::clamp(d / spread, 0.f, 1.f);
                Blend(px2, py, c, f * f * (c.a / 255.f));
            }
    }

    // Filled triangle (arrows / indicators), via barycentric coverage.
    void Triangle(float ax, float ay, float bx, float by, float cx, float cy, Color col) {
        int x0 = (int)std::floor(std::min({ax, bx, cx}) - 1);
        int x1 = (int)std::ceil(std::max({ax, bx, cx}) + 1);
        int y0 = (int)std::floor(std::min({ay, by, cy}) - 1);
        int y1 = (int)std::ceil(std::max({ay, by, cy}) + 1);
        auto edge = [](float px_, float py_, float x0_, float y0_, float x1_, float y1_) {
            return (px_ - x0_) * (y1_ - y0_) - (py_ - y0_) * (x1_ - x0_);
        };
        for (int py = y0; py <= y1; ++py)
            for (int px2 = x0; px2 <= x1; ++px2) {
                // 2x2 supersample for cheap edge AA
                int hits = 0;
                for (int sy = 0; sy < 2; ++sy)
                    for (int sx = 0; sx < 2; ++sx) {
                        float fx = px2 + 0.25f + sx * 0.5f;
                        float fy = py + 0.25f + sy * 0.5f;
                        float w0 = edge(fx, fy, bx, by, cx, cy);
                        float w1 = edge(fx, fy, cx, cy, ax, ay);
                        float w2 = edge(fx, fy, ax, ay, bx, by);
                        bool in = (w0 >= 0 && w1 >= 0 && w2 >= 0) ||
                                  (w0 <= 0 && w1 <= 0 && w2 <= 0);
                        if (in) hits++;
                    }
                if (hits) Blend(px2, py, col, hits / 4.f);
            }
    }

    // ---- Text (embedded 8x8 bitmap font, integer scaled) ----
    int TextWidth(const std::string& s, int scale = 1) const {
        return (int)s.size() * 6 * scale; // 5px glyph + 1px gap, ×scale
    }
    int TextHeight(int scale = 1) const { return 7 * scale; }

    void Text(float x, float y, const std::string& s, Color c, int scale = 1);
    void TextCentered(float cx, float y, const std::string& s, Color c, int scale = 1) {
        Text(cx - TextWidth(s, scale) * 0.5f, y, s, c, scale);
    }

    bool savePNG(const std::string& path) const;

private:
    int W, H;
    std::vector<uint8_t> px;

    void setRaw(int i, Color c) {
        px[(size_t)i * 4 + 0] = c.r;
        px[(size_t)i * 4 + 1] = c.g;
        px[(size_t)i * 4 + 2] = c.b;
        px[(size_t)i * 4 + 3] = 255;
    }

    static float sdRound(float px_, float py_, float cx, float cy,
                         float hx, float hy, float r) {
        float qx = std::fabs(px_ - cx) - hx + r;
        float qy = std::fabs(py_ - cy) - hy + r;
        float ox = std::max(qx, 0.f), oy = std::max(qy, 0.f);
        float outside = std::hypot(ox, oy);
        float inside = std::min(std::max(qx, qy), 0.f);
        return outside + inside - r;
    }

    static float sdSeg(float px_, float py_, float ax, float ay, float bx, float by) {
        float vx = bx - ax, vy = by - ay;
        float wx = px_ - ax, wy = py_ - ay;
        float len2 = vx * vx + vy * vy;
        float t = len2 > 0 ? std::clamp((wx * vx + wy * vy) / len2, 0.f, 1.f) : 0.f;
        return std::hypot(px_ - (ax + vx * t), py_ - (ay + vy * t));
    }
};
