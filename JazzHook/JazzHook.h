#pragma once

#include "SDK/Stubs.h"

// JazzHook
// --------
// A custom prediction feature module modeled on ArcticTech's CPrediction
// (Features/Misc/Prediction.cpp ~line 338): the R8 Revolver's delayed-fire
// timing. The R8 doesn't fire on the tick you press attack; holding the
// trigger arms a "postpone fire ready" time and the round only leaves on a
// later tick. JazzHook predicts that ready tick from a circular history of
// recent commands so the rest of the mod knows exactly when the shot lands.

class JazzHook
{
public:
    // Lifecycle, mirroring CPrediction's frame hooks.
    void Start(C_BasePlayer* local);
    void End();

    // Called once per user command before it is sent. This is where the
    // revolver fire-ready prediction lives (the analogue of line 338).
    void RunCommand(C_BasePlayer* local, CUserCmd* cmd);

    // True once the predicted ready time has elapsed: the held R8 trigger
    // will actually release a round on the current command.
    bool WillFireThisTick() const { return will_fire_this_tick_; }

    // The tick on which we expect the held R8 shot to fire.
    int PredictedFireTick() const { return predicted_fire_tick_; }

private:
    static constexpr int kHistorySize = 150; // ~2.3s at 64 tick

    struct CommandRecord
    {
        int   command_number = 0;
        int   tick_count     = 0;
        int   buttons        = 0;
        float postpone_ready = 0.f;
        bool  valid          = false;
    };

    void StoreRecord(const CUserCmd* cmd, float postpone_ready);
    const CommandRecord* FindRecord(int command_number) const;

    // Circular buffer of recent commands (mirrors the cheat's history ring).
    CommandRecord history_[kHistorySize] = {};
    int           history_head_ = 0;

    C_BasePlayer* local_              = nullptr;
    bool          will_fire_this_tick_ = false;
    int           predicted_fire_tick_ = -1;
    bool          trigger_held_        = false;
};

// Single shared instance, the way these feature modules are usually exposed.
inline JazzHook g_jazz_hook;
