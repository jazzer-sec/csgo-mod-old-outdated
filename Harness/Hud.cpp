#include "Hud.h"
#include "Theme.h"
#include <string>
#include <vector>
#include <cmath>

namespace {

const Theme& t() { return theme(); }

// text with a 1px dark shadow for legibility over the world
void txtS(Render2D& r, float x, float y, const std::string& s, Color c, int scale = 1) {
    r.Text(x + 1, y + 1, s, Color(0, 0, 0, 170), scale);
    r.Text(x, y, s, c, scale);
}
void txtSC(Render2D& r, float cx, float y, const std::string& s, Color c, int scale = 1) {
    txtS(r, cx - r.TextWidth(s, scale) * 0.5f, y, s, c, scale);
}

Color hpColor(float f) { // 1 = full (green) -> 0 (red)
    return Color::lerp(t().bad, t().good, std::clamp(f, 0.f, 1.f));
}

// classic cornered ESP box with black contrast underlay
void cornerBox(Render2D& r, float x, float y, float w, float h, Color c) {
    float len = std::min(w, h) * 0.28f;
    auto seg = [&](float ax, float ay, float bx, float by) {
        r.Line(ax, ay, bx, by, Color(0, 0, 0, 200), 3.0f);
        r.Line(ax, ay, bx, by, c, 1.4f);
    };
    // top-left
    seg(x, y, x + len, y);            seg(x, y, x, y + len);
    // top-right
    seg(x + w, y, x + w - len, y);    seg(x + w, y, x + w, y + len);
    // bottom-left
    seg(x, y + h, x + len, y + h);    seg(x, y + h, x, y + h - len);
    // bottom-right
    seg(x + w, y + h, x + w - len, y + h); seg(x + w, y + h, x + w, y + h - len);
}

struct EspPlayer {
    float x, y, w, h;
    std::string name, weapon;
    float hp;            // 0..1
    bool hit;            // flash this frame
    std::vector<std::string> flags;
};

void drawEsp(Render2D& r, const EspPlayer& p) {
    Color box = p.hit ? t().accent2 : Color(235, 236, 245);
    cornerBox(r, p.x, p.y, p.w, p.h, box);

    // health bar (left)
    float bx = p.x - 7, bw = 3;
    r.RoundedBox(bx - 1, p.y - 1, bw + 2, p.h + 2, 1.5f, Color(0, 0, 0, 180));
    float fillH = p.h * p.hp;
    r.RoundedBox(bx, p.y + (p.h - fillH), bw, fillH, 1.5f, hpColor(p.hp));

    // name above
    txtSC(r, p.x + p.w / 2, p.y - 12, p.name, Color(235, 236, 245), 1);
    // hp number at bar top
    if (p.hp < 1.f)
        txtSC(r, bx + 1, p.y + (p.h - fillH) - 10, std::to_string((int)std::round(p.hp * 100)),
              hpColor(p.hp), 1);
    // weapon below
    txtSC(r, p.x + p.w / 2, p.y + p.h + 4, p.weapon, t().textDim, 1);

    // flags (right, stacked)
    float fy = p.y;
    for (auto& f : p.flags) {
        Color fc = t().warn;
        if (f == "HIT") fc = t().accent2;
        if (f == "FLASHED") fc = Color(220, 220, 235);
        txtS(r, p.x + p.w + 6, fy, f, fc, 1);
        fy += 11;
    }
}

// rounded overlay panel with an accent edge
void panel(Render2D& r, float x, float y, float w, float h) {
    r.Shadow(x, y, w, h, 6, 10, Color(0, 0, 0, 120));
    r.RoundedBox(x, y, w, h, 6, t().panel.withA(225));
    r.RoundedBoxOutline(x, y, w, h, 6, 1.f, t().outline);
}

} // namespace

void Hud::DrawBackdrop(Render2D& r) {
    int W = r.width(), H = r.height();
    // sky gradient
    r.GradientBox(0, 0, W, H * 0.62f, Color(70, 92, 120), Color(120, 142, 168), true);
    // ground
    r.GradientBox(0, H * 0.6f, W, H * 0.42f, Color(58, 54, 46), Color(34, 32, 28), true);
    // a few faux structures for depth
    Color b1(96, 100, 110), b2(80, 84, 96), b3(70, 74, 86);
    r.BoxFilled(120, 250, 180, 200, b1);
    r.BoxFilled(360, 180, 140, 270, b2);
    r.BoxFilled(980, 220, 200, 230, b3);
    r.BoxFilled(820, 300, 110, 150, b1);
    // sun glow
    r.GlowCircle(W * 0.82f, H * 0.18f, 120, Color(255, 240, 200, 90));
    // subtle vignette
    for (int i = 0; i < 60; ++i) {
        float a = i / 60.f;
        r.RoundedBoxOutline(i, i, W - 2 * i, H - 2 * i, 0, 1.f, Color(0, 0, 0, (int)(28 * (1 - a))));
    }
}

