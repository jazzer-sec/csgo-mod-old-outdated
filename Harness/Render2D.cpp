#include "Render2D.h"
#include "PngWriter.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cmath>

// Antialiased text via FreeType, using DejaVu Sans as a Verdana stand-in
// (Verdana is proprietary / not present; DejaVu is the closest free analog —
// same screen-tuned humanist sans, tall x-height). This is what gamesense and
// ArcticTech do (FreeType-rasterized Verdana 11); in the mod the embedded
// Verdana atlas slots in here unchanged behind the same Text() API.
namespace {

struct FontFace {
    FT_Library lib = nullptr;
    FT_Face face = nullptr;
    bool ok = false;
    FontFace() {
        if (FT_Init_FreeType(&lib)) return;
        // Probe common font locations: Linux (DejaVu), macOS (system fonts via Homebrew or built-in)
        static const char* candidates[] = {
            // Linux
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/dejavu/DejaVuSans.ttf",
            // macOS Homebrew font-dejavu
            "/opt/homebrew/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/local/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            // macOS system (Helvetica — close enough for preview)
            "/System/Library/Fonts/Helvetica.ttc",
            "/System/Library/Fonts/HelveticaNeue.ttc",
            // macOS Arial (widely present)
            "/Library/Fonts/Arial.ttf",
            "/System/Library/Fonts/Supplemental/Arial.ttf",
        };
        for (const char* path : candidates)
            if (!FT_New_Face(lib, path, 0, &face)) { ok = true; return; }
    }
};

FontFace& font() { static FontFace f; return f; }

// "scale" maps to a pixel size so existing call sites keep working:
// 1 = body (Verdana-11-ish), 2 = heading, 3 = wordmark.
int pxForScale(int scale) { return scale <= 1 ? 11 : (scale == 2 ? 16 : 22); }

// Decode a UTF-8 string into Unicode codepoints so we can render non-ASCII
// (arrows, Cyrillic, etc.) through FreeType's Unicode charmap.
std::vector<uint32_t> utf8_decode(const std::string& s) {
    std::vector<uint32_t> out;
    size_t i = 0, n = s.size();
    while (i < n) {
        unsigned char c = (unsigned char)s[i];
        uint32_t cp; int extra;
        if (c < 0x80)            { cp = c;        extra = 0; }
        else if ((c >> 5) == 0x6){ cp = c & 0x1F; extra = 1; }
        else if ((c >> 4) == 0xE){ cp = c & 0x0F; extra = 2; }
        else if ((c >> 3) == 0x1E){cp = c & 0x07; extra = 3; }
        else                     { cp = 0xFFFD;   extra = 0; }
        ++i;
        for (int k = 0; k < extra && i < n; ++k, ++i)
            cp = (cp << 6) | ((unsigned char)s[i] & 0x3F);
        out.push_back(cp);
    }
    return out;
}

} // namespace

// Legacy proportional 5x7 pixel font kept for reference / fallback only.
struct Glyph { unsigned char w; unsigned char rows[7]; };

