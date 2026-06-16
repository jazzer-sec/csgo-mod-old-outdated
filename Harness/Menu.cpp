#include "Menu.h"
#include "Theme.h"
#include "Anim.h"
#include "../src/Config.h"
#include <string>

static const char* kTabs[] = {"rage", "anti-aim", "visuals", "misc", "configs"};
static const int kTabCount = 5;

void Menu::Draw(Render2D& r, float openT)
{
    const Theme& t = theme();
    openT = std::clamp(openT, 0.f, 1.f);
    float e = ease_out_cubic(openT);
    gAlpha = e;
    oy = (1.f - e) * 14.f;
    live = openT >= 0.999f; // only react to the mouse once fully open

    const float x = 214, y = 168, w = 584, h = 360, sideW = 152;

    r.Shadow(x, Y(y), w, h, 5, 26, C(t.shadow));
    r.RoundedBox(x, Y(y), w, h, 5, C(t.panel));

    sidebar(r, x, y, sideW, h);

    // content pane
    const float cx = x + sideW, cw = w - sideW;
    contentHead(r, cx, y, cw, kTabs[activeTab]);

    const float pad = 14, colGap = 12;
    const float bodyX = cx + pad, bodyW = cw - pad * 2;
    const float colW = (bodyW - colGap) / 2.f;
    const float lx = bodyX, rx = bodyX + colW + colGap;
    const float by0 = y + 54;

    switch (activeTab) {
    case 0:  pageRage(r, lx, rx, colW, by0);     break;
    case 1:  pageAntiAim(r, lx, rx, colW, by0);  break;
    case 2:  pageVisuals(r, lx, rx, colW, by0);  break;
    case 3:  pageMisc(r, lx, rx, colW, by0);     break;
    default: pageConfigs(r, lx, rx, colW, by0);  break;
    }

    // content footer (centered)
    r.BoxFilled(cx + pad, Y(y + h - 26), cw - pad * 2, 1, C(t.lineSoft));
    r.TextMid(cx + cw / 2.f, Y(y + h - 14), "right-click a control to bind a key", C(t.textFaint), 1);

    // dropdown (legacy PNG interaction demo) under the rage "target" combo
    if (comboOpen && activeTab == 0) {
        const char* opts[] = {"nearest", "lowest hp", "crosshair", "high damage"};
        float bw = 96, bx = lx + colW - 12 - bw, dy = Y(comboAY) + 20, ih = 16;
        float dh = ih * 4 + 6;
        r.Shadow(bx, dy, bw, dh, 3, 9, C(t.shadow));
        r.RoundedBox(bx, dy, bw, dh, 3, C(t.field));
        r.RoundedBoxOutline(bx, dy, bw, dh, 3, 1.f, C(t.line));
        for (int i = 0; i < 4; ++i) {
            float oyy = dy + 3 + i * ih;
            bool sel = (i == 0);
            if (sel) r.RoundedBox(bx + 2, oyy, bw - 4, ih, 2, C(t.accent.withA(34)));
            r.TextLMid(bx + 9, oyy + ih / 2, opts[i], C(sel ? t.accent : t.textDim), 1);
        }
    }

    r.RoundedBoxOutline(x, Y(y), w, h, 5, 1.f, C(t.line));
}

