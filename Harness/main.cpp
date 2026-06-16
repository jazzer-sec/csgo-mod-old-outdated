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
#include "GifWriter.h"
#include <cstdio>
#include <string>
#include <vector>

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

// 2x box-downscale to RGBA, so the GIF is half-size (640x360) and lighter.
static std::vector<uint8_t> downscale2(const Render2D& r) {
    int w = r.width() / 2, h = r.height() / 2;
    const uint8_t* src = r.data();
    std::vector<uint8_t> out((size_t)w * h * 4);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            int sx = x * 2, sy = y * 2;
            for (int c = 0; c < 4; ++c) {
                int s = src[((size_t)(sy)     * r.width() + sx)     * 4 + c]
                      + src[((size_t)(sy)     * r.width() + sx + 1) * 4 + c]
                      + src[((size_t)(sy + 1) * r.width() + sx)     * 4 + c]
                      + src[((size_t)(sy + 1) * r.width() + sx + 1) * 4 + c];
                out[((size_t)y * w + x) * 4 + c] = (uint8_t)(s / 4);
            }
        }
    return out;
}

// Render one animation frame: scene + dimmed menu at the given open factor.
static std::vector<uint8_t> animFrame(float openT, int activeTab, bool comboOpen) {
    Render2D r(W, H);
    scene(r);
    dim(r, openT);
    if (openT > 0.001f) {
        Menu m; m.activeTab = activeTab; m.comboOpen = comboOpen;
        m.Draw(r, openT);
    }
    return downscale2(r);
}

// Build the interaction timeline and export it as a looping GIF.
static void writeGif() {
    std::vector<std::vector<uint8_t>> frames;
    auto hold = [&](float openT, int tab, bool combo, int n) {
        std::vector<uint8_t> f = animFrame(openT, tab, combo);
        for (int i = 0; i < n; ++i) frames.push_back(f);
    };
    auto ramp = [&](float a, float b, int tab, bool combo, int n) {
        for (int i = 0; i < n; ++i) {
            float t = (float)i / (n - 1);
            frames.push_back(animFrame(a + (b - a) * t, tab, combo));
        }
    };

    hold(0.f, 0, false, 8);          // closed scene
    ramp(0.f, 1.f, 0, false, 16);    // open: fade + slide (eased inside Menu)
    hold(1.f, 0, false, 14);         // rage tab settles
    hold(1.f, 0, true, 16);          // open the target dropdown
    hold(1.f, 0, false, 6);          // close it
    hold(1.f, 1, false, 18);         // anti-aim
    hold(1.f, 2, false, 18);         // visuals
    hold(1.f, 3, false, 18);         // misc
    hold(1.f, 4, false, 18);         // configs
    hold(1.f, 0, false, 8);          // back to rage
    ramp(1.f, 0.f, 0, false, 14);    // close
    hold(0.f, 0, false, 10);         // rest before loop

    int w = W / 2, h = H / 2;
    if (gif::write_animation("Harness/out/preview.gif", w, h, frames, 4))
        std::printf("wrote Harness/out/preview.gif (%zu frames, %dx%d)\n",
                    frames.size(), w, h);
    else
        std::printf("GIF write FAILED\n");
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

    // 4..7) each tab's own page
    const char* names[] = {nullptr, "04_antiaim", "05_visuals", "06_misc", "07_configs"};
    for (int tab = 1; tab <= 4; ++tab) {
        Render2D r(W, H);
        scene(r);
        dim(r, 1.f);
        Menu m; m.activeTab = tab;
        m.Draw(r, 1.f);
        r.savePNG(std::string("Harness/out/") + names[tab] + ".png");
    }

    // 8) mid-open animation keyframe (fade + slide)
    {
        Render2D r(W, H);
        scene(r);
        dim(r, 0.55f);
        Menu m; m.activeTab = 0;
        m.Draw(r, 0.55f);
        r.savePNG("Harness/out/08_menu_opening.png");
    }

    std::printf("wrote frames to Harness/out/\n");

    // animated GIF of the open → tabs → close interaction
    writeGif();
    return 0;
}
