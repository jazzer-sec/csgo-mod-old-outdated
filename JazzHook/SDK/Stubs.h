#pragma once

// Minimal self-contained SDK stubs.
//
// The original ArcticTech tree pulls these symbols from real engine
// interfaces (../../SDK/Interfaces.h, etc.). This repo has no SDK, so the
// pieces JazzHook actually touches are reproduced here just well enough to
// build and reason about the logic. Swap these for the real interfaces when
// wiring into an actual client.

#include <cstdint>
#include <cstring>

// CS:GO item definition indices we care about. Only the revolver matters here.
enum ItemDefinitionIndex : int
{
    WEAPON_REVOLVER = 64, // R8 Revolver
};

// Stand-in for CUserCmd. The real struct is much larger; these are the
// fields JazzHook reads/writes.
struct CUserCmd
{
    int   command_number = 0;
    int   tick_count     = 0;
    float forwardmove    = 0.f;
    float sidemove       = 0.f;
    int   buttons        = 0;
};

enum InButtons : int
{
    IN_ATTACK = (1 << 0),
};

// Stand-in for the active weapon. m_flPostponeFireReadyTime gates when the
// R8's held-trigger shot is actually allowed to fire.
class C_BaseCombatWeapon
{
public:
    int item_definition_index = 0;
    float postpone_fire_ready_time = 0.f;

    int   m_iItemDefinitionIndex() const { return item_definition_index; }
    float m_flPostponeFireReadyTime() const { return postpone_fire_ready_time; }
    void  set_PostponeFireReadyTime(float v) { postpone_fire_ready_time = v; }
};

// Stand-in for the local player.
class C_BasePlayer
{
public:
    C_BaseCombatWeapon* active_weapon = nullptr;
    C_BaseCombatWeapon* GetActiveWeapon() const { return active_weapon; }
};

// Global tick/interval helpers the prediction code leans on.
struct GlobalVars
{
    float interval_per_tick = 1.f / 64.f; // 64-tick default
    float curtime           = 0.f;
};

inline GlobalVars g_global_vars;

inline float TICKS_TO_TIME(int ticks)
{
    return ticks * g_global_vars.interval_per_tick;
}

inline int TIME_TO_TICKS(float t)
{
    return static_cast<int>(0.5f + t / g_global_vars.interval_per_tick);
}
