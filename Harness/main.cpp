// Preview driver: renders the custom UI + HUD to PNGs with mocked state.
// No game required — pure software rasterizer.
//
//   g++ -std=c++17 -O2 Harness/*.cpp -o /tmp/uiharness && /tmp/uiharness
//
// Output: Harness/out/*.png

#include "Render2D.h"
#include "Theme.h"
#include "Anim.h"
#include "Hud.h"
#include "Menu.h"
#include <cstdio>
#include <string>

static const int W = 1280, H = 720;

static void scene(Render2D& r) {
    Hud::DrawBackdrop(r);
    Hud::DrawWorld(r);
    Hud::DrawOverlays(r);
}

static void dim(Render2D& r, float a) {
    if (a <= 0) return;
    r.BoxFilled(0, 0, (float)r.width(), (float)r.height(), Color(0, 0, 0, (int)(120 * a)));
}

int main() {
    // 1) full HvH scene, menu closed
    {
        Render2D r(W, H);
        scene(r);
        r.savePNG("Harness/out/01_scene.png");
    }

    // 2) menu open, settled (rage tab)
    {
        Render2D r(W, H);
        scene(r);
        dim(r, 1.f);
        Menu m; m.activeTab = 0;
        m.Draw(r, 1.f);
        r.savePNG("Harness/out/02_menu.png");
    }

    // 3) interaction: target dropdown expanded
    {
        Render2D r(W, H);
        scene(r);
        dim(r, 1.f);
        Menu m; m.activeTab = 0; m.comboOpen = true;
        m.Draw(r, 1.f);
        r.savePNG("Harness/out/03_menu_interact.png");
    }

    // 4) mid-open animation keyframe (fade + slide)
    {
        Render2D r(W, H);
        scene(r);
        dim(r, 0.55f);
        Menu m; m.activeTab = 1;
        m.Draw(r, 0.55f);
        r.savePNG("Harness/out/04_menu_opening.png");
    }

    std::printf("wrote 4 frames to Harness/out/\n");
    return 0;
}
