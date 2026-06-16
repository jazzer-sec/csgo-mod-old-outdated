// Headless interaction test for the standalone app: drives app_frame() with
// synthetic mouse input and asserts the menu actually mutates cfg::g_cfg. Runs
// without SDL (the windowing layer) so the interaction logic is verifiable here.
//   g++ -std=c++17 -O2 App/headless_test.cpp Harness/Render2D.cpp Harness/Hud.cpp \
//       Harness/Menu.cpp $(pkg-config --cflags --libs freetype2) -o /tmp/apptest
#include "Scene.h"
#include <cstdio>

static int fails = 0;
#define CHECK(c, msg) do { if (!(c)) { std::printf("  FAIL  %s\n", msg); ++fails; } \
                           else std::printf("  ok    %s\n", msg); } while (0)

static Menu::Input mouse(float x, float y, bool down, bool clicked) {
    Menu::Input in; in.mx = x; in.my = y; in.down = down; in.clicked = clicked;
    return in;
}

int main() {
    Render2D r(1280, 720);
    AppState st;
    app_init(st);

    // open the menu and let the animation settle
    app_frame(r, st, mouse(0, 0, false, false), true, 0.3f);
    for (int i = 0; i < 6; ++i)
        app_frame(r, st, mouse(0, 0, false, false), false, 0.3f);
    CHECK(st.open.cur > 0.999f, "menu animates fully open");

    // widget coords for the centered menu (W=1280,H=720 => x=348,y=180)
    std::printf("[interaction]\n");
    bool wasEnabled = cfg::g_cfg.rage.enabled;
    app_frame(r, st, mouse(612, 278, true, true), false, 0.016f);
    CHECK(cfg::g_cfg.rage.enabled != wasEnabled, "clicking a checkbox toggles its cfg field");

    app_frame(r, st, mouse(700, 308, true, false), false, 0.016f);
    CHECK(cfg::g_cfg.rage.min_damage >= 95, "dragging a slider writes the value");

    app_frame(r, st, mouse(424, 303, true, true), false, 0.016f);
    CHECK(st.menu.activeTab == 1, "clicking a sidebar tab switches the page");

    // let the tab-highlight glide finish, then snapshot for a visual sanity check
    for (int i = 0; i < 12; ++i)
        app_frame(r, st, mouse(0, 0, false, false), false, 0.05f);
    r.savePNG("App/out/standalone_frame.png");
    std::printf("wrote App/out/standalone_frame.png\n");

    std::printf(fails ? "\n%d check(s) FAILED\n" : "\nall checks passed\n", fails);
    return fails ? 1 : 0;
}
