#pragma once
#include "../Harness/Render2D.h"
#include "../Harness/Theme.h"
#include "../Harness/Anim.h"
#include <cmath>
#include <string>

// Premium on-inject splash for "knicksware", as a sequence:
//   1. "knicks" eases in (fade + slide)
//   2. "ware" slides out to the right while the whole wordmark recenters
//   3. a little basketball arcs into a hoop above the wordmark; loader fills
//   4. scene-1 out: the made shot drops and the wordmark lifts/fades away
//   5. "how bout them knicks?" revealed by a horizontal slash, more prominent
//   6. tagline out
// Cosmetic only (no hooks). Drawn through Render2D so every backend matches.
struct Splash {
    static constexpr float DUR = 5.4f;
    float t = -1.f;                 // < 0 = inactive

    void start() { t = 0.f; }
    bool active() const { return t >= 0.f; }
    void update(float dt) { if (t >= 0.f) { t += dt; if (t > DUR) t = -1.f; } }

    // 0..1 ramp over [a,b]
    static float seg(float x, float a, float b) {
        if (b <= a) return x >= b ? 1.f : 0.f;
        return std::clamp((x - a) / (b - a), 0.f, 1.f);
    }
    static float smooth(float x) { return ease_out_cubic(std::clamp(x, 0.f, 1.f)); }

