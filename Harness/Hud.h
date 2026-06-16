#pragma once
#include "Render2D.h"

// Hud — the always-on overlay suite drawn over the game world: watermark,
// keybind list, spectator list, hit log, bottom indicators, off-screen arrow,
// crosshair, and sample player ESP. All mocked here; in the mod each pulls from
// the entity list / events / m_KeyBinds as noted in the plan.
namespace Hud {
    void DrawBackdrop(Render2D& r);   // fake map so overlays have context
    void DrawWorld(Render2D& r);      // ESP players, tracers, off-screen arrow
    void DrawOverlays(Render2D& r);   // watermark / keybinds / specs / hitlog / indicators
}
