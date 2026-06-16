#include "Hud.h"
#include "Theme.h"
#include "../src/Config.h"
#include <string>
#include <vector>
#include <cmath>

namespace {

const Theme& t() { return theme(); }

// text with a soft dark shadow for legibility over the world (vertically centered)
void txt(Render2D& r, float x, float cy, const std::string& s, Color c, int scale = 1) {
    r.Text(x + 1, cy - r.TextHeight(scale) * 0.5f + 1, s, Color(0, 0, 0, 150), scale);
    r.TextLMid(x, cy, s, c, scale);
}
void txtC(Render2D& r, float cx, float cy, const std::string& s, Color c, int scale = 1) {
    txt(r, cx - r.TextWidth(s, scale) * 0.5f, cy, s, c, scale);
}

Color hpColor(float f) { return Color::lerp(t().bad, t().good, std::clamp(f, 0.f, 1.f)); }

void cornerBox(Render2D& r, float x, float y, float w, float h, Color c) {
    float len = std::min(w, h) * 0.28f;
    auto seg = [&](float ax, float ay, float bx, float by) {
        r.Line(ax, ay, bx, by, Color(0, 0, 0, 200), 3.0f);
        r.Line(ax, ay, bx, by, c, 1.3f);
    };
    seg(x, y, x + len, y);               seg(x, y, x, y + len);
    seg(x + w, y, x + w - len, y);       seg(x + w, y, x + w, y + len);
    seg(x, y + h, x + len, y + h);       seg(x, y + h, x, y + h - len);
    seg(x + w, y + h, x + w - len, y + h); seg(x + w, y + h, x + w, y + h - len);
}

struct EspPlayer {
    float x, y, w, h;
    std::string name, weapon;
    float hp;
    bool hit;
    std::vector<std::string> flags;
};

void drawEsp(Render2D& r, const EspPlayer& p) {
    Color box = p.hit ? t().accent2 : Color(232, 233, 240);
    cornerBox(r, p.x, p.y, p.w, p.h, box);

    float bx = p.x - 7, bw = 3;
    r.RoundedBox(bx - 1, p.y - 1, bw + 2, p.h + 2, 1.5f, Color(0, 0, 0, 180));
    float fillH = p.h * p.hp;
    r.RoundedBox(bx, p.y + (p.h - fillH), bw, fillH, 1.5f, hpColor(p.hp));

    txtC(r, p.x + p.w / 2, p.y - 9, p.name, Color(232, 233, 240), 1);
    txtC(r, p.x + p.w / 2, p.y + p.h + 8, p.weapon, t().textDim, 1);

    float fy = p.y + 5;
    for (auto& f : p.flags) {
        Color fc = (f == "hit") ? t().accent2 : (f == "flashed" ? Color(220, 220, 235) : t().accent);
        txt(r, p.x + p.w + 6, fy, f, fc, 1);
        fy += 12;
    }
}

// premium overlay panel: rounded, soft shadow, thin accent top hairline
void panel(Render2D& r, float x, float y, float w, float h) {
    r.Shadow(x, y, w, h, 4, 10, Color(0, 0, 0, 130));
    r.RoundedBox(x, y, w, h, 4, t().panel.withA(232));
    r.RoundedBoxOutline(x, y, w, h, 4, 1.f, t().line);
    r.RoundedBox(x + 8, y, w - 16, 1.5f, 1, t().accent.withA(150));
}

// centered, multi-color text line (for the hit log) with a faint backing pill
struct Seg { std::string s; Color c; };
void drawSegsPill(Render2D& r, float cx, float cy, const std::vector<Seg>& segs, float alpha) {
    float total = 0;
    for (auto& sg : segs) total += r.TextWidth(sg.s, 1);
    float pw = total + 20, ph = 18;
    r.RoundedBox(cx - pw / 2, cy - ph / 2, pw, ph, 4, Color(9, 9, 11).withAf(0.82f * alpha));
    r.RoundedBoxOutline(cx - pw / 2, cy - ph / 2, pw, ph, 4, 1.f, Color(255, 255, 255, 16).withAf(alpha));
    float px = cx - total / 2;
    for (auto& sg : segs) {
        r.TextLMid(px, cy, sg.s, sg.c.withAf(alpha), 1);
        px += r.TextWidth(sg.s, 1);
    }
}

} // namespace

void Hud::DrawBackdrop(Render2D& r) {
    int W = r.width(), H = r.height();
    r.GradientBox(0, 0, W, H * 0.62f, Color(70, 92, 120), Color(120, 142, 168), true);
    r.GradientBox(0, H * 0.6f, W, H * 0.42f, Color(58, 54, 46), Color(34, 32, 28), true);
    Color b1(96, 100, 110), b2(80, 84, 96), b3(70, 74, 86);
    r.BoxFilled(120, 250, 180, 200, b1);
    r.BoxFilled(360, 180, 140, 270, b2);
    r.BoxFilled(980, 220, 200, 230, b3);
    r.BoxFilled(820, 300, 110, 150, b1);
    r.GlowCircle(W * 0.82f, H * 0.18f, 120, Color(255, 240, 200, 90));
    for (int i = 0; i < 60; ++i) {
        float a = i / 60.f;
        r.RoundedBoxOutline(i, i, W - 2 * i, H - 2 * i, 0, 1.f, Color(0, 0, 0, (int)(28 * (1 - a))));
    }
}

