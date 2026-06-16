#pragma once
#include "Ragebot.h"
#include "AntiAim.h"
#include "Movement.h"
#include "Esp.h"

// Ties the sections together into one create-move pass, in the order the real
// client would run them each command tick.
namespace feat {

struct TickInput {
    sdk::Player local;
    std::vector<sdk::Player> players;
    float view_yaw = 0.f;
    float yaw_delta = 0.f;
    int   tick = 0;
};

struct TickOutput {
    AAResult aa;
    AimResult aim;
    std::vector<EspEntry> esp;
};

inline TickOutput create_move(sdk::UserCmd* cmd, const TickInput& in) {
    TickOutput o;
    // Ragebot first: when it fires we want the real angle on the command, not AA.
    o.aim = ragebot_run(in.local, in.players, in.tick);
    if (o.aim.fire) {
        cmd->buttons |= sdk::IN_ATTACK;
        // snap the command to the chosen aim point (yaw/pitch toward the hitbox)
        sdk::Vec3 d = o.aim.point - in.local.eye;
        cmd->viewangles.y = std::atan2(d.y, d.x) * 180.f / 3.14159265f;
        cmd->viewangles.x = -std::atan2(d.z, d.length2d()) * 180.f / 3.14159265f;
        if (cfg::g_cfg.rage.auto_stop) { cmd->forwardmove = 0.f; cmd->sidemove = 0.f; }
    } else {
        o.aa = anti_aim_apply(cmd, in.local, in.players, in.view_yaw, in.tick);
    }
    movement_run(cmd, in.local, in.view_yaw, in.yaw_delta);
    o.esp = esp_build(in.local, in.players);
    return o;
}

} // namespace feat