void Menu::icon(Render2D& r, int tab, float cx, float cy, Color c)
{
    switch (tab) {
    case 0:
        r.Circle(cx, cy, 4.5f, 1.1f, c);
        r.Line(cx - 7, cy, cx - 3.5f, cy, c, 1.1f);
        r.Line(cx + 3.5f, cy, cx + 7, cy, c, 1.1f);
        r.Line(cx, cy - 7, cx, cy - 3.5f, c, 1.1f);
        r.Line(cx, cy + 3.5f, cx, cy + 7, c, 1.1f);
        break;
    case 1:
        r.Triangle(cx - 7, cy, cx - 2, cy - 4, cx - 2, cy + 4, c);
        r.Triangle(cx + 7, cy, cx + 2, cy - 4, cx + 2, cy + 4, c);
        r.Line(cx - 2, cy, cx + 2, cy, c, 1.1f);
        break;
    case 2:
        r.Circle(cx, cy, 5.5f, 1.1f, c);
        r.CircleFilled(cx, cy, 2.0f, c);
        break;
    case 3:
        r.Circle(cx, cy, 3.5f, 1.3f, c);
        r.Line(cx, cy - 6.5f, cx, cy - 3.5f, c, 1.3f);
        r.Line(cx, cy + 3.5f, cx, cy + 6.5f, c, 1.3f);
        r.Line(cx - 6.5f, cy, cx - 3.5f, cy, c, 1.3f);
        r.Line(cx + 3.5f, cy, cx + 6.5f, cy, c, 1.3f);
        r.Line(cx - 4.6f, cy - 4.6f, cx - 2.5f, cy - 2.5f, c, 1.3f);
        r.Line(cx + 2.5f, cy + 2.5f, cx + 4.6f, cy + 4.6f, c, 1.3f);
        break;
    default:
        r.Line(cx - 6, cy - 4, cx + 6, cy - 4, c, 1.1f); r.CircleFilled(cx + 2, cy - 4, 1.7f, c);
        r.Line(cx - 6, cy,     cx + 6, cy,     c, 1.1f); r.CircleFilled(cx - 3, cy,     1.7f, c);
        r.Line(cx - 6, cy + 4, cx + 6, cy + 4, c, 1.1f); r.CircleFilled(cx + 3, cy + 4, 1.7f, c);
        break;
    }
}

void Menu::sidebar(Render2D& r, float x, float y, float w, float h)
{
    const Theme& t = theme();
    r.GradientBox(x + 1, Y(y + 1), w - 1, h - 2, C(t.sideTop), C(t.sideBot), true, 5);
    r.BoxFilled(x + w, Y(y + 8), 1, h - 16, C(t.line)); // divider

    // wordmark
    r.TextLMid(x + 16, Y(y + 18 + 8), "jazzhook", C(t.text), 2);
    r.TextLMid(x + 16, Y(y + 42), "hvh legacy", C(t.textFaint), 1);
    r.BoxFilled(x + 16, Y(y + 54), w - 32, 1, C(t.lineSoft));

    // vertical nav
    float ty = y + 66, cellH = 38;
    for (int i = 0; i < kTabCount; ++i) {
        float cy = ty + i * cellH;
        bool active = (i == activeTab);
        bool over = hot(x + 10, cy + 3, w - 20, cellH - 6);
        if (over && input.clicked) { activeTab = i; active = true; }
        if (active) {
            r.RoundedBox(x + 10, Y(cy + 3), w - 20, cellH - 6, 4, C(t.accent.withA(22)));
            r.RoundedBox(x + 10, Y(cy + 7), 2.5f, cellH - 14, 1.5f, C(t.accent));
        } else if (over) {
            r.RoundedBox(x + 10, Y(cy + 3), w - 20, cellH - 6, 4, C(t.accent.withA(10)));
        }
        Color c = active ? t.accent : t.textDim;
        icon(r, i, x + 28, Y(cy + cellH / 2.f), c);
        r.TextLMid(x + 44, Y(cy + cellH / 2.f), kTabs[i], active ? C(t.text) : C(t.textDim), 1);
    }

    // bottom user strip
    r.BoxFilled(x + 16, Y(y + h - 40), w - 32, 1, C(t.lineSoft));
    r.TextLMid(x + 16, Y(y + h - 24), "skippy", C(t.textDim), 1);
    r.TextRMid(x + w - 16, Y(y + h - 24), "build 06.15", C(t.textFaint), 1);
}

