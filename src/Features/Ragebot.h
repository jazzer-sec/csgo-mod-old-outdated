#pragma once
#include "../SDK/Sdk.h"
#include "../Config.h"
#include <string>
#include <vector>

// Rage section — target selection + damage/hitchance gating. Mirrors the real
// ragebot decision path (scan enemies, score, simulate damage, decide to fire),
// driven entirely by cfg::g_cfg.rage.
namespace feat {

struct AimResult {
    bool has_target = false;
    int  target = -1;
    int  damage = 0;
    int  hitchance = 0;
    bool fire = false;
    std::string reason = "idle";
};

inline int estimate_damage(const sdk::Player& local, const sdk::Player& tgt) {
    const sdk::Weapon* w = local.weapon;
    if (!w) return 0;
    float dist = (tgt.eye - local.eye).length();
    float falloff = std::max(0.f, 1.f - dist / (w->range * 0.5f));
    int dmg = (int)(w->base_damage * 4.0f * falloff); // headshot multiplier
    if (tgt.armor > 0) dmg = (int)(dmg * 0.775f);      // armor reduction
    int cap = tgt.health + (tgt.armor > 0 ? 100 : 0);
    return std::max(0, std::min(dmg, cap));
}

inline int estimate_hitchance(const sdk::Player& local, const sdk::Player& tgt) {
    float dist = (tgt.eye - local.eye).length();
    return (int)std::clamp(100.f - dist / 50.f, 0.f, 100.f);
}

inline AimResult ragebot_run(const sdk::Player& local, const std::vector<sdk::Player>& players) {
    AimResult res;
    const auto& c = cfg::g_cfg.rage;
    if (!c.enabled) { res.reason = "disabled"; return res; }

    const sdk::Player* best = nullptr;
    float bestKey = 1e9f;
    for (const auto& p : players) {
        if (!p.alive || !p.enemy) continue;
        float key;
        switch (c.target) {
            case 1:  key = (float)p.health; break;                       // lowest hp
            case 3:  key = -(float)estimate_damage(local, p); break;     // high damage
            default: key = (p.eye - local.eye).length(); break;          // nearest / crosshair
        }
        if (key < bestKey) { bestKey = key; best = &p; }
    }
    if (!best) { res.reason = "no target"; return res; }

    res.has_target = true;
    res.target = best->index;
    res.damage = estimate_damage(local, *best);
    res.hitchance = estimate_hitchance(local, *best);

    bool ready = !local.weapon || local.weapon->next_primary_attack <= sdk::g_globals.curtime;
    if (res.damage < c.min_damage)    { res.reason = "min damage"; return res; }
    if (res.hitchance < c.hitchance)  { res.reason = "hitchance"; return res; }
    if (!ready)                       { res.reason = "not ready"; return res; }

    res.fire = true;
    res.reason = "fire";
    return res;
}

} // namespace feat