void Hud::DrawWorld(Render2D& r) {
    int W = r.width(), H = r.height();
    float cx = W / 2.f, cy = H / 2.f;

    EspPlayer p1{ 470, 250, 70, 170, "trash_kid", "ak-47", 0.62f, true, {"hit", "scoped"} };
    EspPlayer p2{ 1000, 280, 60, 150, "noscoper", "awp", 1.0f, false, {"flashed"} };
    drawEsp(r, p1);
    drawEsp(r, p2);

    r.Line(cx, cy + 6, p1.x + p1.w / 2, p1.y + p1.h * 0.25f, t().accent2.withA(170), 1.3f);
    r.GlowCircle(p1.x + p1.w / 2, p1.y + p1.h * 0.25f, 8, t().accent2.withA(150));

    // off-screen arrow
    {
        float ang = 3.1415926f * 1.15f, rad = 92;
        float ax = cx + std::cos(ang) * rad, ay = cy + std::sin(ang) * rad;
        float dx = std::cos(ang), dy = std::sin(ang), px = -dy, py = dx;
        r.Triangle(ax + dx * 11, ay + dy * 11,
                   ax - dx * 5 + px * 7, ay - dy * 5 + py * 7,
                   ax - dx * 5 - px * 7, ay - dy * 5 - py * 7, t().accent);
    }

    // crosshair
    r.BoxFilled(cx - 7, cy - 0.5f, 5, 1.5f, Color(255, 255, 255, 220));
    r.BoxFilled(cx + 2, cy - 0.5f, 5, 1.5f, Color(255, 255, 255, 220));
    r.BoxFilled(cx - 0.5f, cy - 7, 1.5f, 5, Color(255, 255, 255, 220));
    r.BoxFilled(cx - 0.5f, cy + 2, 1.5f, 5, Color(255, 255, 255, 220));

    // manual-AA arrows (left active)
    auto aaArrow = [&](float ang, bool on) {
        float rad = 26;
        float ax = cx + std::cos(ang) * rad, ay = cy + std::sin(ang) * rad;
        float dx = std::cos(ang), dy = std::sin(ang), px = -dy, py = dx;
        Color c = on ? t().accent : Color(255, 255, 255, 60);
        r.Triangle(ax + dx * 7, ay + dy * 7, ax - px * 5, ay - py * 5, ax + px * 5, ay + py * 5, c);
    };
    aaArrow(3.14159f, true);
    aaArrow(0.f, false);
    aaArrow(-3.14159f / 2, false);
}

