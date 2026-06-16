#pragma once
#include "../SDK/Sdk.h"
#include "../Config.h"
#include <vector>

// Anti-aim section — fake/real angle separation modeled on ArcticTech's
// anti-aim + animation desync: pitch, base direction (incl. freestanding toward
// safety), yaw jitter, and a real desync delta bounded by movement state. The
// "real" yaw is what we send on the command; "fake" is what enemies resolve to.
namespace feat {

struct AAResult {
    float yaw = 0.f;        // real / command yaw
    float pitch = 0.f;
    float fake_yaw = 0.f;   // what enemies see (real + side * desync_delta)
    float desync_delta = 0.f;
    int   desync_side = 1;  // -1 / +1
    bool  fake_duck = false;
    bool  freestanding = false;
};

// Achievable desync, bounded by animation state: ~58° standing, less while
// running, a touch more crouched. Stand-in for get_max_desync_delta().
inline float max_desync_delta(const sdk::Player& p) {
    float speed = p.velocity.length2d();
    float t = std::clamp(speed / 260.f, 0.f, 1.f);
    float d = 58.f - t * 18.f;          // 58 standing -> 40 running
    if (p.ducking) d += 2.f;
    return d;
}

// Yaw (deg) from local toward a world point, on the XY plane.
inline float yaw_to(const sdk::Player& local, const sdk::Vec3& at) {
    sdk::Vec3 d = at - local.eye;
    return std::atan2(d.y, d.x) * 180.f / 3.14159265f;
}

// Freestanding: face the model away from the dominant threat so the desync hides
// the real angle behind cover/body. Returns the chosen base yaw.
inline float freestand_yaw(const sdk::Player& local, const std::vector<sdk::Player>& enemies,
                           float fallback) {
    float sx = 0.f, sy = 0.f; int n = 0;
    for (const auto& e : enemies) {
        if (!e.alive || !e.enemy) continue;
        sdk::Vec3 d = e.eye - local.eye;
        float len = d.length2d();
        if (len < 1.f) continue;
        sx += d.x / len; sy += d.y / len; ++n;
    }
    if (!n) return fallback;
    float threat = std::atan2(sy, sx) * 180.f / 3.14159265f;
    return sdk::normalize_yaw(threat + 180.f); // point away from the threat
}

inline AAResult anti_aim_apply(sdk::UserCmd* cmd, const sdk::Player& local,
                               const std::vector<sdk::Player>& enemies,
                               float view_yaw, int tick) {
    AAResult r;
    const auto& c = cfg::g_cfg.aa;
    if (!c.enabled) {
        r.yaw = view_yaw; r.fake_yaw = view_yaw;
        r.pitch = cmd->viewangles.x;
        return r;
    }

    // pitch
    r.pitch = c.pitch == 0 ? 89.f : (c.pitch == 1 ? -89.f : 0.f);

    // base yaw relative to the view direction
    float base = view_yaw;
    switch (c.yaw_base) {
        case 1: base = view_yaw + 180.f; break;
        case 2: base = view_yaw + 90.f;  break;
        case 3: base = view_yaw - 90.f;  break;
        default: base = view_yaw;         break;
    }

    // freestanding overrides the base to point away from the threat
    if (c.freestand) { base = freestand_yaw(local, enemies, base); r.freestanding = true; }

    // manual override takes priority over everything
    if (c.manual_left)       base = view_yaw + 90.f;
    else if (c.manual_right) base = view_yaw - 90.f;
    else if (c.manual_back)  base = view_yaw + 180.f;

    // yaw jitter
    float jitter = 0.f;
    switch (c.yaw_jitter) {
        case 2: jitter = (tick % 2 ? -1.f : 1.f) * (c.desync * 0.3f); break; // offset
        case 3: jitter = (float)((tick * 53) % 60) - 30.f; break;           // random
        default: jitter = 0.f; break;
    }

    r.yaw = sdk::normalize_yaw(base + jitter);
    r.desync_side = (c.inverter == 0) ? 1 : ((tick % 2) ? 1 : -1);
    r.desync_delta = max_desync_delta(local) * (c.desync / 100.f);
    r.fake_yaw = sdk::normalize_yaw(r.yaw + r.desync_side * r.desync_delta);
    r.fake_duck = c.fake_duck;

    cmd->viewangles.x = r.pitch;
    cmd->viewangles.y = r.yaw;
    if (r.fake_duck) cmd->buttons |= sdk::IN_DUCK;
    return r;
}

} // namespace feat