void Menu::contentHead(Render2D& r, float x, float y, float w, const std::string& title)
{
    const Theme& t = theme();
    r.TextLMid(x + 16, Y(y + 24), title, C(t.text), 2);
    // weapon selector (rage only)
    if (activeTab == 0) {
        std::string val = "universal";
        float bw = 110, bx = x + w - 16 - bw, bh = 17, byc = Y(y + 24);
        r.RoundedBox(bx, byc - bh / 2, bw, bh, 3, C(t.field));
        r.RoundedBoxOutline(bx, byc - bh / 2, bw, bh, 3, 1.f, C(t.line));
        r.TextLMid(bx + 8, byc, val, C(t.textDim), 1);
        float chx = bx + bw - 9;
        r.Line(chx - 2.5f, byc - 1.5f, chx, byc + 1.5f, C(t.textDim), 1.1f);
        r.Line(chx, byc + 1.5f, chx + 2.5f, byc - 1.5f, C(t.textDim), 1.1f);
    }
    r.BoxFilled(x + 16, Y(y + 44), w - 32, 1, C(t.line));
}

// ----------------------------------------------------------------------------
// per-tab pages
// ----------------------------------------------------------------------------
void Menu::pageRage(Render2D& r, float lx, float rx, float colW, float by0)
{
    const Theme& t = theme();
    auto& c = cfg::g_cfg.rage;
    float lcy = container(r, lx, by0, colW, 134, "aimbot");
    lcy = rowCheck (r, lx, lcy, colW, "enabled", &c.enabled);
    lcy = rowSlider(r, lx, lcy, colW, "minimum damage", &c.min_damage, 0, 100);
    comboAY = lcy;
    lcy = rowCombo (r, lx, lcy, colW, "target", &c.target, cfg::kTarget, 4);
    lcy = rowCheck (r, lx, lcy, colW, "auto fire", &c.auto_fire, "mouse5");

    lcy = container(r, lx, by0 + 144, colW, 92, "accuracy");
    lcy = rowCheck (r, lx, lcy, colW, "auto stop", &c.auto_stop);
    lcy = rowCheck (r, lx, lcy, colW, "auto scope", &c.auto_scope);

    float rcy = container(r, rx, by0, colW, 134, "exploits");
    rcy = rowCheck (r, rx, rcy, colW, "double tap", &c.double_tap, "x");
    rcy = rowCheck (r, rx, rcy, colW, "hide shots", &c.hide_shots, "c");
    rcy = rowSlider(r, rx, rcy, colW, "shift amount", &c.shift_amount, 0, 16, "t");

    rcy = container(r, rx, by0 + 144, colW, 92, "overrides");
    rcy = rowSlider(r, rx, rcy, colW, "hitchance", &c.hitchance, 0, 100, "%");
    rcy = rowColor (r, rx, rcy, colW, "accent", t.accent);
}

void Menu::pageAntiAim(Render2D& r, float lx, float rx, float colW, float by0)
{
    auto& a = cfg::g_cfg.aa;
    float lcy = container(r, lx, by0, colW, 120, "anti-aimbot");
    lcy = rowCheck (r, lx, lcy, colW, "enabled", &a.enabled);
    lcy = rowCombo (r, lx, lcy, colW, "pitch", &a.pitch, cfg::kPitch, 3);
    lcy = rowCombo (r, lx, lcy, colW, "yaw base", &a.yaw_base, cfg::kYawBase, 4);
    lcy = rowCombo (r, lx, lcy, colW, "yaw jitter", &a.yaw_jitter, cfg::kYawJitter, 4);

    lcy = container(r, lx, by0 + 130, colW, 114, "desync");
    lcy = rowSlider(r, lx, lcy, colW, "amount", &a.desync, 0, 100);
    lcy = rowCombo (r, lx, lcy, colW, "inverter", &a.inverter, cfg::kInverter, 2);
    lcy = rowSlider(r, lx, lcy, colW, "body lean", &a.body_lean, 0, 100);

    float rcy = container(r, rx, by0, colW, 100, "on shot");
    rcy = rowCheck (r, rx, rcy, colW, "fake duck", &a.fake_duck, "f");
    rcy = rowCheck (r, rx, rcy, colW, "defensive aa", &a.defensive);
    rcy = rowCombo (r, rx, rcy, colW, "hide shots", &a.hide_shots, cfg::kHideShots, 3);

    rcy = container(r, rx, by0 + 110, colW, 134, "manual");
    rcy = rowCheck (r, rx, rcy, colW, "left", &a.manual_left, "left");
    rcy = rowCheck (r, rx, rcy, colW, "right", &a.manual_right, "right");
    rcy = rowCheck (r, rx, rcy, colW, "backward", &a.manual_back, "down");
    rcy = rowCheck (r, rx, rcy, colW, "freestand", &a.freestand, "alt");
}

