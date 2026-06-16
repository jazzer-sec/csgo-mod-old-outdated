#pragma once
#include "Render2D.h"
#include <string>

// Menu — premium legacy-HvH menu: left sidebar (logo + vertical nav), content
// pane on the right. Restrained teal accent, cream text, soft depth. Drawn
// through the CRender-style Render2D API. The widgets are immediate-mode: when
// `input` carries mouse state they read/write cfg::g_cfg directly, so the menu
// is interactive in the standalone app and inert (static preview) without input.
class Menu {
public:
    struct Input {
        float mx = 0, my = 0;  // cursor position
        bool  down = false;    // left button held (sliders)
        bool  clicked = false; // left button pressed this frame (toggles/tabs)
    };

    int   activeTab = 0;
    bool  comboOpen = false;   // legacy PNG dropdown demo only
    int   selectedConfig = 0;
    Input input;               // set each frame by the app; default = no interaction

    // openT: 0 closed, 1 open (fade + slide-in). Interaction only when settled.
    void Draw(Render2D& r, float openT);

private:
    float gAlpha = 1.f;
    float oy = 0.f;
    bool  live = false;        // interaction enabled this frame (menu settled open)
    Color C(Color c) const { return c.withAf(gAlpha); }
    float Y(float y) const { return y + oy; }

    float comboAY = 0.f; // y of the rage "target" combo, for the demo dropdown

    // hit-test a rect in screen space (accounts for the open-slide offset)
    bool hot(float x, float y, float w, float h) const {
        return live && input.mx >= x && input.mx <= x + w &&
               input.my >= Y(y) && input.my <= Y(y) + h;
    }

    void sidebar(Render2D& r, float x, float y, float w, float h);
    void icon(Render2D& r, int tab, float cx, float cy, Color c);
    void contentHead(Render2D& r, float x, float y, float w, const std::string& title);

    // per-tab content pages
    void pageRage(Render2D& r, float lx, float rx, float colW, float by0);
    void pageAntiAim(Render2D& r, float lx, float rx, float colW, float by0);
    void pageVisuals(Render2D& r, float lx, float rx, float colW, float by0);
    void pageMisc(Render2D& r, float lx, float rx, float colW, float by0);
    void pageConfigs(Render2D& r, float lx, float rx, float colW, float by0);

    float container(Render2D& r, float x, float y, float w, float h, const std::string& title);
    // interactive widgets — bound to a cfg field by pointer
    float rowCheck(Render2D& r, float x, float y, float w, const std::string& label,
                   bool* on, const std::string& bind = "");
    float rowSlider(Render2D& r, float x, float y, float w, const std::string& label,
                    int* val, int lo, int hi, const std::string& suffix = "",
                    const char* const* labels = nullptr);
    float rowCombo(Render2D& r, float x, float y, float w, const std::string& label,
                   int* idx, const char* const* opts, int n);
    float rowColor(Render2D& r, float x, float y, float w, const std::string& label, Color c);
    float rowButton(Render2D& r, float x, float y, float w, const std::string& label, bool primary);
    float rowListItem(Render2D& r, float x, float y, float w, const std::string& label, int idx);
};
