#include "Menu.h"
#include "Theme.h"
#include "Anim.h"
#include <vector>
#include <string>

static const char* kTabs[] = {"RAGE", "ANTI-AIM", "VISUALS", "MISC", "CONFIG"};
static const int   kTabCount = 5;

void Menu::Draw(Render2D& r, float openT)
{
    const Theme& t = theme();
    openT = std::clamp(openT, 0.f, 1.f);
    float e = ease_out_cubic(openT);
    gAlpha = e;
    oy = (1.f - e) * 22.f; // slide up into place

    const float x = 150, y = 120, w = 660, h = 480;

    // drop shadow + panel
    r.Shadow(x, Y(y), w, h, 14, 26, C(t.shadow));
    r.RoundedBox(x, Y(y), w, h, 14, C(t.panel));
    r.RoundedBoxOutline(x, Y(y), w, h, 14, 1.f, C(t.outline));

    header(r, x, y, w);
    tabbar(r, x, y + 64, w);

    const float contentY = y + 108;
    const float colGap = 16;
    const float colW = (w - 16 * 2 - colGap) / 2.f;
    const float lx = x + 16;
    const float rx = lx + colW + colGap;

    // ---- left column ----
    float cy = groupboxTop(r, lx, contentY, colW, 196, "Aimbot");
    cy = rowToggle(r, lx, cy, colW, "Enabled", true);
    cy = rowSlider(r, lx, cy, colW, "Field of view", 0.45f, "4.5");
    cy = rowSlider(r, lx, cy, colW, "Hitchance", 0.72f, "72%");
    cy = rowCombo (r, lx, cy, colW, "Target", "Nearest");
    cy = rowKeybind(r, lx, cy, colW, "Fire key", "MOUSE5", "hold");

    cy = groupboxTop(r, lx, contentY + 212, colW, 132, "Accuracy");
    cy = rowToggle(r, lx, cy, colW, "Auto-stop", true);
    cy = rowToggle(r, lx, cy, colW, "Auto-scope", true);
    cy = rowSlider(r, lx, cy, colW, "Min damage", 0.35f, "35");

    // ---- right column ----
    float ry = groupboxTop(r, rx, contentY, colW, 196, "Anti-Aim");
    ry = rowToggle(r, rx, ry, colW, "Enabled", true);
    ry = rowCombo (r, rx, ry, colW, "Yaw", "Jitter");
    ry = rowSlider(r, rx, ry, colW, "Desync", 0.58f, "58", true);
    ry = rowToggle(r, rx, ry, colW, "Fake duck", false);
    ry = rowKeybind(r, rx, ry, colW, "Manual AA", "ARROWS", "hold");

    ry = groupboxTop(r, rx, contentY + 212, colW, 122, "Appearance");
    ry = rowColor(r, rx, ry, colW, "Accent color", t.accent);
    ry = rowCombo(r, rx, ry, colW, "ESP boxes", "Cornered");

    // footer hint
    r.Text(x + 18, Y(y + h - 22), "INSERT", C(t.accent), 1);
    r.Text(x + 18 + r.TextWidth("INSERT", 1) + 6, Y(y + h - 22), "toggle menu", C(t.textDim), 1);

    // expanded dropdown drawn last so it overlays neighbours (interaction demo)
    if (comboOpen) {
        const char* opts[] = {"Static", "Jitter", "Spin", "Random"};
        float dx = rx + 16 + 96, dw = colW - 16 * 2 - 96;
        float dy = Y(contentY + 28 + 30 + 30) + 22; // under the Yaw combo
        float ih = 22;
        float dh = ih * 4 + 8;
        r.Shadow(dx, dy, dw, dh, 6, 12, C(t.shadow));
        r.RoundedBox(dx, dy, dw, dh, 6, C(t.panel2));
        r.RoundedBoxOutline(dx, dy, dw, dh, 6, 1.f, C(t.outline));
        for (int i = 0; i < 4; ++i) {
            float oyy = dy + 4 + i * ih;
            bool sel = (i == 1);
            if (sel) r.RoundedBox(dx + 4, oyy, dw - 8, ih, 4, C(t.accent.withA(40)));
            r.Text(dx + 12, oyy + 7, opts[i], C(sel ? t.accent : t.text), 1);
        }
    }
}

