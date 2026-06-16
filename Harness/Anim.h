#pragma once
// Anim — framerate-independent eased interpolation. Every UI value that can
// change (menu open factor, tab underline x, hover glow, healthbar fill, …)
// is stored as an Anim and stepped toward its target each frame, so nothing in
// the interface ever snaps. This is the single helper that makes the UI feel
// animated rather than static.

#include <algorithm>
#include <cmath>

inline float ease_out_cubic(float t) {
    t = std::clamp(t, 0.f, 1.f);
    float u = 1.f - t;
    return 1.f - u * u * u;
}

inline float ease_in_out(float t) {
    t = std::clamp(t, 0.f, 1.f);
    return t < 0.5f ? 4.f * t * t * t
                    : 1.f - std::pow(-2.f * t + 2.f, 3.f) / 2.f;
}

struct Anim {
    float cur = 0.f;
    float target = 0.f;
    float speed = 12.f; // higher = snappier

    Anim() = default;
    explicit Anim(float v) : cur(v), target(v) {}

    // Exponential smoothing toward target; stable for any dt.
    void step(float dt) {
        float a = 1.f - std::exp(-speed * dt);
        cur += (target - cur) * a;
    }
    void set(float v) { target = v; }
    void snap(float v) { cur = target = v; }
};
