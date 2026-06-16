#pragma once
#include "../Harness/Render2D.h"
#include "../Harness/Theme.h"
#include "../Harness/Anim.h"
#include <cmath>
#include <string>

// Premium on-inject splash for "knicksware", as a snappy sequence:
//   1. "knicks" pops in (overshoot)
//   2. "ware" snaps out to the right while the wordmark recenters
//   3. a spinning basketball (with motion trail) arcs into a hoop; loader fills
//   4. the made shot bursts (sparks + swish) and scene 1 lifts/fades away
//   5. "how bout them knicks?" blooms open from the center outward, letters
//      revealing from the middle to the sides behind an expanding beam
//   6. tagline animates out
// Cosmetic only (no hooks). Drawn through Render2D so every backend matches.
struct Splash {
    static constexpr float DUR = 5.8f;
    float t = -1.f;                 // < 0 = inactive

    void start() { t = 0.f; }
    bool active() const { return t >= 0.f; }
    void update(float dt) { if (t >= 0.f) { t += dt; if (t > DUR) t = -1.f; } }

    static float seg(float x, float a, float b) {
        if (b <= a) return x >= b ? 1.f : 0.f;
        return std::clamp((x - a) / (b - a), 0.f, 1.f);
    }

    // a spinning basketball: orange disc, outline, rotating cross + curved seams
    static void ball(Render2D& r, float bx, float by, float rad, float ang, float a) {
        if (rad < 0.6f) return;
        Color orange(224, 132, 56), seam(18, 20, 22, 235);
        r.CircleFilled(bx, by, rad, orange.withAf(a));
        r.Circle(bx, by, rad, 1.3f, seam.withAf(a));
        float ca = std::cos(ang), sa = std::sin(ang);
        auto rot = [&](float x, float y, float& ox, float& oy) {
            ox = bx + x * ca - y * sa; oy = by + x * sa + y * ca;
        };
        // two diameters (the cross seams)
        float x0, y0, x1, y1;
        rot(-rad, 0, x0, y0); rot(rad, 0, x1, y1);
        r.Line(x0, y0, x1, y1, seam.withAf(a), 1.2f);
        rot(0, -rad, x0, y0); rot(0, rad, x1, y1);
        r.Line(x0, y0, x1, y1, seam.withAf(a), 1.2f);
        // two curved vertical seams bulging left/right
        for (int s = -1; s <= 1; s += 2) {
            float px = 0, py = 0; bool have = false;
            for (int k = 0; k <= 8; ++k) {
                float u = k / 8.f;
                float lx = (float)s * std::sin(u * 3.14159f) * rad * 0.62f;
                float ly = (u - 0.5f) * 2.f * rad;
                float ox, oy; rot(lx, ly, ox, oy);
                if (have) r.Line(px, py, ox, oy, seam.withAf(a), 1.f);
                px = ox; py = oy; have = true;
            }
        }
    }