void Menu::header(Render2D& r, float x, float y, float w)
{
    const Theme& t = theme();
    // header gradient band
    r.GradientBox(x + 1, Y(y + 1), w - 2, 62, C(t.panel2), C(t.panel), true, 14);
    // logo tile
    r.GradientBox(x + 16, Y(y + 16), 32, 32, C(t.accent), C(t.accent2), false, 8);
    r.TextCentered(x + 16 + 16, Y(y + 16 + 9), "J", C(Color(20, 20, 28)), 2);
    // title + subtitle
    r.Text(x + 60, Y(y + 16), "jazzhook", C(t.text), 3);
    r.Text(x + 60, Y(y + 40), "h v h   build 06.15", C(t.textDim), 1);
    // right-side status
    std::string who = "user: skippy";
    r.Text(x + w - 16 - r.TextWidth(who, 1), Y(y + 22), who, C(t.textDim), 1);
    r.CircleFilled(x + w - 16 - r.TextWidth(who, 1) - 10, Y(y + 22 + 3), 3, C(t.good));
    // divider
    r.BoxFilled(x + 1, Y(y + 63), w - 2, 1, C(t.outline));
}

void Menu::tabbar(Render2D& r, float x, float y, float w)
{
    const Theme& t = theme();
    float tabW = (w - 32) / (float)kTabCount;
    float bx = x + 16;
    // animated underline glides from previous to active tab
    float fromX = bx + (activeTab == 0 ? 0 : activeTab - 1) * tabW;
    float toX = bx + activeTab * tabW;
    float ux = fromX + (toX - fromX) * ease_in_out(underlineT);

    for (int i = 0; i < kTabCount; ++i) {
        bool active = (i == activeTab);
        Color col = active ? t.text : t.textDim;
        r.TextCentered(bx + i * tabW + tabW / 2, Y(y + 10), kTabs[i], C(col), 1);
    }
    // underline pill
    r.RoundedBox(ux + tabW * 0.18f, Y(y + 26), tabW * 0.64f, 3, 1.5f, C(t.accent));
    r.BoxFilled(x + 1, Y(y + 30), w - 2, 1, C(t.outline));
}

float Menu::groupboxTop(Render2D& r, float x, float y, float w, float h, const std::string& title)
{
    const Theme& t = theme();
    r.RoundedBox(x, Y(y), w, h, 8, C(t.groupbox));
    r.RoundedBoxOutline(x, Y(y), w, h, 8, 1.f, C(t.outline));
    // accent tick + title
    r.RoundedBox(x + 12, Y(y + 13), 3, 11, 1.5f, C(t.accent));
    r.Text(x + 22, Y(y + 13), title, C(t.text), 1);
    return y + 34;
}

float Menu::rowToggle(Render2D& r, float x, float y, float w, const std::string& label, bool on)
{
    const Theme& t = theme();
    const float pad = 14, row = 30;
    r.Text(x + pad, Y(y + 7), label, C(on ? t.text : t.textDim), 1);
    // switch
    float sw = 36, sh = 18, sx = x + w - pad - sw, sy = Y(y + 2);
    r.RoundedBox(sx, sy, sw, sh, 9, C(on ? t.accent : t.panel2));
    if (!on) r.RoundedBoxOutline(sx, sy, sw, sh, 9, 1.f, C(t.outline));
    float knob = on ? (sx + sw - sh / 2 - 2) : (sx + sh / 2 + 2);
    r.CircleFilled(knob, sy + sh / 2, sh / 2 - 3, C(on ? Color(245, 246, 255) : t.textDim));
    return y + row;
}

