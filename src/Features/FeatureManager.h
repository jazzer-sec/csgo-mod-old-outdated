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
    o.aa  = anti_aim_apply(cmd, in.view_yaw, in.tick);
    movement_run(cmd, in.local, in.yaw_delta);
    o.aim = ragebot_run(in.local, in.players);
    if (o.aim.fire) cmd->buttons |= sdk::IN_ATTACK;
    o.esp = esp_build(in.local, in.players);
    return o;
}

} // namespace feat
