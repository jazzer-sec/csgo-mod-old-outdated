#pragma once
#include "../SDK/Sdk.h"
#include "../Config.h"
#include <string>
#include <vector>

// Visuals section — builds the per-enemy ESP entries the HUD draws. Pure data
// (no rendering here); the harness/HUD turns these into boxes/flags.
namespace feat {

struct EspEntry {
    int index = 0;
    std::string name;
    int health = 0;
    int distance = 0; // feet-ish
    std::vector<std::string> flags;
};

inline std::vector<EspEntry> esp_build(const sdk::Player& local, const std::vector<sdk::Player>& players) {
    std::vector<EspEntry> out;
    if (!cfg::g_cfg.visuals.players) return out;
    for (const auto& p : players) {
        if (!p.alive || !p.enemy) continue;
        EspEntry e;
        e.index = p.index;
        e.name = p.name;
        e.health = p.health;
        e.distance = (int)((p.origin - local.origin).length() / 12.f);
        if (p.scoped)   e.flags.push_back("scoped");
        if (p.defusing) e.flags.push_back("defusing");
        if (p.health < 40) e.flags.push_back("low");
        out.push_back(e);
    }
    return out;
}

} // namespace feat