float Menu::rowSlider(Render2D& r, float x, float y, float w, const std::string& label,
                      float tval, const std::string& valueText, bool dragging)
{
    const Theme& t = theme();
    const float pad = 14, row = 36;
    r.Text(x + pad, Y(y + 4), label, C(t.text), 1);
    r.Text(x + w - pad - r.TextWidth(valueText, 1), Y(y + 4), valueText, C(t.textDim), 1);
    // track
    float tx = x + pad, tw = w - pad * 2, ty = Y(y + 22), th = 4;
    r.RoundedBox(tx, ty, tw, th, 2, C(t.panel2));
    float fill = tw * std::clamp(tval, 0.f, 1.f);
    r.GradientBox(tx, ty, fill, th, C(t.accent2), C(t.accent), false, 2);
    // knob (enlarged when dragging)
    float kr = dragging ? 7.f : 5.f;
    if (dragging) r.GlowCircle(tx + fill, ty + th / 2, 14, C(t.accent.withA(120)));
    r.CircleFilled(tx + fill, ty + th / 2, kr, C(Color(245, 246, 255)));
    if (dragging) {
        // value tooltip above the knob
        std::string tip = valueText;
        float bw = r.TextWidth(tip, 1) + 12, bh = 16;
        float bx = tx + fill - bw / 2, by = ty - bh - 8;
        r.RoundedBox(bx, by, bw, bh, 4, C(t.accent));
        r.TextCentered(tx + fill, by + 5, tip, C(Color(20, 20, 28)), 1);
    }
    return y + row;
}

float Menu::rowCombo(Render2D& r, float x, float y, float w, const std::string& label,
                     const std::string& value)
{
    const Theme& t = theme();
    const float pad = 14, row = 30;
    r.Text(x + pad, Y(y + 7), label, C(t.text), 1);
    float bw = 96, bx = x + w - pad - bw, by = Y(y + 2), bh = 20;
    r.RoundedBox(bx, by, bw, bh, 5, C(t.panel2));
    r.RoundedBoxOutline(bx, by, bw, bh, 5, 1.f, C(t.outline));
    r.Text(bx + 8, by + 7, value, C(t.text), 1);
    // chevron
    float chx = bx + bw - 12, chy = by + bh / 2;
    r.Line(chx - 3, chy - 2, chx, chy + 2, C(t.textDim), 1.4f);
    r.Line(chx, chy + 2, chx + 3, chy - 2, C(t.textDim), 1.4f);
    return y + row;
}

float Menu::rowKeybind(Render2D& r, float x, float y, float w, const std::string& label,
                       const std::string& key, const std::string& mode)
{
    const Theme& t = theme();
    const float pad = 14, row = 30;
    r.Text(x + pad, Y(y + 7), label, C(t.text), 1);
    // mode pill (right-most)
    float mw = r.TextWidth(mode, 1) + 12, mx = x + w - pad - mw, my = Y(y + 3), mh = 16;
    r.RoundedBox(mx, my, mw, mh, 8, C(t.accent.withA(45)));
    r.Text(mx + 6, my + 5, mode, C(t.accent), 1);
    // key box (left of pill)
    float kw = r.TextWidth(key, 1) + 14, kx = mx - 6 - kw, ky = Y(y + 2), kh = 18;
    r.RoundedBox(kx, ky, kw, kh, 4, C(t.panel2));
    r.RoundedBoxOutline(kx, ky, kw, kh, 4, 1.f, C(t.outline));
    r.Text(kx + 7, ky + 6, key, C(t.textDim), 1);
    return y + row;
}

float Menu::rowColor(Render2D& r, float x, float y, float w, const std::string& label, Color c)
{
    const Theme& t = theme();
    const float pad = 14, row = 30;
    r.Text(x + pad, Y(y + 7), label, C(t.text), 1);
    float sw = 28, sx = x + w - pad - sw, sy = Y(y + 3), sh = 16;
    r.RoundedBox(sx, sy, sw, sh, 4, C(c));
    r.RoundedBoxOutline(sx, sy, sw, sh, 4, 1.f, C(t.outline));
    return y + row;
}