[[maybe_unused]] static const Glyph kFont[95] = {
    /* ' ' */ {2,{0,0,0,0,0,0,0}},
    /* '!' */ {1,{1,1,1,1,1,0,1}},
    /* '"' */ {3,{0b101,0b101,0,0,0,0,0}},
    /* '#' */ {5,{0,0b01010,0b11111,0b01010,0b11111,0b01010,0}},
    /* '$' */ {5,{0b00100,0b01111,0b10100,0b01110,0b00101,0b11110,0b00100}},
    /* '%' */ {5,{0b00011,0b10011,0b00100,0b01000,0b10010,0b11000,0}},
    /* '&' */ {5,{0b00110,0b01001,0b00110,0b01101,0b10010,0b10010,0b01101}},
    /* '\''*/ {1,{1,1,0,0,0,0,0}},
    /* '(' */ {2,{0b10,0b01,0b01,0b01,0b01,0b01,0b10}},
    /* ')' */ {2,{0b01,0b10,0b10,0b10,0b10,0b10,0b01}},
    /* '*' */ {3,{0,0b101,0b010,0b101,0,0,0}},
    /* '+' */ {3,{0,0b010,0b111,0b010,0,0,0}},
    /* ',' */ {2,{0,0,0,0,0,0b10,0b01}},
    /* '-' */ {3,{0,0,0,0b111,0,0,0}},
    /* '.' */ {1,{0,0,0,0,0,0,1}},
    /* '/' */ {3,{0b100,0b100,0b010,0b010,0b010,0b001,0b001}},
    /* '0' */ {4,{6,9,9,9,9,9,6}},
    /* '1' */ {4,{4,6,4,4,4,4,14}},
    /* '2' */ {4,{6,9,8,4,2,1,15}},
    /* '3' */ {4,{6,9,8,6,8,9,6}},
    /* '4' */ {4,{8,12,10,9,15,8,8}},
    /* '5' */ {4,{15,1,7,8,8,9,6}},
    /* '6' */ {4,{6,1,1,7,9,9,6}},
    /* '7' */ {4,{15,8,4,4,2,2,2}},
    /* '8' */ {4,{6,9,9,6,9,9,6}},
    /* '9' */ {4,{6,9,9,14,8,8,6}},
    /* ':' */ {1,{0,0,1,0,0,1,0}},
    /* ';' */ {2,{0,0,0b01,0,0,0b10,0b01}},
    /* '<' */ {3,{0,0b100,0b010,0b001,0b010,0b100,0}},
    /* '=' */ {3,{0,0,0b111,0,0b111,0,0}},
    /* '>' */ {3,{0,0b001,0b010,0b100,0b010,0b001,0}},
    /* '?' */ {4,{6,9,8,4,4,0,4}},
    /* '@' */ {5,{14,17,21,29,13,1,14}},
    /* 'A' */ {4,{6,9,9,15,9,9,9}},
    /* 'B' */ {4,{7,9,9,7,9,9,7}},
    /* 'C' */ {4,{6,9,1,1,1,9,6}},
    /* 'D' */ {4,{7,9,9,9,9,9,7}},
    /* 'E' */ {4,{15,1,1,7,1,1,15}},
    /* 'F' */ {4,{15,1,1,7,1,1,1}},
    /* 'G' */ {4,{6,9,1,13,9,9,14}},
    /* 'H' */ {4,{9,9,9,15,9,9,9}},
    /* 'I' */ {3,{7,2,2,2,2,2,7}},
    /* 'J' */ {4,{8,8,8,8,9,9,6}},
    /* 'K' */ {4,{9,5,3,3,3,5,9}},
    /* 'L' */ {4,{1,1,1,1,1,1,15}},
    /* 'M' */ {5,{17,27,21,21,17,17,17}},
    /* 'N' */ {5,{17,19,21,21,25,17,17}},
    /* 'O' */ {4,{6,9,9,9,9,9,6}},
    /* 'P' */ {4,{7,9,9,7,1,1,1}},
    /* 'Q' */ {4,{6,9,9,9,5,9,10}},
    /* 'R' */ {4,{7,9,9,7,5,9,9}},
    /* 'S' */ {4,{14,1,1,6,8,8,7}},
    /* 'T' */ {3,{7,2,2,2,2,2,2}},
    /* 'U' */ {4,{9,9,9,9,9,9,6}},
    /* 'V' */ {4,{9,9,9,9,9,6,6}},
    /* 'W' */ {5,{17,17,17,21,21,21,10}},
    /* 'X' */ {4,{9,9,6,6,6,9,9}},
    /* 'Y' */ {3,{5,5,5,2,2,2,2}},
    /* 'Z' */ {4,{15,8,4,2,2,1,15}},
    /* '[' */ {2,{0b11,0b01,0b01,0b01,0b01,0b01,0b11}},
    /* '\\'*/ {3,{0b001,0b001,0b010,0b010,0b010,0b100,0b100}},
    /* ']' */ {2,{0b11,0b10,0b10,0b10,0b10,0b10,0b11}},
    /* '^' */ {3,{0b010,0b101,0,0,0,0,0}},
    /* '_' */ {4,{0,0,0,0,0,0,15}},
    /* '`' */ {2,{0b01,0b10,0,0,0,0,0}},
    /* 'a' */ {4,{0,0,6,8,14,9,14}},
    /* 'b' */ {4,{1,1,7,9,9,9,7}},
    /* 'c' */ {4,{0,0,6,9,1,9,6}},
    /* 'd' */ {4,{8,8,14,9,9,9,14}},
    /* 'e' */ {4,{0,0,6,9,15,1,6}},
    /* 'f' */ {3,{6,1,7,1,1,1,1}},
    /* 'g' */ {4,{0,14,9,9,14,8,6}},
    /* 'h' */ {4,{1,1,7,9,9,9,9}},
    /* 'i' */ {1,{1,0,1,1,1,1,1}},
    /* 'j' */ {2,{0b10,0,0b10,0b10,0b10,0b10,0b01}},
    /* 'k' */ {4,{1,1,5,3,3,5,9}},
    /* 'l' */ {1,{1,1,1,1,1,1,1}},
    /* 'm' */ {5,{0,0,11,21,21,17,17}},
    /* 'n' */ {4,{0,0,7,9,9,9,9}},
    /* 'o' */ {4,{0,0,6,9,9,9,6}},
    /* 'p' */ {4,{0,7,9,9,7,1,1}},
    /* 'q' */ {4,{0,14,9,9,14,8,8}},
    /* 'r' */ {3,{0,0,5,3,1,1,1}},
    /* 's' */ {4,{0,0,14,1,6,8,7}},
    /* 't' */ {3,{1,1,7,1,1,1,6}},
    /* 'u' */ {4,{0,0,9,9,9,9,14}},
    /* 'v' */ {4,{0,0,9,9,9,6,6}},
    /* 'w' */ {5,{0,0,17,21,21,21,10}},
    /* 'x' */ {4,{0,0,9,6,6,6,9}},
    /* 'y' */ {4,{0,9,9,9,14,8,6}},
    /* 'z' */ {4,{0,0,15,4,2,1,15}},
    /* '{' */ {3,{6,2,2,1,2,2,6}},
    /* '|' */ {1,{1,1,1,1,1,1,1}},
    /* '}' */ {3,{3,2,2,4,2,2,3}},
    /* '~' */ {4,{0,0,0,0b00110,0b01001,0,0}},
};

