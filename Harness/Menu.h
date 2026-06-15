#pragma once
#include "Render2D.h"
#include <string>

// Menu — legacy HvH menu (icon tab-rail + sub-tab strip + dense sections),
// drawn through the CRender-style Render2D API. Demo content is hardcoded.
class Menu {
public:
    int   activeTab  = 0;   // vertical rail selection
    int   subTab     = 0;   // horizontal sub-tab selection
    float subUlineT  = 1.f; // 0..1 glide of the sub-tab underline
    bool  comboOpen  = false;

    // openT: 0 closed, 1 open (drives fade + slide-in).
    void Draw(Render2D& r, float openT);

private:
    float gAlpha = 1.f;
    float oy = 0.f;
    Color C(Color c) const { return c.withAf(gAlpha); }
    float Y(float y) const { return y + oy; }

    void cap(Render2D& r, float x, float y, float w);
    void header(Render2D& r, float x, float y, float w);
    void topTabs(Render2D& r, float x, float y, float w);
    void subtabs(Render2D& r, float x, float y, float w);
    void icon(Render2D& r, int tab, float cx, float cy, Color c);

    // sections + dense rows; each row returns next y
    float section(Render2D& r, float x, float y, float w, float h, const std::string& title);
    float rowCheck(Render2D& r, float x, float y, float w, const std::string& label,
                   bool on, const std::string& bind = "");
    float rowSlider(Render2D& r, float x, float y, float w, const std::string& label,
                    float t, const std::string& valueText);
    float rowCombo(Render2D& r, float x, float y, float w, const std::string& label,
                   const std::string& value);
    float rowColor(Render2D& r, float x, float y, float w, const std::string& label, Color c);
};
