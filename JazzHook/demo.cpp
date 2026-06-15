// Tiny driver that exercises JazzHook without a game attached. It fakes a
// local player holding an R8 and steps commands forward, printing when the
// module predicts the shot will fire. Build:
//
//   g++ -std=c++17 JazzHook/JazzHook.cpp JazzHook/demo.cpp -o jazzhook_demo
//   ./jazzhook_demo

#include "JazzHook.h"
#include <cstdio>

int main()
{
    C_BaseCombatWeapon r8;
    r8.item_definition_index = WEAPON_REVOLVER;

    C_BasePlayer player;
    player.active_weapon = &r8;

    g_global_vars.curtime = 0.f;
    g_jazz_hook.Start(&player);

    bool reported_fire = false;
    for (int tick = 0; tick < 40; ++tick)
    {
        g_global_vars.curtime = TICKS_TO_TIME(tick);

        CUserCmd cmd;
        cmd.command_number = tick;
        cmd.tick_count     = tick;
        cmd.buttons        = IN_ATTACK; // hold the trigger the whole time

        g_jazz_hook.RunCommand(&player, &cmd);

        if (g_jazz_hook.WillFireThisTick() && !reported_fire)
        {
            std::printf("tick %d (t=%.4f): R8 releases the round; predicted fire tick was %d\n",
                        tick, g_global_vars.curtime, g_jazz_hook.PredictedFireTick());
            reported_fire = true;
        }
    }

    g_jazz_hook.End();

    if (!reported_fire)
    {
        std::printf("R8 never reached its postpone-fire-ready time in the window\n");
        return 1;
    }
    return 0;
}