int Render2D::TextWidth(const std::string& s, int scale) const {
    auto& f = font();
    if (!f.ok) return 0;
    FT_Set_Pixel_Sizes(f.face, 0, pxForScale(scale));
    long w = 0;
    for (uint32_t cp : utf8_decode(s)) {
        if (FT_Load_Char(f.face, cp, FT_LOAD_DEFAULT)) continue;
        w += f.face->glyph->advance.x >> 6;
    }
    return (int)w;
}

int Render2D::TextHeight(int scale) const { return pxForScale(scale); }

void Render2D::Text(float x, float y, const std::string& s, Color c, int scale)
{
    auto& f = font();
    if (!f.ok) return;
    int px = pxForScale(scale);
    FT_Set_Pixel_Sizes(f.face, 0, px);
    int ascent = f.face->size->metrics.ascender >> 6;
    float pen = x;
    float baseline = y + ascent - 1; // ~top-aligned to y
    for (uint32_t cp : utf8_decode(s)) {
        if (FT_Load_Char(f.face, cp, FT_LOAD_RENDER)) continue;
        FT_GlyphSlot g = f.face->glyph;
        const FT_Bitmap& bm = g->bitmap;
        int gx = (int)std::lround(pen) + g->bitmap_left;
        int gy = (int)std::lround(baseline) - g->bitmap_top;
        for (unsigned row = 0; row < bm.rows; ++row)
            for (unsigned col = 0; col < bm.width; ++col) {
                unsigned char a = bm.buffer[row * bm.pitch + col];
                if (a) Blend(gx + col, gy + row, c, a / 255.f);
            }
        pen += g->advance.x >> 6;
    }
}

bool Render2D::savePNG(const std::string& path) const
{
    return png::write_rgba(path, W, H, px.data());
}
