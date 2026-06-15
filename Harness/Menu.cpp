#include "Menu.h"
#include "Theme.h"
#include "Anim.h"
#include <vector>
#include <string>

// gamesense-inspired layout: wordmark + horizontal icon tab bar on top, a text
// sub-tab row, then dense multi-column containers. Verdana-like AA font, single
// yellow accent, flat 1px framing. Demo content is hardcoded.

static const char* kTabs[] = {"rage", "anti-aim", "visuals", "misc", "configs"};
static const int kTabCount = 5;
static const char* kSub[] = {"general", "weapon", "accuracy"};
static const int kSubCount = 3;

void Menu::Draw(Render2D& r, float openT)
{
    const Theme& t = theme();
    openT = std::clamp(openT, 0.f, 1.f);
    float e = ease_out_cubic(openT);
    gAlpha = e;
    oy = (1.f - e) * 16.f;

    const float x = 200, y = 150, w = 600, h = 346;
    const float headH = 34, tabH = 30, subH = 22;

    r.Shadow(x, Y(y), w, h, 4, 22, C(t.shadow));
    r.RoundedBox(x, Y(y), w, h, 4, C(t.panel));

    cap(r, x, y, w);
    header(r, x, y, w);
    topTabs(r, x, y + headH, w);
    subtabs(r, x, y + headH + tabH, w);
    r.RoundedBoxOutline(x, Y(y), w, h, 4, 1.f, C(t.outlineLt));

    const float pad = 10, colGap = 10;
    const float contentX = x + pad, contentW = w - pad * 2;
    const float colW = (contentW - colGap) / 2.f;
    const float lx = contentX, rx = contentX + colW + colGap;
    const float cy0 = y + headH + tabH + subH + 8;

    // ---- left column ----
    float lcy = section(r, lx, cy0, colW, 143, "Aimbot");
    lcy = rowCheck (r, lx, lcy, colW, "Enabled", true);
    lcy = rowSlider(r, lx, lcy, colW, "Minimum damage", 0.35f, "35");
    lcy = rowSlider(r, lx, lcy, colW, "Hitchance", 0.72f, "72%");
    float targetY = lcy;
    lcy = rowCombo (r, lx, lcy, colW, "Target", "nearest");
    lcy = rowCheck (r, lx, lcy, colW, "Auto fire", true, "M5");

    lcy = section(r, lx, cy0 + 153, colW, 70, "Accuracy");
    lcy = rowCheck (r, lx, lcy, colW, "Auto stop", true);
    lcy = rowCheck (r, lx, lcy, colW, "Auto scope", true);

    // ---- right column ----
    float rcy = section(r, rx, cy0, colW, 100, "Targeting");
    rcy = rowCombo (r, rx, rcy, colW, "Hitboxes", "head +3");
    rcy = rowCheck (r, rx, rcy, colW, "Prefer body aim", false);
    rcy = rowSlider(r, rx, rcy, colW, "Max misses", 0.40f, "2");

    rcy = section(r, rx, cy0 + 110, colW, 88, "Exploits");
    rcy = rowCheck (r, rx, rcy, colW, "Double tap", true, "X");
    rcy = rowCheck (r, rx, rcy, colW, "Hide shots", true, "C");
    rcy = rowColor (r, rx, rcy, colW, "Accent color", t.accent);

    // footer
    r.BoxFilled(x + 1, Y(y + h - 18), w - 2, 1, C(t.outline));
    r.Text(contentX, Y(y + h - 15), "insert", C(t.accent), 1);
    r.Text(contentX + r.TextWidth("insert", 1) + 5, Y(y + h - 15),
           "toggle    right-click any control to bind", C(t.textDim), 1);

    // expanded dropdown (interaction demo) — under the Target combo
    if (comboOpen) {
        const char* opts[] = {"nearest", "lowest hp", "crosshair", "high damage"};
        float bw = 104, dx = lx + colW - 9 - bw, dy = Y(targetY) + 18, ih = 15;
        float dh = ih * 4 + 6;
        r.Shadow(dx, dy, bw, dh, 3, 8, C(t.shadow));
        r.RoundedBox(dx, dy, bw, dh, 3, C(t.panel2));
        r.RoundedBoxOutline(dx, dy, bw, dh, 3, 1.f, C(t.outlineLt));
        for (int i = 0; i < 4; ++i) {
            float oyy = dy + 3 + i * ih;
            bool sel = (i == 0);
            if (sel) r.RoundedBox(dx + 2, oyy, bw - 4, ih, 2, C(t.accent.withA(40)));
            r.Text(dx + 7, oyy + 3, opts[i], C(sel ? t.accent : t.textDim), 1);
        }
    }
}

void Menu::cap(Render2D& r, float x, float y, float w)
{
    const Theme& t = theme();
    r.RoundedBox(x, Y(y), w, 2, 1, C(t.accent));
}

