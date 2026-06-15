#pragma once
#include "Render2D.h"

// Menu — the custom animated menu, drawn entirely through the CRender-style
// Render2D API. Demo content is hardcoded (this is a visual preview); in the
// real mod the rows bind to config_t / m_KeyBinds exactly like ArcticTech.
class Menu {
public:
    int   activeTab    = 0;     // which top tab is selected
    float underlineT   = 0.f;   // 0..1 glide of the active-underline between tabs
    bool  comboOpen    = false; // show an expanded dropdown (interaction demo)

    // openT: 0 = fully closed, 1 = fully open (drives slide + fade-in).
    void Draw(Render2D& r, float openT);

private:
    float gAlpha = 1.f; // global opacity (for the open fade)
    float oy     = 0.f; // global y offset (for the open slide)

    // alpha-aware color + offset-aware coordinate helpers
    Color C(Color c) const { return c.withAf(gAlpha); }
    float Y(float y) const { return y + oy; }

    void header(Render2D& r, float x, float y, float w);
    void tabbar(Render2D& r, float x, float y, float w);

    // widget rows; each returns the y for the next row
    float groupboxTop(Render2D& r, float x, float y, float w, float h, const std::string& title);
    float rowToggle(Render2D& r, float x, float y, float w, const std::string& label, bool on);
    float rowSlider(Render2D& r, float x, float y, float w, const std::string& label,
                    float t, const std::string& valueText, bool dragging = false);
    float rowCombo(Render2D& r, float x, float y, float w, const std::string& label,
                   const std::string& value);
    float rowKeybind(Render2D& r, float x, float y, float w, const std::string& label,
                     const std::string& key, const std::string& mode);
    float rowColor(Render2D& r, float x, float y, float w, const std::string& label, Color c);
};
