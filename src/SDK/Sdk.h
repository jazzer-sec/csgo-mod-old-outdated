#pragma once
#include "Math.h"
#include <string>
#include <vector>

// Minimal self-contained engine stand-ins (same philosophy as JazzHook/SDK):
// just enough of the Source/CS:GO surface for the feature logic to be real and
// testable. Swap these for the actual interfaces when wiring into a client.
namespace sdk {

enum Buttons {
    IN_ATTACK    = 1 << 0,
    IN_JUMP      = 1 << 1,
    IN_DUCK      = 1 << 2,
    IN_FORWARD   = 1 << 3,
    IN_BACK      = 1 << 4,
    IN_MOVELEFT  = 1 << 9,
    IN_MOVERIGHT = 1 << 10,
};

enum WeaponId { WEAP_NONE = 0, WEAP_DEAGLE = 1, WEAP_AK = 7, WEAP_AWP = 9, WEAP_REVOLVER = 64 };

struct Weapon {
    int   def_index = WEAP_AK;
    int   base_damage = 36;
    float range = 8192.f;
    float range_modifier = 0.98f;  // per-CS:GO damage falloff base
    float inaccuracy = 0.012f;     // base cone (radians) for hitchance sims
    int   penetration = 1;         // wall-bang power (autowall model)
    float next_primary_attack = 0.f; // curtime gate
    bool  is_sniper = false;
};

struct UserCmd {
    int   command_number = 0;
    int   tick_count = 0;
    float forwardmove = 0.f;
    float sidemove = 0.f;
    float upmove = 0.f;
    int   buttons = 0;
    Vec3  viewangles; // x = pitch, y = yaw
};

struct Player {
    int    index = 0;
    bool   alive = true;
    bool   enemy = true;
    int    team = 3;
    int    health = 100;
    int    armor = 100;
    bool   has_helmet = true;
    Vec3   origin, eye, velocity;
    bool   on_ground = true;
    bool   ducking = false;
    bool   scoped = false;
    bool   defusing = false;
    float  lower_body_yaw = 0.f;   // LBY / animation yaw (desync reference)
    std::string name;
    Weapon* weapon = nullptr;
};

struct Globals {
    float interval_per_tick = 1.f / 64.f;
    float curtime = 0.f;
    int   tickcount = 0;
};

inline Globals g_globals;

} // namespace sdk
