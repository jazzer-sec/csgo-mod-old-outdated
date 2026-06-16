#include "JazzHook.h"

// The R8 Revolver's secondary-style primary fire: pressing IN_ATTACK arms the
// shot rather than firing it. The engine stamps m_flPostponeFireReadyTime and
// only lets the round leave once curtime reaches it. We reproduce that gate
// here from command history so the rest of the mod can plan around the exact
// fire tick.
//
// Reference: ArcticTech Features/Misc/Prediction.cpp, RunCommand() ~line 338.

namespace
{
    // Approximate arm delay the R8 applies when the trigger is first held.
    // Real value comes from the weapon script; kept as a named constant so it
    // is easy to retune against a live client.
    constexpr float kRevolverPostponeDelay = 0.234375f; // ~15 ticks @ 64
}

void JazzHook::Start(C_BasePlayer* local)
{
    local_ = local;
    will_fire_this_tick_ = false;
    predicted_fire_tick_ = -1;
}

void JazzHook::End()
{
    will_fire_this_tick_ = false;
}

void JazzHook::StoreRecord(const CUserCmd* cmd, float postpone_ready)
{
    CommandRecord& rec = history_[history_head_];
    rec.command_number = cmd->command_number;
    rec.tick_count     = cmd->tick_count;
    rec.buttons        = cmd->buttons;
    rec.postpone_ready = postpone_ready;
    rec.valid          = true;

    history_head_ = (history_head_ + 1) % kHistorySize;
}

const JazzHook::CommandRecord* JazzHook::FindRecord(int command_number) const
{
    for (const CommandRecord& rec : history_)
    {
        if (rec.valid && rec.command_number == command_number)
            return &rec;
    }
    return nullptr;
}

void JazzHook::RunCommand(C_BasePlayer* local, CUserCmd* cmd)
{
    will_fire_this_tick_ = false;

    if (!local || !cmd)
        return;

    C_BaseCombatWeapon* weapon = local->GetActiveWeapon();
    if (!weapon)
        return;

    // Only the R8 has this postpone-fire behaviour; everything else fires on
    // the press tick and needs no special handling here.
    if (weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER)
    {
        trigger_held_ = false;
        StoreRecord(cmd, 0.f);
        return;
    }

    const bool attack_down = (cmd->buttons & IN_ATTACK) != 0;
    float postpone_ready   = weapon->m_flPostponeFireReadyTime();

    if (attack_down)
    {
        // Rising edge: the trigger was just pulled, so the engine is about to
        // arm the postpone timer. Predict the ready time ourselves.
        if (!trigger_held_)
        {
            postpone_ready = g_global_vars.curtime + kRevolverPostponeDelay;
            weapon->set_PostponeFireReadyTime(postpone_ready);
        }
        trigger_held_ = true;
    }
    else
    {
        trigger_held_ = false;
    }

    // Translate the (predicted) ready time into a tick and decide whether the
    // shot leaves on this very command.
    if (postpone_ready > 0.f)
    {
        predicted_fire_tick_ = cmd->tick_count + TIME_TO_TICKS(postpone_ready - g_global_vars.curtime);
        will_fire_this_tick_ = trigger_held_ && (g_global_vars.curtime >= postpone_ready);
    }
    else
    {
        predicted_fire_tick_ = -1;
    }

    StoreRecord(cmd, postpone_ready);
}