void Menu::pageVisuals(Render2D& r, float lx, float rx, float colW, float by0)
{
    const Theme& t = theme();
    auto& v = cfg::g_cfg.visuals;
    float lcy = container(r, lx, by0, colW, 134, "players");
    lcy = rowCheck (r, lx, lcy, colW, "enabled", &v.players);
    lcy = rowCombo (r, lx, lcy, colW, "boxes", &v.boxes, cfg::kBoxes, 3);
    lcy = rowCheck (r, lx, lcy, colW, "skeleton", &v.skeleton);
    lcy = rowCheck (r, lx, lcy, colW, "health bar", &v.health);

    lcy = container(r, lx, by0 + 144, colW, 92, "effects");
    lcy = rowCombo (r, lx, lcy, colW, "chams", &v.chams, cfg::kChams, 4);
    lcy = rowCheck (r, lx, lcy, colW, "hit marker", &v.hit_marker);

    float rcy = container(r, rx, by0, colW, 92, "world");
    rcy = rowCheck (r, rx, rcy, colW, "remove scope", &v.remove_scope);
    rcy = rowColor (r, rx, rcy, colW, "esp color", t.accent);

    rcy = container(r, rx, by0 + 102, colW, 134, "interface");
    rcy = rowCheck (r, rx, rcy, colW, "watermark", &v.watermark);
    rcy = rowCheck (r, rx, rcy, colW, "keybinds", &v.keybinds);
    rcy = rowCheck (r, rx, rcy, colW, "spectators", &v.spectators);
    rcy = rowCheck (r, rx, rcy, colW, "hit log", &v.hit_log);
}

void Menu::pageMisc(Render2D& r, float lx, float rx, float colW, float by0)
{
    auto& m = cfg::g_cfg.misc;
    float lcy = container(r, lx, by0, colW, 114, "movement");
    lcy = rowCheck (r, lx, lcy, colW, "bunny hop", &m.bhop);
    lcy = rowCheck (r, lx, lcy, colW, "auto strafe", &m.auto_strafe);
    lcy = rowCheck (r, lx, lcy, colW, "fast stop", &m.fast_stop);

    lcy = container(r, lx, by0 + 124, colW, 92, "automation");
    lcy = rowCheck (r, lx, lcy, colW, "auto accept", &m.auto_accept);
    lcy = rowCheck (r, lx, lcy, colW, "auto pistol", &m.auto_pistol);

    float rcy = container(r, rx, by0, colW, 114, "misc");
    rcy = rowCheck (r, rx, rcy, colW, "anti aim-block", &m.anti_aimblock);
    rcy = rowCheck (r, rx, rcy, colW, "fake lag", &m.fake_lag, "shift");
    rcy = rowSlider(r, rx, rcy, colW, "fps boost", &m.fps_boost, 0, 3, "", cfg::kFps);

    rcy = container(r, rx, by0 + 124, colW, 92, "camera");
    rcy = rowCheck (r, rx, rcy, colW, "third person", &m.third_person, "n");
    rcy = rowCheck (r, rx, rcy, colW, "free cam", &m.free_cam, "m");
}