void Hud::DrawOverlays(Render2D& r) {
    int W = r.width(), H = r.height();

    // ---------- watermark (top-right) ----------
    if (cfg::g_cfg.visuals.watermark) {
        std::vector<std::string> parts = {"jazzhook", "skippy", "64 tick", "120 fps", "13 ms", "09:33:35"};
        float gap = 12;
        float tw = 0;
        for (size_t i = 0; i < parts.size(); ++i) tw += r.TextWidth(parts[i], 1) + (i ? gap : 0);
        float w = tw + 22, h = 22, x = W - w - 16, y = 16, cyc = y + h / 2;
        panel(r, x, y, w, h);
        float px = x + 11;
        for (size_t i = 0; i < parts.size(); ++i) {
            Color c = (i == 0) ? t().accent : t().text;
            r.TextLMid(px, cyc, parts[i], c, 1);
            px += r.TextWidth(parts[i], 1);
            if (i + 1 < parts.size()) { r.CircleFilled(px + gap / 2, cyc, 1.2f, t().textFaint); px += gap; }
        }
    }

    // ---------- keybind list (right) ----------
    if (cfg::g_cfg.visuals.keybinds) {
        const auto& rc = cfg::g_cfg.rage; const auto& ac = cfg::g_cfg.aa;
        struct KB { std::string name, mode; };
        std::vector<KB> binds = {
            {"double tap", rc.double_tap ? "on" : "off"},
            {"hide shots", rc.hide_shots ? "holding" : "off"},
            {"fake duck", ac.fake_duck ? "on" : "off"},
            {"manual back", ac.manual_back ? "holding" : "off"},
        };
        float w = 152, rowH = 17, x = W - w - 16, y = 50;
        float h = 24 + binds.size() * rowH + 6;
        panel(r, x, y, w, h);
        r.TextMid(x + w / 2, y + 12, "keybinds", t().textDim, 1);
        r.BoxFilled(x + 10, y + 21, w - 20, 1, t().line);
        float yy = y + 30 + rowH / 2;
        for (auto& b : binds) {
            Color mc = (b.mode == "off") ? t().textFaint : t().accent;
            r.TextLMid(x + 11, yy, b.name, t().text, 1);
            r.TextRMid(x + w - 11, yy, b.mode, mc, 1);
            yy += rowH;
        }
    }

    // ---------- spectator list (right) ----------
    if (cfg::g_cfg.visuals.spectators) {
        struct SP { std::string name, mode; };
        std::vector<SP> specs = { {"cancer_kid", "1st"}, {"silent", "free"} };
        float w = 152, rowH = 17, x = W - w - 16, y = 186;
        float h = 24 + specs.size() * rowH + 6;
        panel(r, x, y, w, h);
        r.TextMid(x + w / 2, y + 12, "spectators", t().textDim, 1);
        r.BoxFilled(x + 10, y + 21, w - 20, 1, t().line);
        float yy = y + 30 + rowH / 2;
        for (auto& s : specs) {
            r.TextLMid(x + 11, yy, s.name, t().text, 1);
            r.TextRMid(x + w - 11, yy, s.mode, t().textDim, 1);
            yy += rowH;
        }
    }

    // ---------- bottom-center indicators ----------
    float indY = H - 58;
    {
        struct Ind { std::string label; bool on; };
        std::vector<Ind> inds = {
            {"dt", cfg::g_cfg.rage.double_tap},
            {"hide", cfg::g_cfg.rage.hide_shots},
            {"baim", cfg::g_cfg.aa.manual_back},
            {"fake", cfg::g_cfg.aa.fake_duck},
            {"safe", cfg::g_cfg.rage.auto_stop},
        };
        float gap = 7;
        std::vector<float> ws; float total = 0;
        for (auto& i : inds) { float w = r.TextWidth(i.label, 1) + 18; ws.push_back(w); total += w + gap; }
        total -= gap;
        float x = W / 2.f - total / 2.f, h = 19;
        for (size_t i = 0; i < inds.size(); ++i) {
            float w = ws[i], cyc = indY;
            if (inds[i].on) {
                r.RoundedBox(x, cyc - h / 2, w, h, 4, t().accent.withA(40));
                r.RoundedBoxOutline(x, cyc - h / 2, w, h, 4, 1.f, t().accent.withA(150));
                r.TextMid(x + w / 2, cyc, inds[i].label, t().accent, 1);
            } else {
                r.RoundedBoxOutline(x, cyc - h / 2, w, h, 4, 1.f, t().line);
                r.TextMid(x + w / 2, cyc, inds[i].label, t().textFaint, 1);
            }
            x += w + gap;
        }
    }

    // ---------- hit log (centered, above indicators) ----------
    // Detailed gamesense-style line, modeled on the user's reference shot:
    //   jazzhook  Hit <victim> in the <hitbox> for <dmg> damage (<hp> health left) (hc: <x>% -> bt: <y>t)
    if (cfg::g_cfg.visuals.hit_log) {
        auto hit = [&](const std::string& victim, const std::string& hitbox,
                       int dmg, int hp, int hc, int bt) {
            std::vector<Seg> s;
            Color cream = t().text, dim = t().textDim, acc = t().accent;
            s.push_back({"jazzhook", acc});
            s.push_back({"  Hit ", cream});
            s.push_back({victim, acc});
            s.push_back({" in the ", cream});
            s.push_back({hitbox, acc});
            s.push_back({" for ", cream});
            s.push_back({std::to_string(dmg), acc});
            s.push_back({" damage ", cream});
            s.push_back({"(", dim});
            s.push_back({std::to_string(hp), hp == 0 ? t().bad : cream});
            s.push_back({" health left) ", dim});
            s.push_back({"(hc: ", dim});
            s.push_back({std::to_string(hc) + "%", acc});
            s.push_back({" → bt: ", dim});
            s.push_back({std::to_string(bt) + "t", acc});
            s.push_back({")", dim});
            return s;
        };
        std::vector<std::vector<Seg>> log = {
            hit("дамский угодник", "head", 289, 0, 98, 12),
            hit("trash_kid", "chest", 134, 27, 76, 8),
            hit("noscoper", "stomach", 41, 63, 64, 4),
        };
        float baseY = indY - 28; // freshest line sits just above the indicators
        float fades[] = {1.0f, 0.66f, 0.4f};
        for (size_t i = 0; i < log.size(); ++i)
            drawSegsPill(r, W / 2.f, baseY - i * 21, log[i], fades[i]);
    }

    // ---------- toast (bottom-right) ----------
    {
        std::string msg = "config  \"hvh_main\"  loaded";
        float w = r.TextWidth(msg, 1) + 28, h = 24, x = W - w - 16, y = H - h - 16, cyc = y + h / 2;
        panel(r, x, y, w, h);
        r.RoundedBox(x + 8, cyc - 6, 2.5f, 12, 1.5f, t().good);
        r.TextLMid(x + 16, cyc, msg, t().text, 1);
    }
}
