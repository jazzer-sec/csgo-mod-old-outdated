#pragma once
// Scene — the standalone "fake game" frame: a mocked HvH scene with live ESP/HUD
// overlays, the real feature pipeline (create_move) running over scripted bots,
// and the interactive menu composited on top. Backend-agnostic: it only needs a
// Render2D framebuffer and a Menu::Input, so the same frame() drives the SDL
// window and the headless test. No game, no injection — the cheat logic runs
// against simulated opponents inside this build.

#include "../Harness/Render2D.h"
#include "../Harness/Theme.h"
#include "../Harness/Anim.h"
#include "../Harness/Hud.h"
#include "../Harness/Menu.h"
#include "../src/Features/FeatureManager.h"
#include <cmath>
#include <string>
#include <vector>

struct AppState {
    Menu  menu;
    Anim  open{0.f};        // menu open/close factor (eased)
    bool  menuTarget = false;

    // simulated scene
    sdk::Weapon weapon;
    sdk::Player local;
    std::vector<sdk::Player> bots;
    float orbit = 0.f;      // bots circle the local player
    feat::TickOutput last;  // most recent pipeline result (for the status line)
};

inline void app_init(AppState& st) {
    sdk::g_globals.curtime = 100.f;
    st.weapon.def_index = sdk::WEAP_AK;
    st.weapon.base_damage = 36;
    st.local.eye = {0, 0, 64};
    st.local.origin = {0, 0, 0};
    st.local.enemy = false;
    st.local.weapon = &st.weapon;
    st.local.name = "skippy";

    for (int i = 0; i < 3; ++i) {
        sdk::Player b;
        b.index = i + 1;
        b.enemy = true;
        b.health = 100 - i * 30;
        b.armor = (i == 0) ? 100 : 0;
        b.name = (i == 0) ? "trash_kid" : (i == 1 ? "noscoper" : "дамский угодник");
        b.scoped = (i == 1);
        st.bots.push_back(b);
    }
}

// Advance the scripted scene and run the real create_move pipeline over it.
inline void app_sim(AppState& st, float dt) {
    sdk::g_globals.curtime += dt;
    st.orbit += dt * 0.6f;
    for (size_t i = 0; i < st.bots.size(); ++i) {
        float a = st.orbit + (float)i * 2.094f; // 120° apart
        float radius = 320.f + 120.f * (float)i;
        st.bots[i].origin = {std::cos(a) * radius, std::sin(a) * radius, 0};
        st.bots[i].eye = st.bots[i].origin + sdk::Vec3{0, 0, 64};
    }
    sdk::UserCmd cmd;
    feat::TickInput in;
    in.local = st.local;
    in.players = st.bots;
    in.view_yaw = 0.f;
    in.tick = (int)(sdk::g_globals.curtime * 64.f);
    st.last = feat::create_move(&cmd, in);
}

// Draw the live rage decision so the simulated logic is visible reacting to the
// menu (raise min-damage / hitchance and watch it stop firing, etc.).
inline void app_status(Render2D& r, const AppState& st) {
    const Theme& t = theme();
    const feat::AimResult& a = st.last.aim;
    r.Text(14, 12, "jazzhook standalone — simulated scene (no game)", t.textDim, 1);
    std::string line = "rage: " + a.reason;
    if (a.has_target) {
        line += "  tgt#" + std::to_string(a.target) +
                "  dmg " + std::to_string(a.damage) +
                "  hc " + std::to_string(a.hitchance) + "%";
        if (a.safe_point) line += "  [safe]";
    }
    r.Text(14, 28, line, a.fire ? t.accent : t.textDim, 1);
    r.Text(14, 44, "INSERT: menu    ESC: quit", t.textFaint, 1);
}

// One full frame: sim -> scene/overlays -> status -> menu. `toggleMenu` is the
// edge-triggered INSERT key; `in` is the current mouse state.
inline void app_frame(Render2D& r, AppState& st, const Menu::Input& in,
                      bool toggleMenu, float dt) {
    if (toggleMenu) st.menuTarget = !st.menuTarget;
    st.open.set(st.menuTarget ? 1.f : 0.f);
    st.open.step(dt);

    app_sim(st, dt);

    r.Clear(Color(0, 0, 0));
    Hud::DrawBackdrop(r);
    Hud::DrawWorld(r);
    Hud::DrawOverlays(r);
    app_status(r, st);

    if (st.open.cur > 0.001f) {
        r.BoxFilled(0, 0, (float)r.width(), (float)r.height(),
                    Color(0, 0, 0, (int)(120 * std::clamp(st.open.cur, 0.f, 1.f))));
        st.menu.input = in;
        st.menu.frameDt = dt;
        st.menu.Draw(r, st.open.cur);
    }
}
