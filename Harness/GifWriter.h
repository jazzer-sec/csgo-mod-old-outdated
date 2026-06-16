#pragma once
// Minimal, dependency-free animated GIF writer.
// Builds a single global 256-color palette across all frames (median-cut),
// quantizes via a 32x32x32 nearest-color LUT with ordered dithering, then
// LZW-compresses each frame. Loops forever (Netscape extension).
//
// Used by the harness to export the menu/HUD animation as a GIF that opens in
// any browser with no installs — the no-dependency cousin of the PNG writer.

#include <cstdint>
#include <cstdio>
#include <vector>
#include <string>
#include <algorithm>

namespace gif {

struct RGB { uint8_t r, g, b; };

// ---- median-cut palette over sampled colors ----
struct Box {
    int lo, hi;        // index range into the sample array
    uint8_t rmin, rmax, gmin, gmax, bmin, bmax;
};

inline void shrink(Box& bx, const std::vector<RGB>& s) {
    bx.rmin = bx.gmin = bx.bmin = 255;
    bx.rmax = bx.gmax = bx.bmax = 0;
    for (int i = bx.lo; i < bx.hi; ++i) {
        const RGB& c = s[i];
        bx.rmin = std::min(bx.rmin, c.r); bx.rmax = std::max(bx.rmax, c.r);
        bx.gmin = std::min(bx.gmin, c.g); bx.gmax = std::max(bx.gmax, c.g);
        bx.bmin = std::min(bx.bmin, c.b); bx.bmax = std::max(bx.bmax, c.b);
    }
}

inline std::vector<RGB> median_cut(std::vector<RGB> samples, int want) {
    std::vector<RGB> palette;
    if (samples.empty()) { palette.push_back({0, 0, 0}); return palette; }
    std::vector<Box> boxes;
    Box first{0, (int)samples.size(), 0, 0, 0, 0, 0, 0};
    shrink(first, samples);
    boxes.push_back(first);

    while ((int)boxes.size() < want) {
        // pick the box with the largest channel extent
        int pick = -1, bestRange = -1;
        for (int i = 0; i < (int)boxes.size(); ++i) {
            const Box& b = boxes[i];
            if (b.hi - b.lo < 2) continue;
            int rr = b.rmax - b.rmin, gr = b.gmax - b.gmin, br = b.bmax - b.bmin;
            int m = std::max({rr, gr, br});
            if (m > bestRange) { bestRange = m; pick = i; }
        }
        if (pick < 0) break;
        Box b = boxes[pick];
        int rr = b.rmax - b.rmin, gr = b.gmax - b.gmin, br = b.bmax - b.bmin;
        int ch = (rr >= gr && rr >= br) ? 0 : (gr >= br ? 1 : 2);
        auto cmp = [ch](const RGB& a, const RGB& c) {
            return ch == 0 ? a.r < c.r : ch == 1 ? a.g < c.g : a.b < c.b;
        };
        std::sort(samples.begin() + b.lo, samples.begin() + b.hi, cmp);
        int mid = (b.lo + b.hi) / 2;
        Box left{b.lo, mid, 0, 0, 0, 0, 0, 0}, right{mid, b.hi, 0, 0, 0, 0, 0, 0};
        shrink(left, samples); shrink(right, samples);
        boxes[pick] = left;
        boxes.push_back(right);
    }

    for (const Box& b : boxes) {
        long r = 0, g = 0, bl = 0; int n = b.hi - b.lo;
        for (int i = b.lo; i < b.hi; ++i) { r += samples[i].r; g += samples[i].g; bl += samples[i].b; }
        if (n <= 0) { palette.push_back({0, 0, 0}); continue; }
        palette.push_back({(uint8_t)(r / n), (uint8_t)(g / n), (uint8_t)(bl / n)});
    }
    while ((int)palette.size() < 256) palette.push_back(palette.back());
    return palette;
}

// 32^3 nearest-color LUT for O(1) per-pixel quantization.
inline std::vector<uint8_t> build_lut(const std::vector<RGB>& pal) {
    std::vector<uint8_t> lut(32 * 32 * 32);
    for (int r = 0; r < 32; ++r)
        for (int g = 0; g < 32; ++g)
            for (int b = 0; b < 32; ++b) {
                int R = (r << 3) | 4, G = (g << 3) | 4, B = (b << 3) | 4;
                int best = 0; long bestd = 1L << 60;
                for (int i = 0; i < (int)pal.size(); ++i) {
                    long dr = R - pal[i].r, dg = G - pal[i].g, db = B - pal[i].b;
                    long d = dr * dr + dg * dg + db * db;
                    if (d < bestd) { bestd = d; best = i; }
                }
                lut[(r << 10) | (g << 5) | b] = (uint8_t)best;
            }
    return lut;
}

// ---- LZW (GIF variable-length, LSB-first) ----
struct BitWriter {
    std::vector<uint8_t> out;
    uint32_t acc = 0; int nbits = 0;
    void put(int code, int size) {
        acc |= (uint32_t)code << nbits;
        nbits += size;
        while (nbits >= 8) { out.push_back(acc & 0xff); acc >>= 8; nbits -= 8; }
    }
    void flush() { if (nbits > 0) { out.push_back(acc & 0xff); acc = 0; nbits = 0; } }
};

inline std::vector<uint8_t> lzw_compress(const std::vector<uint8_t>& idx, int minCode) {
    const int clearCode = 1 << minCode;
    const int endCode = clearCode + 1;
    BitWriter bw;
    int codeSize = minCode + 1;
    int next = endCode + 1;

    // dictionary: map from (prefix<<8 | k) -> code
    std::vector<int> dict(1 << 20, -1);
    auto reset = [&]() {
        std::fill(dict.begin(), dict.end(), -1);
        next = endCode + 1;
        codeSize = minCode + 1;
    };

    bw.put(clearCode, codeSize);
    if (idx.empty()) { bw.put(endCode, codeSize); bw.flush(); return bw.out; }

    int prefix = idx[0];
    for (size_t i = 1; i < idx.size(); ++i) {
        int k = idx[i];
        int key = (prefix << 8) | k;
        if (key < (int)dict.size() && dict[key] >= 0) {
            prefix = dict[key];
        } else {
            bw.put(prefix, codeSize);
            if (next < 4096) {
                if (key < (int)dict.size()) dict[key] = next++;
                if (next > (1 << codeSize) && codeSize < 12) ++codeSize;
            } else {
                bw.put(clearCode, codeSize);
                reset();
            }
            prefix = k;
        }
    }
    bw.put(prefix, codeSize);
    bw.put(endCode, codeSize);
    bw.flush();
    return bw.out;
}

inline void put_subblocks(std::vector<uint8_t>& out, const std::vector<uint8_t>& data) {
    size_t pos = 0;
    while (pos < data.size()) {
        size_t n = std::min<size_t>(255, data.size() - pos);
        out.push_back((uint8_t)n);
        out.insert(out.end(), data.begin() + pos, data.begin() + pos + n);
        pos += n;
    }
    out.push_back(0); // block terminator
}

// 4x4 Bayer matrix (0..15), used as a small dither to soften gradient banding.
inline int bayer(int x, int y) {
    static const int m[16] = {0,8,2,10, 12,4,14,6, 3,11,1,9, 15,7,13,5};
    return m[(y & 3) * 4 + (x & 3)];
}

// frames: each is w*h*4 RGBA. delay_cs is per-frame delay in centiseconds.
inline bool write_animation(const std::string& path, int w, int h,
                            const std::vector<std::vector<uint8_t>>& frames,
                            int delay_cs) {
    if (frames.empty()) return false;

    // sample colors across all frames for the global palette
    std::vector<RGB> samples;
    samples.reserve(200000);
    int stride = std::max(1, (int)((long)w * h * frames.size() / 180000));
    for (const auto& f : frames)
        for (size_t i = 0; i < (size_t)w * h; i += stride)
            samples.push_back({f[i * 4], f[i * 4 + 1], f[i * 4 + 2]});

    std::vector<RGB> pal = median_cut(std::move(samples), 256);
    std::vector<uint8_t> lut = build_lut(pal);

    std::vector<uint8_t> out;
    auto w16 = [&](int x) { out.push_back(x & 0xff); out.push_back((x >> 8) & 0xff); };

    // header + logical screen descriptor
    const char* sig = "GIF89a";
    out.insert(out.end(), sig, sig + 6);
    w16(w); w16(h);
    out.push_back(0xF7);   // GCT present, 256 entries
    out.push_back(0);      // background index
    out.push_back(0);      // aspect ratio
    for (int i = 0; i < 256; ++i) {
        out.push_back(pal[i].r); out.push_back(pal[i].g); out.push_back(pal[i].b);
    }

    // Netscape loop extension (loop forever)
    const uint8_t nsapp[] = {0x21, 0xFF, 0x0B, 'N','E','T','S','C','A','P','E','2','.','0',
                             0x03, 0x01, 0x00, 0x00, 0x00};
    out.insert(out.end(), nsapp, nsapp + sizeof(nsapp));

    std::vector<uint8_t> idx((size_t)w * h);
    for (const auto& f : frames) {
        for (size_t i = 0; i < (size_t)w * h; ++i) {
            int x = (int)(i % w), y = (int)(i / w);
            int t = bayer(x, y) - 8; // -8..+7
            int R = std::clamp((int)f[i * 4]     + t, 0, 255);
            int G = std::clamp((int)f[i * 4 + 1] + t, 0, 255);
            int B = std::clamp((int)f[i * 4 + 2] + t, 0, 255);
            idx[i] = lut[((R >> 3) << 10) | ((G >> 3) << 5) | (B >> 3)];
        }

        // graphics control extension (delay)
        out.push_back(0x21); out.push_back(0xF9); out.push_back(0x04);
        out.push_back(0x04); // disposal = do not dispose, no transparency
        w16(delay_cs);
        out.push_back(0x00); // transparent index (unused)
        out.push_back(0x00);

        // image descriptor
        out.push_back(0x2C);
        w16(0); w16(0); w16(w); w16(h);
        out.push_back(0x00); // no local color table

        out.push_back(0x08); // LZW minimum code size
        std::vector<uint8_t> comp = lzw_compress(idx, 8);
        put_subblocks(out, comp);
    }

    out.push_back(0x3B); // trailer

    FILE* fp = fopen(path.c_str(), "wb");
    if (!fp) return false;
    fwrite(out.data(), 1, out.size(), fp);
    fclose(fp);
    return true;
}

} // namespace gif
