#pragma once
#include "../SDK/Sdk.h"
#include "../SDK/Hitbox.h"
#include "../Config.h"
#include <string>
#include <vector>

// Rage section — ArcticTech-style aimbot decision path: scan enemies, build a
// multipoint candidate set per hitbox, run the damage + hitchance model on each
// point, keep the best point that clears the config gates, then pick the target
// by the configured priority and decide whether to fire. Driven by cfg::g_cfg.rage.
namespace feat {

struct AimResult {
    bool has_target = false;
    int  target = -1;
    int  hitbox = sdk::HB_HEAD;
    sdk::Vec3 point;
    int  damage = 0;
    int  hitchance = 0;
    bool safe_point = false;
    bool fire = false;
    std::string reason = "idle";
};

// Hitboxes the ragebot will try, head first (best damage), body as fallback.
inline const int* scan_hitboxes(int& n) {
    static const int hb[] = { sdk::HB_HEAD, sdk::HB_CHEST, sdk::HB_STOMACH,
                              sdk::HB_PELVIS, sdk::HB_LEG_L, sdk::HB_LEG_R };
    n = (int)(sizeof(hb) / sizeof(hb[0]));
    return hb;
}

// Multipoint set for a hitbox: center (the safe point) plus 4 scaled offsets,
// so a partially exposed enemy can still be hit at the edges.
inline void multipoints(const sdk::HitboxInfo& hbi, std::vector<sdk::Vec3>& out,
                        std::vector<bool>& safe) {
    out.clear(); safe.clear();
    out.push_back(hbi.pos);                 safe.push_back(true);  // center = safe
    const float s = hbi.radius * 0.85f;
    out.push_back({hbi.pos.x + s, hbi.pos.y, hbi.pos.z}); safe.push_back(false);
    out.push_back({hbi.pos.x - s, hbi.pos.y, hbi.pos.z}); safe.push_back(false);
    out.push_back({hbi.pos.x, hbi.pos.y, hbi.pos.z + s}); safe.push_back(false);
    out.push_back({hbi.pos.x, hbi.pos.y, hbi.pos.z - s}); safe.push_back(false);
}

// Best firing point on one enemy that clears min damage + hitchance. Prefers
// higher damage; among equal damage prefers the safe (center) point.
inline AimResult best_point(const sdk::Player& local, const sdk::Player& tgt, int tick) {
    const auto& c = cfg::g_cfg.rage;
    AimResult r;
    r.target = tgt.index;

    int nhb; const int* hbs = scan_hitboxes(nhb);
    std::vector<sdk::Vec3> pts; std::vector<bool> safe;
    int bestScore = -1;
    for (int i = 0; i < nhb; ++i) {
        int hb = hbs[i];
        if (hb == sdk::HB_HEAD && c.force_baim) continue;   // body aim only
        sdk::HitboxInfo hbi = sdk::hitbox_info(tgt, hb);
        multipoints(hbi, pts, safe);
        for (size_t k = 0; k < pts.size(); ++k) {
            // "aim head if safe": only take the head on its safe (center) point
            if (hb == sdk::HB_HEAD && c.aim_head_safe && !safe[k]) continue;
            int dmg = sdk::get_damage(local, tgt, hb, pts[k]);
            if (dmg < c.min_damage) continue;
            int hc = sdk::hitchance(local, tgt, hb, pts[k], tick);
            if (hc < c.hitchance) continue;
            // score: damage dominates, safe point breaks ties
            int score = dmg * 2 + (safe[k] ? 1 : 0);
            if (score > bestScore) {
                bestScore = score;
                r.has_target = true;
                r.hitbox = hb;
                r.point = pts[k];
                r.damage = dmg;
                r.hitchance = hc;
                r.safe_point = safe[k];
            }
        }
    }
    return r;
}

inline AimResult ragebot_run(const sdk::Player& local, const std::vector<sdk::Player>& players, int tick = 0) {
    AimResult res;
    const auto& c = cfg::g_cfg.rage;
    if (!c.enabled) { res.reason = "disabled"; return res; }

    // Pick the target by configured priority, among enemies we can actually hit.
    const sdk::Player* best = nullptr;
    AimResult bestAim;
    float bestKey = 1e9f;
    for (const auto& p : players) {
        if (!p.alive || !p.enemy) continue;
        AimResult a = best_point(local, p, tick);
        if (!a.has_target) continue; // no point clears the gates on this enemy

        float key;
        switch (c.target) {
            case 1:  key = (float)p.health; break;               // lowest hp
            case 3:  key = -(float)a.damage; break;              // high damage
            default: key = (p.eye - local.eye).length(); break;  // nearest / crosshair
        }
        if (key < bestKey) { bestKey = key; best = &p; bestAim = a; }
    }

    if (!best) { res.reason = "no target"; return res; }
    res = bestAim;

    // Fire gating: weapon must be ready; snipers must be scoped first.
    bool ready = !local.weapon || local.weapon->next_primary_attack <= sdk::g_globals.curtime;
    if (local.weapon && local.weapon->is_sniper && c.auto_scope && !local.scoped) {
        res.reason = "scoping"; return res;
    }
    if (!ready)        { res.reason = "not ready"; return res; }
    if (!c.auto_fire)  { res.reason = "ready (manual)"; return res; }

    res.fire = true;
    res.reason = res.safe_point ? "fire (safe)" : "fire";
    return res;
}

} // namespace feat