void Menu::header(Render2D& r, float x, float y, float w)
{
    const Theme& t = theme();
    r.RoundedBox(x + 10, Y(y + 8), 18, 18, 3, C(t.accent));
    r.TextCentered(x + 10 + 9, Y(y + 8 + 3), "j", C(Color(18, 18, 20)), 2);
    r.Text(x + 34, Y(y + 9), "jazzhook", C(t.text), 2);
    // right status
    std::string who = "skippy";
    float wx = x + w - 12 - r.TextWidth(who, 1);
    r.Text(wx, Y(y + 12), who, C(t.textDim), 1);
    r.CircleFilled(wx - 8, Y(y + 12 + 4), 2.5f, C(t.good));
    r.BoxFilled(x + 1, Y(y + 33), w - 2, 1, C(t.outline));
}

void Menu::icon(Render2D& r, int tab, float cx, float cy, Color c)
{
    switch (tab) {
    case 0: // rage — crosshair
        r.Circle(cx, cy, 4.5f, 1.2f, c);
        r.Line(cx - 7, cy, cx - 3.5f, cy, c, 1.2f);
        r.Line(cx + 3.5f, cy, cx + 7, cy, c, 1.2f);
        r.Line(cx, cy - 7, cx, cy - 3.5f, c, 1.2f);
        r.Line(cx, cy + 3.5f, cx, cy + 7, c, 1.2f);
        break;
    case 1: // anti-aim — opposing arrows
        r.Triangle(cx - 7, cy, cx - 2, cy - 4, cx - 2, cy + 4, c);
        r.Triangle(cx + 7, cy, cx + 2, cy - 4, cx + 2, cy + 4, c);
        r.Line(cx - 2, cy, cx + 2, cy, c, 1.2f);
        break;
    case 2: // visuals — eye
        r.Circle(cx, cy, 5.5f, 1.2f, c);
        r.CircleFilled(cx, cy, 2.0f, c);
        break;
    case 3: // misc — gear
        r.Circle(cx, cy, 3.5f, 1.4f, c);
        r.Line(cx, cy - 6.5f, cx, cy - 3.5f, c, 1.4f);
        r.Line(cx, cy + 3.5f, cx, cy + 6.5f, c, 1.4f);
        r.Line(cx - 6.5f, cy, cx - 3.5f, cy, c, 1.4f);
        r.Line(cx + 3.5f, cy, cx + 6.5f, cy, c, 1.4f);
        r.Line(cx - 4.6f, cy - 4.6f, cx - 2.5f, cy - 2.5f, c, 1.4f);
        r.Line(cx + 2.5f, cy + 2.5f, cx + 4.6f, cy + 4.6f, c, 1.4f);
        break;
    default: // configs — sliders
        r.Line(cx - 6, cy - 4, cx + 6, cy - 4, c, 1.2f); r.CircleFilled(cx + 2, cy - 4, 1.8f, c);
        r.Line(cx - 6, cy,     cx + 6, cy,     c, 1.2f); r.CircleFilled(cx - 3, cy,     1.8f, c);
        r.Line(cx - 6, cy + 4, cx + 6, cy + 4, c, 1.2f); r.CircleFilled(cx + 3, cy + 4, 1.8f, c);
        break;
    }
}

void Menu::topTabs(Render2D& r, float x, float y, float w)
{
    const Theme& t = theme();
    float h = 30;
    float cellW = w / (float)kTabCount;
    for (int i = 0; i < kTabCount; ++i) {
        float cellX = x + i * cellW;
        bool active = (i == activeTab);
        Color c = active ? t.accent : t.textDim;
        // icon + label, centered as a group
        float lblW = r.TextWidth(kTabs[i], 1);
        float groupW = 16 + 5 + lblW;
        float gx = cellX + (cellW - groupW) / 2.f;
        icon(r, i, gx + 8, Y(y + h / 2.f - 1), c);
        r.Text(gx + 16 + 5, Y(y + h / 2.f - 6), kTabs[i], c, 1);
        if (active) r.BoxFilled(cellX + cellW * 0.16f, Y(y + h - 2), cellW * 0.68f, 2, C(t.accent));
    }
    r.BoxFilled(x + 1, Y(y + h), w - 2, 1, C(t.outline));
}