    void draw(Render2D& r) const {
        if (t < 0.f) return;
        const Theme& th = theme();
        const float W = (float)r.width(), H = (float)r.height();
        const float cx = W * 0.5f, cy = H * 0.5f;

        // ---- snappy timeline ----
        const float scrimIn  = ease_out_cubic(seg(t, 0.0f, 0.35f));
        const float scrimOut = 1.f - seg(t, DUR - 0.45f, DUR);
        const float A = scrimIn * scrimOut;

        const float knIn  = ease_out_back(seg(t, 0.20f, 0.70f));   // "knicks" pop
        const float rec   = ease_out_back(seg(t, 0.70f, 1.20f));   // "ware" snap + recenter
        const float undr  = ease_out_cubic(seg(t, 1.10f, 1.45f));
        const float load  = ease_in_out(seg(t, 0.7f, 2.4f));
        const float shot  = seg(t, 1.45f, 2.35f);                  // ball travel (linear-ish)
        const float make  = seg(t, 2.30f, 2.75f);                  // swish burst
        const float s1out = ease_out_cubic(seg(t, 2.55f, 3.1f));
        const float tagOut= ease_out_cubic(seg(t, 5.05f, 5.7f));   // scene-2 exit

        // dark scrim + smooth radial glow
        r.BoxFilled(0, 0, W, H, Color(8, 9, 11).withAf(0.96f * A));
        r.GlowCircle(cx, cy - 4, 280, th.accent.withA(28).withAf(A * (1.f - s1out)));

        // ===================== scene 1 =====================
        const float s1A = A * (1.f - s1out);
        if (s1A > 0.002f) {
            const float lift = s1out * 30.f;

            const std::string kn = "knicks", wa = "ware";
            const float wK = (float)r.TextWidth(kn, 4), wW = (float)r.TextWidth(wa, 4);
            const float total = wK + wW;
            const float knSolo = cx - wK * 0.5f, knFinal = cx - total * 0.5f;
            const float knX = knSolo + (knFinal - knSolo) * std::clamp(rec, 0.f, 1.f);
            const float waX = knX + wK + (1.f - std::clamp(rec, 0.f, 1.f)) * 26.f;
            const float wy = cy - 30 - lift + (1.f - std::clamp(knIn, 0.f, 1.f)) * 14.f;

            r.Text(knX, wy, kn, th.accent.withAf(s1A * std::clamp(knIn, 0.f, 1.f)), 4);
            r.Text(waX, wy, wa, th.text.withAf(s1A * std::clamp(rec, 0.f, 1.f)), 4);

            const float uw = (total + 16.f) * undr;
            const float uy = wy + 46;
            r.RoundedBox(cx - uw * 0.5f, uy, uw, 2.f, 1.f, th.accent.withAf(s1A));

            // ---- hoop above the wordmark ----
            const float hx = cx, hy = cy - 116 - lift, rimW = 34.f;
            r.RoundedBox(hx - 26, hy - 34, 52, 30, 3, th.field.withAf(s1A * 0.9f));
            r.RoundedBoxOutline(hx - 26, hy - 34, 52, 30, 3, 1.f, th.line.withAf(s1A));
            r.RoundedBoxOutline(hx - 9, hy - 26, 18, 13, 2, 1.f, th.accent.withAf(s1A * 0.8f));
            Color rim(224, 132, 56);
            r.Line(hx - rimW * 0.5f, hy, hx + rimW * 0.5f, hy, rim.withAf(s1A), 2.4f);
            // net (swishes on the make)
            float swish = std::sin(make * 3.14159f) * 4.f;
            for (int i = 0; i <= 4; ++i) {
                float fx = hx - rimW * 0.5f + rimW * (i / 4.f);
                float nx = hx - (hx - fx) * 0.45f + swish;
                r.Line(fx, hy, nx, hy + 15, th.textDim.withAf(s1A * 0.7f), 1.f);
            }

            // ---- ball arc (with spin + motion trail) ----
            auto ballPos = [&](float u, float& bx, float& by) {
                float x0 = hx - 165, y0 = cy - 30;
                bx = x0 + (hx - x0) * u;
                by = y0 + (hy - y0) * u - 78.f * 4.f * u * (1.f - u);
            };
            if (s1out <= 0.f && shot > 0.f) {
                // trail
                for (int i = 5; i >= 1; --i) {
                    float u = shot - i * 0.045f;
                    if (u <= 0.f) continue;
                    float gx, gy; ballPos(u, gx, gy);
                    ball(r, gx, gy, 9.f * (1.f - i * 0.12f), shot * 9.f, s1A * (0.10f * (6 - i)));
                }
                float bx, by; ballPos(shot, bx, by);
                ball(r, bx, by, 9.f, shot * 9.f, s1A);
            } else if (s1out > 0.f) {
                ball(r, hx, hy + s1out * 240.f, 9.f, s1out * 6.f, s1A); // made -> drops through
            }

            // ---- swish burst at the rim ----
            if (make > 0.f && make < 1.f) {
                float k = make;
                r.GlowCircle(hx, hy, 14 + 34 * k, rim.withA((int)(180 * (1.f - k))).withAf(s1A));
                for (int i = 0; i < 8; ++i) {
                    float a = i / 8.f * 6.2832f;
                    float r0 = 8 + 26 * k, r1 = r0 + 9;
                    Color sp = (i % 2 ? th.accent : rim);
                    r.Line(hx + std::cos(a) * r0, hy + std::sin(a) * r0,
                           hx + std::cos(a) * r1, hy + std::sin(a) * r1,
                           sp.withA((int)(220 * (1.f - k))).withAf(s1A), 1.6f);
                }
            }

            // ---- loader ----
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

        // ===================== scene 2: comedic setup -> punchline ==================
        // Understated setup slides in, then the punchline "Knicks" slams in big,
        // then the cheeky "?" bounces in last. Deadpan timing = the joke.
        const float tagA = A * (1.f - tagOut);
        const float setup = seg(t, 3.05f, 3.7f);    // "How 'bout them"
        const float punch = seg(t, 3.7f, 4.15f);    // "Knicks" slam
        const float qmark = seg(t, 4.15f, 4.55f);   // "?" bounce
        if (setup > 0.f && tagA > 0.002f) {
            const std::string A1 = "How 'bout them ";   // setup (scale 2, dim)
            const std::string B1 = "Knicks";            // punchline (scale 4, accent)
            const std::string C1 = "?";                 // the cheeky beat (scale 4)
            const float h2 = (float)r.TextHeight(2), h4 = (float)r.TextHeight(4);
            const float wA = (float)r.TextWidth(A1, 2);
            const float wB = (float)r.TextWidth(B1, 4);
            const float wC = (float)r.TextWidth(C1, 4);
            const float total = wA + wB + wC;
            const float x0 = cx - total * 0.5f;
            const float midY = cy;
            const float fade = 1.f - tagOut;

            // setup: small, dim, slides in from the left and settles
            float sE = ease_out_cubic(setup);
            float sx = x0 - (1.f - sE) * 16.f;
            r.Text(sx, midY - h2 * 0.5f, A1, th.textDim.withAf(tagA * sE), 2);

            // punchline bloom + underline keyed to "Knicks?"
            float bx = x0 + wA;                          // left of "Knicks"
            float punchCx = bx + (wB + wC) * 0.5f;
            float pE = ease_out_back(punch);
            if (punch > 0.f) {
                float flash = std::max(0.f, 1.f - seg(t, 3.7f, 4.05f) * 1.4f); // entry pop
                r.GlowCircle(punchCx, midY, 120.f * fade,
                             th.accent.withA(40).withAf(A * fade * (0.5f + 0.5f * pE)));
                r.GlowCircle(punchCx, midY, 60.f,
                             th.accent2.withA(150).withAf(A * fade * flash));
                // punchline underline grows under "Knicks?"
                float uw = (wB + wC + 10.f) * std::clamp(pE, 0.f, 1.f);
                r.RoundedBox(punchCx - uw * 0.5f, midY + h4 * 0.5f + 4.f, uw, 2.f, 1.f,
                             th.accent.withAf(tagA));
            }

            // "Knicks": slams down from above with an overshoot
            float pYoff = (1.f - pE) * -22.f;            // from above; back-ease overshoots
            r.Text(bx, midY - h4 * 0.5f + pYoff, B1, th.accent.withAf(tagA * std::clamp(punch * 3.f, 0.f, 1.f)), 4);

            // "?": bounces in last, a beat behind — the deadpan kicker
            float qE = ease_out_back(qmark);
            float qYoff = (1.f - qE) * -26.f;
            r.Text(bx + wB, midY - h4 * 0.5f + qYoff, C1,
                   th.accent.withAf(tagA * std::clamp(qmark * 3.f, 0.f, 1.f)), 4);
        }
    }
};