void Menu::pageConfigs(Render2D& r, float lx, float rx, float colW, float by0)
{
    static const char* names[] = {"hvh_main", "legit", "fakelag_heavy", "backup_0614", "default"};
    float lcy = container(r, lx, by0, colW, 150, "local configs");
    for (int i = 0; i < 5; ++i)
        lcy = rowListItem(r, lx, lcy, colW, names[i], i);

    float rcy = container(r, rx, by0, colW, 150, "actions");
    rcy = rowButton(r, rx, rcy, colW, "load", true);
    rcy = rowButton(r, rx, rcy, colW, "save", false);
    rcy = rowButton(r, rx, rcy, colW, "create new", false);
    rcy = rowButton(r, rx, rcy, colW, "delete", false);
}

float Menu::container(Render2D& r, float x, float y, float w, float h, const std::string& title)
{
    const Theme& t = theme();
    r.GradientBox(x, Y(y), w, h, C(t.container), C(t.container2), true, 4);
    r.RoundedBoxOutline(x, Y(y), w, h, 4, 1.f, C(t.line));
    r.RoundedBox(x + 12, Y(y + 11), 2, 9, 1, C(t.accent));
    r.TextLMid(x + 19, Y(y + 15), title, C(t.text), 1);
    r.BoxFilled(x + 12, Y(y + 27), w - 24, 1, C(t.lineSoft));
    return y + 34;
}

float Menu::rowCheck(Render2D& r, float x, float y, float w, const std::string& label,
                     bool* on, const std::string& bind)
{
    const Theme& t = theme();
    const float pad = 12, row = 20, bs = 12;
    if (hot(x + pad, y, w - pad * 2, row) && input.clicked) *on = !*on;
    float bx = x + pad, cyc = Y(y + row / 2.f);
    if (*on) {
        r.RoundedBox(bx, cyc - bs / 2, bs, bs, 3, C(t.accent));
        r.Line(bx + 2.6f, cyc, bx + 4.9f, cyc + 2.3f, C(Color(20, 22, 24)), 1.6f);
        r.Line(bx + 4.9f, cyc + 2.3f, bx + 9, cyc - 2.6f, C(Color(20, 22, 24)), 1.6f);
    } else {
        r.RoundedBox(bx, cyc - bs / 2, bs, bs, 3, C(t.field));
        r.RoundedBoxOutline(bx, cyc - bs / 2, bs, bs, 3, 1.f, C(t.line));
    }
    r.TextLMid(bx + bs + 8, cyc, label, C(*on ? t.text : t.textDim), 1);
    if (!bind.empty())
        r.TextRMid(x + w - pad, cyc, bind, C(t.accentDim), 1);
    return y + row;
}

float Menu::rowSlider(Render2D& r, float x, float y, float w, const std::string& label,
                      int* val, int lo, int hi, const std::string& suffix,
                      const char* const* labels)
{
    const Theme& t = theme();
    const float pad = 12, row = 30;
    float tx = x + pad, tw = w - pad * 2, ty = Y(y + 19), th = 4;
    // drag: while held with the cursor over the track band, set value from x
    if (live && input.down && input.mx >= tx - 4 && input.mx <= tx + tw + 4 &&
        input.my >= ty - 8 && input.my <= ty + 12) {
        float f = std::clamp((input.mx - tx) / tw, 0.f, 1.f);
        *val = lo + (int)std::lround(f * (hi - lo));
    }
    *val = std::clamp(*val, lo, hi);

    std::string valueText = labels ? labels[*val] : (std::to_string(*val) + suffix);
    float labCy = Y(y + 9);
    r.TextLMid(x + pad, labCy, label, C(t.text), 1);
    r.TextRMid(x + w - pad, labCy, valueText, C(t.textDim), 1);
    r.RoundedBox(tx, ty, tw, th, 2, C(t.field));
    float frac = (hi > lo) ? (float)(*val - lo) / (hi - lo) : 0.f;
    float fill = tw * std::clamp(frac, 0.f, 1.f);
    r.RoundedBox(tx, ty, fill, th, 2, C(t.accent));
    r.CircleFilled(tx + fill, ty + th / 2, 3.5f, C(t.accent2)); // knob
    return y + row;
}

