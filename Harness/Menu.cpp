#include "Menu.h"
#include "Theme.h"
#include "Anim.h"
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

    // dropdown (interaction demo) under the rage "target" combo
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
        if (active) {
            r.RoundedBox(x + 10, Y(cy + 3), w - 20, cellH - 6, 4, C(t.accent.withA(22)));
            r.RoundedBox(x + 10, Y(cy + 7), 2.5f, cellH - 14, 1.5f, C(t.accent));
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
    float lcy = container(r, lx, by0, colW, 134, "aimbot");
    lcy = rowCheck (r, lx, lcy, colW, "enabled", true);
    lcy = rowSlider(r, lx, lcy, colW, "minimum damage", 0.35f, "35");
    comboAY = lcy;
    lcy = rowCombo (r, lx, lcy, colW, "target", "nearest");
    lcy = rowCheck (r, lx, lcy, colW, "auto fire", true, "mouse5");

    lcy = container(r, lx, by0 + 144, colW, 92, "accuracy");
    lcy = rowCheck (r, lx, lcy, colW, "auto stop", true);
    lcy = rowCheck (r, lx, lcy, colW, "auto scope", true);

    float rcy = container(r, rx, by0, colW, 134, "exploits");
    rcy = rowCheck (r, rx, rcy, colW, "double tap", true, "x");
    rcy = rowCheck (r, rx, rcy, colW, "hide shots", true, "c");
    rcy = rowSlider(r, rx, rcy, colW, "shift amount", 0.85f, "14t");

    rcy = container(r, rx, by0 + 144, colW, 92, "overrides");
    rcy = rowSlider(r, rx, rcy, colW, "hitchance", 0.72f, "72%");
    rcy = rowColor (r, rx, rcy, colW, "accent", t.accent);
}

void Menu::pageAntiAim(Render2D& r, float lx, float rx, float colW, float by0)
{
    float lcy = container(r, lx, by0, colW, 120, "anti-aimbot");
    lcy = rowCheck (r, lx, lcy, colW, "enabled", true);
    lcy = rowCombo (r, lx, lcy, colW, "pitch", "down");
    lcy = rowCombo (r, lx, lcy, colW, "yaw base", "backward");
    lcy = rowCombo (r, lx, lcy, colW, "yaw jitter", "offset");

    lcy = container(r, lx, by0 + 130, colW, 114, "desync");
    lcy = rowSlider(r, lx, lcy, colW, "amount", 0.62f, "62");
    lcy = rowCombo (r, lx, lcy, colW, "inverter", "switch");
    lcy = rowSlider(r, lx, lcy, colW, "body lean", 0.40f, "40");

    float rcy = container(r, rx, by0, colW, 100, "on shot");
    rcy = rowCheck (r, rx, rcy, colW, "fake duck", true, "f");
    rcy = rowCheck (r, rx, rcy, colW, "defensive aa", false);
    rcy = rowCombo (r, rx, rcy, colW, "hide shots", "on key");

    rcy = container(r, rx, by0 + 110, colW, 134, "manual");
    rcy = rowCheck (r, rx, rcy, colW, "left", true, "left");
    rcy = rowCheck (r, rx, rcy, colW, "right", true, "right");
    rcy = rowCheck (r, rx, rcy, colW, "backward", true, "down");
    rcy = rowCheck (r, rx, rcy, colW, "freestand", false, "alt");
}

void Menu::pageVisuals(Render2D& r, float lx, float rx, float colW, float by0)
{
    const Theme& t = theme();
    float lcy = container(r, lx, by0, colW, 134, "players");
    lcy = rowCheck (r, lx, lcy, colW, "enabled", true);
    lcy = rowCombo (r, lx, lcy, colW, "boxes", "cornered");
    lcy = rowCheck (r, lx, lcy, colW, "skeleton", true);
    lcy = rowCheck (r, lx, lcy, colW, "health bar", true);

    lcy = container(r, lx, by0 + 144, colW, 92, "effects");
    lcy = rowCombo (r, lx, lcy, colW, "chams", "flat");
    lcy = rowCheck (r, lx, lcy, colW, "hit marker", true);

    float rcy = container(r, rx, by0, colW, 92, "world");
    rcy = rowCheck (r, rx, rcy, colW, "remove scope", true);
    rcy = rowColor (r, rx, rcy, colW, "esp color", t.accent);

    rcy = container(r, rx, by0 + 102, colW, 134, "interface");
    rcy = rowCheck (r, rx, rcy, colW, "watermark", true);
    rcy = rowCheck (r, rx, rcy, colW, "keybinds", true);
    rcy = rowCheck (r, rx, rcy, colW, "spectators", true);
    rcy = rowCheck (r, rx, rcy, colW, "hit log", true);
}

