#pragma once
#include "../Harness/Render2D.h"
#include "../Harness/Theme.h"
#include "../Harness/Anim.h"
#include <cmath>
#include <string>

// Premium on-inject splash for "knicksware": a dark scrim, a basketball mark,
// the wordmark with the accent on "knicks", an underline that draws in, the
// tagline "how bout them knicks?", and a filling loader. Cosmetic preview of
// what the real inject would show. Drawn through Render2D so every backend
// (software / GDI / ImGui) renders it identically.
struct Splash {
    static constexpr float DUR = 3.4f;
    float t = -1.f;                 // < 0 = inactive

    void start() { t = 0.f; }
    bool active() const { return t >= 0.f; }
    void update(float dt) { if (t >= 0.f) { t += dt; if (t > DUR) t = -1.f; } }

    static float seg(float x, float a, float b) {       // 0..1 ramp over [a,b]
        if (b <= a) return x >= b ? 1.f : 0.f;
        return std::clamp((x - a) / (b - a), 0.f, 1.f);
    }

    void draw(Render2D& r) const {
        if (t < 0.f) return;
        const Theme& th = theme();
        float W = (float)r.width(), H = (float)r.height();
        float cx = W * 0.5f, cy = H * 0.5f;

        float fin  = ease_out_cubic(seg(t, 0.f, 0.4f));
        float fout = 1.f - seg(t, DUR - 0.6f, DUR);
        float A = fin * fout;                            // overall opacity

        // dark scrim + soft accent glow behind the wordmark
        r.BoxFilled(0, 0, W, H, Color(8, 9, 11).withAf(0.96f * A));
        r.GlowCircle(cx, cy - 6, 250, th.accent.withA(26).withAf(A));

        // basketball mark (ironic Knicks nod)
        float bIn = ease_out_cubic(seg(t, 0.25f, 0.95f));
        float br = 17.f * bIn;
        float bx = cx, by = cy - 72;
        if (br > 1.f) {
            r.CircleFilled(bx, by, br, Color(214, 124, 54).withAf(A)); // orange
            Color seam(18, 20, 22, 230);
            r.Circle(bx, by, br, 1.5f, seam.withAf(A));
            r.Line(bx - br, by, bx + br, by, seam.withAf(A), 1.4f);
            r.Line(bx, by - br, bx, by + br, seam.withAf(A), 1.4f);
        }

        // wordmark "knicks" + "ware" (scale 4 / 38px), accent on "knicks"
        float wIn = ease_out_cubic(seg(t, 0.35f, 1.15f));
        float wy = cy - 36 + (1.f - wIn) * 10.f;        // slide up as it fades in
        std::string a = "knicks", b = "ware";
        int aw = r.TextWidth(a, 4), bw = r.TextWidth(b, 4);
        float total = (float)(aw + bw);
        float wx = cx - total * 0.5f;
        r.Text(wx,      wy, a, th.accent.withAf(A * wIn), 4);
        r.Text(wx + aw, wy, b, th.text.withAf(A * wIn),   4);

        // accent underline drawing in left -> right
        float uIn = ease_out_cubic(seg(t, 0.7f, 1.55f));
        float uw = total + 14.f;
        float uy = wy + 46;
        r.RoundedBox(cx - uw * 0.5f, uy, uw * uIn, 2.f, 1.f, th.accent.withAf(A));

        // tagline
        float tIn = ease_out_cubic(seg(t, 1.05f, 1.8f));
        r.TextCentered(cx, uy + 12, "how bout them knicks?", th.textDim.withAf(A * tIn), 2);

        // loader bar with a shimmer on the leading edge
        float barW = 230.f, barH = 3.f;
        float lx = cx - barW * 0.5f, ly = cy + 78;
        r.RoundedBox(lx, ly, barW, barH, 1.5f, th.field.withAf(A * 0.9f));
        float prog = ease_in_out(seg(t, 0.5f, DUR - 0.7f));
        r.RoundedBox(lx, ly, barW * prog, barH, 1.5f, th.accent.withAf(A));
        if (prog > 0.02f && prog < 0.99f)
            r.GlowCircle(lx + barW * prog, ly + barH * 0.5f, 9, th.accent2.withA(140).withAf(A));

        r.TextCentered(cx, ly + 16,
                       std::string("loading  ") + std::to_string((int)(prog * 100)) + "%",
                       th.textFaint.withAf(A), 1);
    }
};
