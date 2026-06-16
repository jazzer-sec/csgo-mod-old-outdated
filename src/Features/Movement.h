#pragma once
#include "../SDK/Sdk.h"
#include "../Config.h"

// Misc/movement section — bunny hop, auto-strafe, fast-stop. Operates on the
// outgoing command + local player state per cfg::g_cfg.misc.
namespace feat {

// yaw_delta: change in view yaw since last tick (drives auto-strafe direction).
inline void movement_run(sdk::UserCmd* cmd, const sdk::Player& local, float yaw_delta) {
    const auto& c = cfg::g_cfg.misc;

    // bunny hop: only allow the jump bit to land on the ground tick
    if (c.bhop) {
        if (!local.on_ground) cmd->buttons &= ~sdk::IN_JUMP;
        else                  cmd->buttons |= sdk::IN_JUMP;
    }

    // auto-strafe: in air, steer sidemove toward the turn direction
    if (c.auto_strafe && !local.on_ground) {
        cmd->sidemove = (yaw_delta >= 0.f) ? 450.f : -450.f;
    }

    // fast-stop: kill movement input when grounded and coasting, so the engine
    // friction brakes immediately (a full impl counter-strafes against velocity).
    if (c.fast_stop && local.on_ground) {
        bool moving_input = cmd->forwardmove != 0.f || cmd->sidemove != 0.f;
        if (!moving_input && local.velocity.length2d() > 5.f) {
            cmd->forwardmove = 0.f;
            cmd->sidemove = 0.f;
        }
    }
}

} // namespace feat
