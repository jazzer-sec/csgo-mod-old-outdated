#pragma once
#include "../SDK/Sdk.h"
#include "../Config.h"

// Anti-aim section — computes fake/real angles from cfg::g_cfg.aa and writes
// them into the outgoing command. Handles base direction, manual override,
// jitter, desync side, and fake-duck.
namespace feat {

struct AAResult {
    float yaw = 0.f;
    float pitch = 0.f;
    bool  fake_duck = false;
    int   desync_side = 1; // -1 / +1
};

inline AAResult anti_aim_apply(sdk::UserCmd* cmd, float view_yaw, int tick) {
    AAResult r;
    const auto& c = cfg::g_cfg.aa;
    if (!c.enabled) {
        r.yaw = view_yaw;
        r.pitch = cmd->viewangles.x;
        return r;
    }

    // pitch
    r.pitch = c.pitch == 0 ? 89.f : (c.pitch == 1 ? -89.f : 0.f);

    // base yaw, relative to the view direction
    float base = view_yaw;
    switch (c.yaw_base) {
        case 1: base = view_yaw + 180.f; break;
        case 2: base = view_yaw + 90.f;  break;
        case 3: base = view_yaw - 90.f;  break;
        default: base = view_yaw;         break;
    }

    // manual override takes priority over the base direction
    if (c.manual_left)       base = view_yaw + 90.f;
    else if (c.manual_right) base = view_yaw - 90.f;
    else if (c.manual_back)  base = view_yaw + 180.f;

    // jitter
    float jitter = 0.f;
    switch (c.yaw_jitter) {
        case 2: jitter = (tick % 2 ? -1.f : 1.f) * (c.desync * 0.3f); break; // offset
        case 3: jitter = (float)((tick * 53) % 60) - 30.f; break;           // random
        default: jitter = 0.f; break;
    }

    r.yaw = sdk::normalize_yaw(base + jitter);
    r.desync_side = (c.inverter == 0) ? 1 : ((tick % 2) ? 1 : -1);
    r.fake_duck = c.fake_duck;

    cmd->viewangles.x = r.pitch;
    cmd->viewangles.y = r.yaw;
    if (r.fake_duck) cmd->buttons |= sdk::IN_DUCK;
    return r;
}

} // namespace feat
