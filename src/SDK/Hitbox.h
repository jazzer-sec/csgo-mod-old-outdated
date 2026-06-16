#pragma once
#include "Sdk.h"
#include <array>
#include <cstdint>
#include <vector>

// Hitbox / damage model — the data the ragebot reasons over, modeled on
// ArcticTech's autowall + hitscan path (Features/Aimbot, Features/Misc/Autowall).
// In the real client the hitbox positions come from the bone matrix produced by
// the CAnimationSystem fix; here they are a stand-in skeleton so the multipoint /
// damage / hitchance math is real and testable without a game.
namespace sdk {

enum Hitbox {
    HB_HEAD = 0,
    HB_CHEST,
    HB_STOMACH,
    HB_PELVIS,
    HB_ARM_L,
    HB_ARM_R,
    HB_LEG_L,
    HB_LEG_R,
    HB_COUNT
};

// CS:GO hit-group damage multipliers (head 4x, stomach 1.25x, legs 0.75x, ...).
inline float hitgroup_mult(int hb) {
    switch (hb) {
        case HB_HEAD:    return 4.0f;
        case HB_STOMACH: return 1.25f;
        case HB_LEG_L:
        case HB_LEG_R:   return 0.75f;
        default:         return 1.0f;
    }
}

inline bool hitgroup_uses_armor(int hb) {
    return hb != HB_LEG_L && hb != HB_LEG_R; // legs ignore armor in CS:GO
}

// Approximate world position + radius of a hitbox for a standing/ducking player.
// Heights are relative to feet (origin.z). Radius drives multipoint spread and
// the hitchance "did the cone still cover the box" test.
struct HitboxInfo { Vec3 pos; float radius; };

inline HitboxInfo hitbox_info(const Player& p, int hb) {
    float scale = p.ducking ? 0.72f : 1.0f; // crouch lowers the whole skeleton
    auto at = [&](float z, float r) -> HitboxInfo {
        return { Vec3{ p.origin.x, p.origin.y, p.origin.z + z * scale }, r };
    };
    switch (hb) {
        case HB_HEAD:    return at(72.f, 3.4f);
        case HB_CHEST:   return at(60.f, 5.0f);
        case HB_STOMACH: return at(46.f, 5.4f);
        case HB_PELVIS:  return at(38.f, 5.0f);
        case HB_ARM_L:   return at(58.f, 2.6f);
        case HB_ARM_R:   return at(58.f, 2.6f);
        case HB_LEG_L:   return at(20.f, 3.4f);
        case HB_LEG_R:   return at(20.f, 3.4f);
        default:         return at(60.f, 5.0f);
    }
}

// Damage from local's weapon to a point on the given hitgroup, mirroring
// CS:GO's TakeDamage path (range falloff -> hitgroup -> armor). No world trace,
// so this is the "clear LOS" case of ArcticTech's autowall::get_damage.
inline int get_damage(const Player& local, const Player& tgt, int hb,
                      const Vec3& point) {
    const Weapon* w = local.weapon;
    if (!w) return 0;
    float dist = (point - local.eye).length();
    float dmg = (float)w->base_damage;
    dmg *= std::pow(w->range_modifier, dist / 500.f); // falloff
    dmg *= hitgroup_mult(hb);

    if (tgt.armor > 0 && hitgroup_uses_armor(hb)) {
        // Helmet only protects the head; otherwise body armor.
        bool protectedHit = (hb != HB_HEAD) || tgt.has_helmet;
        if (protectedHit) dmg *= 0.775f; // simplified armor ratio
    }

    int cap = tgt.health + tgt.armor; // can't deal more than is there
    return std::max(0, std::min((int)dmg, cap));
}

// Deterministic LCG so hitchance simulations are reproducible in tests
// (stands in for the engine's UniformRandomStream / weapon spread seed).
struct SpreadRng {
    uint32_t s;
    explicit SpreadRng(uint32_t seed) : s(seed ? seed : 0x1234567u) {}
    float next01() {
        s = s * 1664525u + 1013904223u;
        return (s >> 8) / 16777216.f; // 24-bit mantissa in [0,1)
    }
};

// Monte-Carlo hitchance: fire N simulated shots through the weapon cone and
// count how many still land inside the hitbox radius at target distance.
// This is the standalone analog of ArcticTech's seed-replay hitchance.
inline int hitchance(const Player& local, const Player& tgt, int hb,
                     const Vec3& point, int tick, int samples = 128) {
    const Weapon* w = local.weapon;
    if (!w) return 0;
    HitboxInfo hbi = hitbox_info(tgt, hb);
    float dist = (point - local.eye).length();
    if (dist < 1.f) return 100;

    // Moving inflates the cone; standing/scoped tightens it.
    float speed = local.velocity.length2d();
    float cone = w->inaccuracy + speed * 0.00012f;
    if (local.scoped) cone *= 0.15f;
    if (cone <= 1e-5f) return 100;

    SpreadRng rng((uint32_t)(tick * 2654435761u) ^ (uint32_t)tgt.index);
    int hits = 0;
    for (int i = 0; i < samples; ++i) {
        float r = cone * std::sqrt(rng.next01()); // uniform over the disc
        float lateral = r * dist;                 // miss distance at the target
        if (lateral <= hbi.radius) ++hits;
    }
    return (int)std::lround(100.0 * hits / samples);
}

} // namespace sdk