void Hud::DrawWorld(Render2D& r) {
    int W = r.width(), H = r.height();
    float cx = W / 2.f, cy = H / 2.f;

    EspPlayer p1{ 470, 250, 70, 170, "trash_kid", "ak-47", 0.62f, true,
                  {"HIT", "SCOPED"} };
    EspPlayer p2{ 1000, 280, 60, 150, "noscoper", "awp", 1.0f, false,
                  {"FLASHED"} };
    drawEsp(r, p1);
    drawEsp(r, p2);

    // bullet tracer to the hit player
    r.Line(cx, cy + 6, p1.x + p1.w / 2, p1.y + p1.h * 0.25f, t().accent2.withA(180), 1.4f);
    r.GlowCircle(p1.x + p1.w / 2, p1.y + p1.h * 0.25f, 9, t().accent2.withA(160)); // impact

    // off-screen arrow (enemy to the left, behind us): around crosshair radius
    {
        float ang = 3.1415926f * 1.15f; // pointing lower-left
        float rad = 90;
        float ax = cx + std::cos(ang) * rad, ay = cy + std::sin(ang) * rad;
        float dx = std::cos(ang), dy = std::sin(ang);
        float px = -dy, py = dx;
        r.Triangle(ax + dx * 12, ay + dy * 12,
                   ax - dx * 6 + px * 8, ay - dy * 6 + py * 8,
                   ax - dx * 6 - px * 8, ay - dy * 6 - py * 8, t().accent);
    }

    // crosshair
    r.BoxFilled(cx - 7, cy - 0.5f, 5, 1.5f, Color(255, 255, 255, 230));
    r.BoxFilled(cx + 2, cy - 0.5f, 5, 1.5f, Color(255, 255, 255, 230));
    r.BoxFilled(cx - 0.5f, cy - 7, 1.5f, 5, Color(255, 255, 255, 230));
    r.BoxFilled(cx - 0.5f, cy + 2, 1.5f, 5, Color(255, 255, 255, 230));

    // manual-AA direction arrows around crosshair (left highlighted)
    auto aaArrow = [&](float ang, bool on) {
        float rad = 26;
        float ax = cx + std::cos(ang) * rad, ay = cy + std::sin(ang) * rad;
        float dx = std::cos(ang), dy = std::sin(ang), px = -dy, py = dx;
        Color c = on ? t().accent : Color(255, 255, 255, 70);
        r.Triangle(ax + dx * 7, ay + dy * 7,
                   ax - px * 5, ay - py * 5,
                   ax + px * 5, ay + py * 5, c);
    };
    aaArrow(3.14159f, true);    // left active
    aaArrow(0.f, false);        // right
    aaArrow(-3.14159f / 2, false); // up (back)
}