void Menu::pageMisc(Render2D& r, float lx, float rx, float colW, float by0)
{
    float lcy = container(r, lx, by0, colW, 114, "movement");
    lcy = rowCheck (r, lx, lcy, colW, "bunny hop", true);
    lcy = rowCheck (r, lx, lcy, colW, "auto strafe", true);
    lcy = rowCheck (r, lx, lcy, colW, "fast stop", true);

    lcy = container(r, lx, by0 + 124, colW, 92, "automation");
    lcy = rowCheck (r, lx, lcy, colW, "auto accept", true);
    lcy = rowCheck (r, lx, lcy, colW, "auto pistol", true);

    float rcy = container(r, rx, by0, colW, 114, "misc");
    rcy = rowCheck (r, rx, rcy, colW, "anti aim-block", true);
    rcy = rowCheck (r, rx, rcy, colW, "fake lag", true, "shift");
    rcy = rowSlider(r, rx, rcy, colW, "fps boost", 0.50f, "med");

    rcy = container(r, rx, by0 + 124, colW, 92, "camera");
    rcy = rowCheck (r, rx, rcy, colW, "third person", false, "n");
    rcy = rowCheck (r, rx, rcy, colW, "free cam", false, "m");
}

void Menu::pageConfigs(Render2D& r, float lx, float rx, float colW, float by0)
{
    float lcy = container(r, lx, by0, colW, 150, "local configs");
    lcy = rowListItem(r, lx, lcy, colW, "hvh_main", true);
    lcy = rowListItem(r, lx, lcy, colW, "legit", false);
    lcy = rowListItem(r, lx, lcy, colW, "fakelag_heavy", false);
    lcy = rowListItem(r, lx, lcy, colW, "backup_0614", false);
    lcy = rowListItem(r, lx, lcy, colW, "default", false);

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
                     bool on, const std::string& bind)
{
    const Theme& t = theme();
    const float pad = 12, row = 20, bs = 12;
    float bx = x + pad, cyc = Y(y + row / 2.f);
    if (on) {
        r.RoundedBox(bx, cyc - bs / 2, bs, bs, 3, C(t.accent));
        r.Line(bx + 2.6f, cyc, bx + 4.9f, cyc + 2.3f, C(Color(20, 22, 24)), 1.6f);
        r.Line(bx + 4.9f, cyc + 2.3f, bx + 9, cyc - 2.6f, C(Color(20, 22, 24)), 1.6f);
    } else {
        r.RoundedBox(bx, cyc - bs / 2, bs, bs, 3, C(t.field));
        r.RoundedBoxOutline(bx, cyc - bs / 2, bs, bs, 3, 1.f, C(t.line));
    }
    r.TextLMid(bx + bs + 8, cyc, label, C(on ? t.text : t.textDim), 1);
    if (!bind.empty())
        r.TextRMid(x + w - pad, cyc, bind, C(t.accentDim), 1);
    return y + row;
}

float Menu::rowSlider(Render2D& r, float x, float y, float w, const std::string& label,
                      float tv, const std::string& valueText)
{
    const Theme& t = theme();
    const float pad = 12, row = 30;
    float labCy = Y(y + 9);
    r.TextLMid(x + pad, labCy, label, C(t.text), 1);
    r.TextRMid(x + w - pad, labCy, valueText, C(t.textDim), 1);
    float tx = x + pad, tw = w - pad * 2, ty = Y(y + 19), th = 4;
    r.RoundedBox(tx, ty, tw, th, 2, C(t.field));
    float fill = tw * std::clamp(tv, 0.f, 1.f);
    r.RoundedBox(tx, ty, fill, th, 2, C(t.accent));
    r.CircleFilled(tx + fill, ty + th / 2, 3.5f, C(t.accent2)); // knob
    return y + row;
}

float Menu::rowCombo(Render2D& r, float x, float y, float w, const std::string& label,
                     const std::string& value)
{
    const Theme& t = theme();
    const float pad = 12, row = 20;
    float cyc = Y(y + row / 2.f);
    r.TextLMid(x + pad, cyc, label, C(t.text), 1);
    float bw = 96, bx = x + w - pad - bw, bh = 15;
    r.RoundedBox(bx, cyc - bh / 2, bw, bh, 3, C(t.field));
    r.RoundedBoxOutline(bx, cyc - bh / 2, bw, bh, 3, 1.f, C(t.line));
    r.TextLMid(bx + 8, cyc, value, C(t.textDim), 1);
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
    r.RoundedBox(bx, by, bw, bh, 3, C(primary ? t.accent : t.field));
    if (!primary) r.RoundedBoxOutline(bx, by, bw, bh, 3, 1.f, C(t.line));
    r.TextMid(bx + bw / 2, by + bh / 2, label, C(primary ? Color(20, 22, 24) : t.text), 1);
    return y + bh + 8;
}

float Menu::rowListItem(Render2D& r, float x, float y, float w, const std::string& label, bool selected)
{
    const Theme& t = theme();
    const float pad = 10, rh = 19;
    float cyc = Y(y + rh / 2.f);
    if (selected) {
        r.RoundedBox(x + pad, Y(y + 1), w - pad * 2, rh - 2, 3, C(t.accent.withA(28)));
        r.RoundedBox(x + pad, Y(y + 5), 2, rh - 10, 1, C(t.accent));
    }
    r.TextLMid(x + pad + 9, cyc, label, C(selected ? t.text : t.textDim), 1);
    return y + rh;
}