void Menu::subtabs(Render2D& r, float x, float y, float w)
{
    const Theme& t = theme();
    float h = 22, padL = 12, gap = 16;
    r.BoxFilled(x + 1, Y(y), w - 2, h, C(t.rail));

    std::vector<float> xs, ws;
    float px = x + padL;
    for (int i = 0; i < kSubCount; ++i) {
        float tw = r.TextWidth(kSub[i], 1);
        xs.push_back(px); ws.push_back(tw);
        px += tw + gap;
    }
    for (int i = 0; i < kSubCount; ++i) {
        bool active = (i == subTab);
        r.Text(xs[i], Y(y + 6), kSub[i], active ? C(t.text) : C(t.textDim), 1);
    }
    int from = subTab == 0 ? 0 : subTab - 1;
    float ux = xs[from] + (xs[subTab] - xs[from]) * ease_in_out(subUlineT);
    float uw = ws[from] + (ws[subTab] - ws[from]) * ease_in_out(subUlineT);
    r.BoxFilled(ux, Y(y + h - 2), uw, 2, C(t.accent));
    r.BoxFilled(x + 1, Y(y + h), w - 2, 1, C(t.outline));
}

float Menu::section(Render2D& r, float x, float y, float w, float h, const std::string& title)
{
    const Theme& t = theme();
    r.RoundedBox(x, Y(y), w, h, 3, C(t.section));
    r.RoundedBoxOutline(x, Y(y), w, h, 3, 1.f, C(t.outline));
    r.BoxFilled(x + 9, Y(y + 8), 2, 9, C(t.accent));
    r.Text(x + 15, Y(y + 6), title, C(t.text), 1);
    r.BoxFilled(x + 9, Y(y + 22), w - 18, 1, C(t.outline));
    return y + 27;
}

float Menu::rowCheck(Render2D& r, float x, float y, float w, const std::string& label,
                     bool on, const std::string& bind)
{
    const Theme& t = theme();
    const float pad = 10, row = 18, bs = 10;
    float bx = x + pad, by = Y(y + 2);
    if (on) {
        r.RoundedBox(bx, by, bs, bs, 2, C(t.accent));
        r.Line(bx + 2.2f, by + 5, bx + 4.2f, by + 7, C(Color(18, 18, 20)), 1.5f);
        r.Line(bx + 4.2f, by + 7, bx + 7.6f, by + 2.8f, C(Color(18, 18, 20)), 1.5f);
    } else {
        r.RoundedBox(bx, by, bs, bs, 2, C(t.panel2));
        r.RoundedBoxOutline(bx, by, bs, bs, 2, 1.f, C(t.outlineLt));
    }
    r.Text(bx + bs + 6, Y(y + 3), label, C(on ? t.text : t.textDim), 1);
    if (!bind.empty()) {
        std::string b = "[" + bind + "]";
        r.Text(x + w - pad - r.TextWidth(b, 1), Y(y + 3), b, C(t.accent), 1);
    }
    return y + row;
}

float Menu::rowSlider(Render2D& r, float x, float y, float w, const std::string& label,
                      float tv, const std::string& valueText)
{
    const Theme& t = theme();
    const float pad = 10, row = 28;
    r.Text(x + pad, Y(y + 2), label, C(t.text), 1);
    r.Text(x + w - pad - r.TextWidth(valueText, 1), Y(y + 2), valueText, C(t.textDim), 1);
    float tx = x + pad, tw = w - pad * 2, ty = Y(y + 16), th = 6;
    r.RoundedBox(tx, ty, tw, th, 2, C(t.panel2));
    r.RoundedBoxOutline(tx, ty, tw, th, 2, 1.f, C(t.outline));
    float fill = tw * std::clamp(tv, 0.f, 1.f);
    r.RoundedBox(tx + 1, ty + 1, std::max(0.f, fill - 2), th - 2, 1.5f, C(t.accent));
    return y + row;
}

float Menu::rowCombo(Render2D& r, float x, float y, float w, const std::string& label,
                     const std::string& value)
{
    const Theme& t = theme();
    const float pad = 10, row = 18;
    r.Text(x + pad, Y(y + 3), label, C(t.text), 1);
    float bw = 104, bx = x + w - pad - bw, by = Y(y + 1), bh = 14;
    r.RoundedBox(bx, by, bw, bh, 2, C(t.panel2));
    r.RoundedBoxOutline(bx, by, bw, bh, 2, 1.f, C(t.outline));
    r.Text(bx + 6, by + 3, value, C(t.textDim), 1);
    float chx = bx + bw - 9, chy = by + bh / 2;
    r.Line(chx - 2.5f, chy - 1.5f, chx, chy + 1.5f, C(t.textDim), 1.2f);
    r.Line(chx, chy + 1.5f, chx + 2.5f, chy - 1.5f, C(t.textDim), 1.2f);
    return y + row;
}

float Menu::rowColor(Render2D& r, float x, float y, float w, const std::string& label, Color c)
{
    const Theme& t = theme();
    const float pad = 10, row = 18;
    r.Text(x + pad, Y(y + 3), label, C(t.text), 1);
    float sw = 24, sh = 11, sx = x + w - pad - sw, sy = Y(y + 2);
    r.RoundedBox(sx, sy, sw, sh, 2, C(c));
    r.RoundedBoxOutline(sx, sy, sw, sh, 2, 1.f, C(t.outlineLt));
    return y + row;
}