void Hud::DrawOverlays(Render2D& r) {
    int W = r.width();

    // ---------- watermark (top-right) ----------
    {
        std::vector<std::string> parts = {
            "jazzhook", "skippy", "64 tick", "120 fps", "13 ms", "09:33:35"
        };
        std::string line;
        for (size_t i = 0; i < parts.size(); ++i)
            line += (i ? "   " : "") + parts[i];
        float pad = 10;
        float tw = r.TextWidth(line, 1);
        float w = tw + pad * 2 + 8, h = 22;
        float x = W - w - 16, y = 16;
        panel(r, x, y, w, h);
        r.GradientBox(x, y, w, 2, t().accent, t().accent2, false, 1); // accent top edge
        // draw parts with accent separators
        float px = x + pad + 6;
        for (size_t i = 0; i < parts.size(); ++i) {
            Color c = (i == 0) ? t().accent : t().text;
            r.Text(px, y + 7, parts[i], c, 1);
            px += r.TextWidth(parts[i], 1);
            if (i + 1 < parts.size()) {
                r.Text(px + 4, y + 7, "|", t().textDim, 1);
                px += 4 + r.TextWidth("|", 1) + 4 + 2;
            }
        }
    }

    // ---------- keybind list (right) ----------
    {
        struct KB { std::string name, mode; };
        std::vector<KB> binds = {
            {"double tap", "on"}, {"hide shots", "holding"},
            {"baim", "holding"},  {"fake duck", "on"}, {"damage override", "off"}
        };
        float w = 150, rowH = 16, h = 22 + binds.size() * rowH + 8;
        float x = W - w - 16, y = 48;
        panel(r, x, y, w, h);
        r.Text(x + 10, y + 8, "keybinds", t().textDim, 1);
        r.BoxFilled(x + 10, y + 19, w - 20, 1, t().outline);
        float yy = y + 26;
        for (auto& b : binds) {
            Color mc = (b.mode == "off") ? t().textDim : t().accent;
            r.Text(x + 10, yy, b.name, t().text, 1);
            std::string m = "[" + b.mode + "]";
            r.Text(x + w - 10 - r.TextWidth(m, 1), yy, m, mc, 1);
            yy += rowH;
        }
    }

    // ---------- spectator list (right, below keybinds) ----------
    {
        struct SP { std::string name, mode; };
        std::vector<SP> specs = { {"cancer_kid", "1st"}, {"silent", "free"} };
        float w = 150, rowH = 16, h = 22 + specs.size() * rowH + 8;
        float x = W - w - 16, y = 188;
        panel(r, x, y, w, h);
        r.Text(x + 10, y + 8, "spectators", t().textDim, 1);
        r.BoxFilled(x + 10, y + 19, w - 20, 1, t().outline);
        float yy = y + 26;
        for (auto& s : specs) {
            r.Text(x + 10, yy, s.name, t().text, 1);
            std::string m = s.mode;
            r.Text(x + w - 10 - r.TextWidth(m, 1), yy, m, t().textDim, 1);
            yy += rowH;
        }
    }

    // ---------- hit log (bottom-left, lines rise & fade) ----------
    {
        struct Hit { std::string text; float fade; };
        std::vector<Hit> log = {
            {"hurt trash_kid  -87  head     hc 92  bt 3t", 1.0f},
            {"hurt noscoper   -24  stomach  hc 64  bt 1t", 0.7f},
            {"miss trash_kid  resolver (spin)",            0.4f},
        };
        float x = 18, baseY = r.height() - 30;
        for (size_t i = 0; i < log.size(); ++i) {
            float yy = baseY - i * 15;
            bool miss = log[i].text.rfind("miss", 0) == 0;
            Color c = (miss ? t().bad : t().accent2).withAf(log[i].fade);
            txtS(r, x, yy, log[i].text, c, 1);
        }
    }

    // ---------- bottom-center indicators ----------
    {
        struct Ind { std::string label; bool on; };
        std::vector<Ind> inds = {
            {"DT", true}, {"HIDE", true}, {"BAIM", false},
            {"FAKE", true}, {"SAFE", true}
        };
        float gap = 8, totalW = 0;
        std::vector<float> ws;
        for (auto& i : inds) { float w = r.TextWidth(i.label, 1) + 16; ws.push_back(w); totalW += w + gap; }
        totalW -= gap;
        float x = r.width() / 2.f - totalW / 2.f;
        float y = r.height() - 70;
        for (size_t i = 0; i < inds.size(); ++i) {
            float w = ws[i], h = 18;
            if (inds[i].on) {
                r.RoundedBox(x, y, w, h, 9, t().accent.withA(55));
                r.RoundedBoxOutline(x, y, w, h, 9, 1.f, t().accent);
                r.TextCentered(x + w / 2, y + 6, inds[i].label, t().accent, 1);
            } else {
                r.RoundedBoxOutline(x, y, w, h, 9, 1.f, t().outline);
                r.TextCentered(x + w / 2, y + 6, inds[i].label, t().textDim, 1);
            }
            x += w + gap;
        }
    }

    // ---------- toast notification (bottom-right) ----------
    {
        std::string msg = "config  \"hvh_main\"  loaded";
        float w = r.TextWidth(msg, 1) + 28, h = 24;
        float x = r.width() - w - 16, y = r.height() - h - 16;
        panel(r, x, y, w, h);
        r.RoundedBox(x, y + 4, 3, h - 8, 1.5f, t().good);
        r.Text(x + 14, y + 8, msg, t().text, 1);
    }
}