float Menu::rowCombo(Render2D& r, float x, float y, float w, const std::string& label,
                     int* idx, const char* const* opts, int n)
{
    const Theme& t = theme();
    const float pad = 12, row = 20;
    float cyc = Y(y + row / 2.f);
    float bw = 96, bx = x + w - pad - bw, bh = 15;
    if (hot(bx, y + row / 2.f - bh / 2, bw, bh) && input.clicked)
        *idx = (*idx + 1) % n; // cycle to the next option
    *idx = (n > 0) ? ((*idx % n) + n) % n : 0;

    r.TextLMid(x + pad, cyc, label, C(t.text), 1);
    r.RoundedBox(bx, cyc - bh / 2, bw, bh, 3, C(t.field));
    r.RoundedBoxOutline(bx, cyc - bh / 2, bw, bh, 3, 1.f, C(t.line));
    r.TextLMid(bx + 8, cyc, opts[*idx], C(t.textDim), 1);
    float chx = bx + bw - 9;
    r.Line(chx - 2.5f, cyc - 1.5f, chx, cyc + 1.5f, C(t.textDim), 1.1f);
    r.Line(chx, cyc + 1.5f, chx + 2.5f, cyc - 1.5f, C(t.textDim), 1.1f);
    return y + row;
}

float Menu::rowColor(Render2D& r, float x, float y, float w, const std::string& label, Color c)
{
    const Theme& t = theme();
    const float pad = 12, row = 20;
    float cyc = Y(y + row / 2.f);
    r.TextLMid(x + pad, cyc, label, C(t.text), 1);
    float sw = 26, sh = 12, sx = x + w - pad - sw;
    r.RoundedBox(sx, cyc - sh / 2, sw, sh, 3, C(c));
    r.RoundedBoxOutline(sx, cyc - sh / 2, sw, sh, 3, 1.f, C(t.line));
    return y + row;
}

float Menu::rowButton(Render2D& r, float x, float y, float w, const std::string& label, bool primary)
{
    const Theme& t = theme();
    const float pad = 12, bh = 22;
    float bx = x + pad, bw = w - pad * 2, by = Y(y + 3);
    bool over = hot(bx, y + 3, bw, bh);
    Color base = primary ? t.accent : t.field;
    if (over) base = primary ? t.accent2 : t.container2;
    r.RoundedBox(bx, by, bw, bh, 3, C(base));
    if (!primary) r.RoundedBoxOutline(bx, by, bw, bh, 3, 1.f, C(t.line));
    r.TextMid(bx + bw / 2, by + bh / 2, label, C(primary ? Color(20, 22, 24) : t.text), 1);
    return y + bh + 8;
}

float Menu::rowListItem(Render2D& r, float x, float y, float w, const std::string& label, int idx)
{
    const Theme& t = theme();
    const float pad = 10, rh = 19;
    if (hot(x + pad, y + 1, w - pad * 2, rh - 2) && input.clicked) selectedConfig = idx;
    bool selected = (selectedConfig == idx);
    float cyc = Y(y + rh / 2.f);
    if (selected) {
        r.RoundedBox(x + pad, Y(y + 1), w - pad * 2, rh - 2, 3, C(t.accent.withA(28)));
        r.RoundedBox(x + pad, Y(y + 5), 2, rh - 10, 1, C(t.accent));
    }
    r.TextLMid(x + pad + 9, cyc, label, C(selected ? t.text : t.textDim), 1);
    return y + rh;
}
