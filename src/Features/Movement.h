#pragma once
#include "../SDK/Sdk.h"
#include "../Config.h"
#include <cmath>

// Misc/movement section — bunny hop, air-strafe optimizer, and counter-strafe
// fast-stop, modeled on ArcticTech's movement helpers. Operates on the outgoing
// command + local state per cfg::g_cfg.misc.
namespace feat {

// yaw_delta: change in view yaw since last tick (drives air-strafe direction).
// view_yaw: current view yaw, used to project velocity into local move space.
inline void movement_run(sdk::UserCmd* cmd, const sdk::Player& local,
                         float view_yaw, float yaw_delta) {
    const auto& c = cfg::g_cfg.misc;
    const float MAXMOVE = 450.f;

    // bunny hop: only let the jump bit land on the ground tick
    if (c.bhop) {
        if (!local.on_ground) cmd->buttons &= ~sdk::IN_JUMP;
        else                  cmd->buttons |= sdk::IN_JUMP;
    }

    // air-strafe: in air, kill forward input and push full sidemove toward the
    // turn so air-acceleration gains speed (sign follows the mouse turn).
    if (c.auto_strafe && !local.on_ground) {
        cmd->forwardmove = 0.f;
        // "strafe smooth" eases the air-steer magnitude (0% = gentle, 100% = full).
        float k = 0.5f + 0.5f * std::clamp(c.auto_strafe_smooth / 100.f, 0.f, 1.f);
        cmd->sidemove = ((yaw_delta >= 0.f) ? MAXMOVE : -MAXMOVE) * k;
        return;
    }

    // fast-stop: when grounded, coasting, and no input, counter-strafe by driving
    // movement opposite to the current velocity so engine friction brakes hard.
    if (c.fast_stop && local.on_ground) {
        bool moving_input = cmd->forwardmove != 0.f || cmd->sidemove != 0.f;
        float speed = local.velocity.length2d();
        if (!moving_input && speed > 5.f) {
            float vel_yaw = std::atan2(local.velocity.y, local.velocity.x) * 180.f / 3.14159265f;
            float rel = sdk::deg2rad(sdk::normalize_yaw(vel_yaw - view_yaw));
            cmd->forwardmove = -std::cos(rel) * MAXMOVE; // oppose motion
            cmd->sidemove    = -std::sin(rel) * MAXMOVE;
        }
    }
}

} // namespace feat