    void draw(Render2D& r) const {
        if (t < 0.f) return;
        const Theme& th = theme();
        const float W = (float)r.width(), H = (float)r.height();
        const float cx = W * 0.5f, cy = H * 0.5f;

        // ---- timeline ----
        const float scrimIn  = smooth(seg(t, 0.0f, 0.45f));
        const float scrimOut = 1.f - seg(t, DUR - 0.5f, DUR);
        const float A = scrimIn * scrimOut;                 // global opacity

        const float knIn   = smooth(seg(t, 0.30f, 1.05f));  // "knicks" in
        const float rec    = smooth(seg(t, 1.05f, 1.95f));  // "ware" out + recenter
        const float undr   = smooth(seg(t, 1.7f, 2.2f));    // underline
        const float load   = ease_in_out(seg(t, 1.0f, 3.0f)); // loader fill
        const float shot   = smooth(seg(t, 2.0f, 3.0f));    // ball arc into hoop
        const float s1out  = smooth(seg(t, 3.05f, 3.6f));   // scene-1 leaves
        const float tagIn  = smooth(seg(t, 3.5f, 4.2f));    // tagline slash in
        const float tagOut = smooth(seg(t, 4.7f, 5.3f));    // tagline leaves

        // dark scrim + smooth radial glow behind the wordmark
        r.BoxFilled(0, 0, W, H, Color(8, 9, 11).withAf(0.96f * A));
        r.GlowCircle(cx, cy - 4, 280, th.accent.withA(30).withAf(A * (1.f - tagIn * 0.6f)));

        // ===================== scene 1: wordmark + hoop + loader =====================
        const float s1A = A * (1.f - s1out);
        if (s1A > 0.002f) {
            const float lift = s1out * 26.f;               // whole scene lifts on out

            // wordmark layout
            const std::string kn = "knicks", wa = "ware";
            const float wK = (float)r.TextWidth(kn, 4);
            const float wW = (float)r.TextWidth(wa, 4);
            const float total = wK + wW;
            const float knSolo  = cx - wK * 0.5f;           // "knicks" centered alone
            const float knFinal = cx - total * 0.5f;        // in the full wordmark
            const float knX = knSolo + (knFinal - knSolo) * rec;
            const float waSlide = (1.f - rec) * 22.f;       // "ware" slides in from right
            const float waX = knX + wK + waSlide;
            const float wy = cy - 30 - lift + (1.f - knIn) * 12.f;

            r.Text(knX, wy, kn, th.accent.withAf(s1A * knIn), 4);
            r.Text(waX, wy, wa, th.text.withAf(s1A * rec), 4);

            // underline grows from center
            const float uw = (total + 16.f) * undr;
            const float uy = wy + 46;
            r.RoundedBox(cx - uw * 0.5f, uy, uw, 2.f, 1.f, th.accent.withAf(s1A));

            // ---- basketball + hoop above the wordmark ----
            const float hx = cx, hy = cy - 116 - lift;      // rim center
            const float rimW = 34.f, rimH = 9.f;
            // backboard
            r.RoundedBox(hx - 26, hy - 34, 52, 30, 3, th.field.withAf(s1A * 0.9f));
            r.RoundedBoxOutline(hx - 26, hy - 34, 52, 30, 3, 1.f, th.line.withAf(s1A));
            r.RoundedBoxOutline(hx - 9, hy - 26, 18, 13, 2, 1.f, th.accent.withAf(s1A * 0.8f)); // shooter's square
            // rim (orange) + simple net
            Color rim(214, 124, 54);
            r.Line(hx - rimW * 0.5f, hy, hx + rimW * 0.5f, hy, rim.withAf(s1A), 2.4f);
            const float swish = std::sin(std::clamp((shot - 0.7f) * 12.f, 0.f, 3.14159f)) * (1.f - s1out);
            for (int i = 0; i <= 4; ++i) {
                float fx = hx - rimW * 0.5f + rimW * (i / 4.f);
                float nx = hx - (hx - fx) * 0.5f + swish * 3.f;
                r.Line(fx, hy, nx, hy + 14, th.textDim.withAf(s1A * 0.7f), 1.f);
            }

            // ball: lobs from lower-left up and down through the rim, then drops on out
            float u = shot;                                  // 0..1 to the rim
            float bx, by;
            if (s1out <= 0.f) {
                float x0 = hx - 150, y0 = cy - 40;
                bx = x0 + (hx - x0) * u;
                by = y0 + (hy - y0) * u - 70.f * 4.f * u * (1.f - u); // lob arc
            } else {
                bx = hx;                                      // made -> falls straight down
                by = hy + s1out * 220.f;
            }
            float bA = s1A;
            if (shot > 0.f || s1out > 0.f) {
                float br = 9.f;
                r.CircleFilled(bx, by, br, rim.withAf(bA));
                Color seam(18, 20, 22, 220);
                r.Circle(bx, by, br, 1.2f, seam.withAf(bA));
                r.Line(bx - br, by, bx + br, by, seam.withAf(bA), 1.f);
                r.Line(bx, by - br, bx, by + br, seam.withAf(bA), 1.f);
            }

            // loader
            const float barW = 230.f, barH = 3.f;
            const float lx = cx - barW * 0.5f, ly = cy + 70 - lift;
            r.RoundedBox(lx, ly, barW, barH, 1.5f, th.field.withAf(s1A * 0.9f));
            r.RoundedBox(lx, ly, barW * load, barH, 1.5f, th.accent.withAf(s1A));
            if (load > 0.02f && load < 0.99f)
                r.GlowCircle(lx + barW * load, ly + barH * 0.5f, 10, th.accent2.withA(150).withAf(s1A));
            r.TextCentered(cx, ly + 16,
                std::string("loading  ") + std::to_string((int)(load * 100)) + "%",
                th.textFaint.withAf(s1A), 1);
        }

        // ===================== scene 2: tagline (horizontal slash reveal) =============
        const float tagA = A * tagIn * (1.f - tagOut);
        if (tagA > 0.002f) {
            const std::string tag = "how bout them knicks?";
            const float ty = cy - 4;
            // the "slash": a bright accent line sweeps/expands across, carrying the text
            const float reach = (W * 0.36f) * tagIn * (1.f - tagOut);
            const float ly = ty + 20;
            // thin streak with a hot leading edge
            r.RoundedBox(cx - reach, ly, reach * 2.f, 2.f, 1.f, th.accent.withAf(tagA));
            r.GlowCircle(cx + reach, ly + 1, 14, th.accent2.withA(170).withAf(tagA));
            r.GlowCircle(cx - reach, ly + 1, 14, th.accent2.withA(170).withAf(tagA));
            // tagline, prominent (scale 3), riding the reveal + a small settle
            float slide = (1.f - tagIn) * 8.f - tagOut * 10.f;
            r.TextCentered(cx, ty - 12 + slide, tag, th.text.withAf(tagA), 3);
        }
    }
};
