#pragma once
#include "Render2D.h"
#include <string>

// Menu — premium legacy-HvH menu: left sidebar (logo + vertical nav), content
// pane on the right. Restrained teal accent, cream text, soft depth. Drawn
// through the CRender-style Render2D API; demo content hardcoded.
class Menu {
public:
    int  activeTab = 0;
    bool comboOpen = false;

    // openT: 0 closed, 1 open (fade + slide-in).
    void Draw(Render2D& r, float openT);

private:
    float gAlpha = 1.f;
    float oy = 0.f;
    Color C(Color c) const { return c.withAf(gAlpha); }
    float Y(float y) const { return y + oy; }

    void sidebar(Render2D& r, float x, float y, float w, float h);
    void icon(Render2D& r, int tab, float cx, float cy, Color c);
    void contentHead(Render2D& r, float x, float y, float w, const std::string& title);

    float container(Render2D& r, float x, float y, float w, float h, const std::string& title);
    float rowCheck(Render2D& r, float x, float y, float w, const std::string& label,
                   bool on, const std::string& bind = "");
    float rowSlider(Render2D& r, float x, float y, float w, const std::string& label,
                    float t, const std::string& valueText);
    float rowCombo(Render2D& r, float x, float y, float w, const std::string& label,
                   const std::string& value);
    float rowColor(Render2D& r, float x, float y, float w, const std::string& label, Color c);
};
