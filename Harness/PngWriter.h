#pragma once
// Minimal, dependency-free PNG writer (8-bit RGBA).
// Uses zlib "stored" (uncompressed) deflate blocks so we need no zlib/libpng.
// Good enough for preview screenshots; not size-optimized.

#include <cstdint>
#include <cstdio>
#include <vector>
#include <string>

namespace png {

inline uint32_t crc32(const uint8_t* data, size_t len, uint32_t crc = 0xffffffffu)
{
    static uint32_t table[256];
    static bool built = false;
    if (!built) {
        for (uint32_t n = 0; n < 256; ++n) {
            uint32_t c = n;
            for (int k = 0; k < 8; ++k)
                c = (c & 1) ? (0xedb88320u ^ (c >> 1)) : (c >> 1);
            table[n] = c;
        }
        built = true;
    }
    for (size_t i = 0; i < len; ++i)
        crc = table[(crc ^ data[i]) & 0xff] ^ (crc >> 8);
    return crc;
}

inline void put_u32(std::vector<uint8_t>& v, uint32_t x)
{
    v.push_back((x >> 24) & 0xff);
    v.push_back((x >> 16) & 0xff);
    v.push_back((x >> 8) & 0xff);
    v.push_back(x & 0xff);
}

inline void chunk(std::vector<uint8_t>& out, const char* type, const std::vector<uint8_t>& data)
{
    put_u32(out, (uint32_t)data.size());
    std::vector<uint8_t> td;
    td.insert(td.end(), type, type + 4);
    td.insert(td.end(), data.begin(), data.end());
    out.insert(out.end(), td.begin(), td.end());
    uint32_t c = crc32(td.data(), td.size()) ^ 0xffffffffu;
    put_u32(out, c);
}

// raw = filtered scanlines (each row: 1 filter byte + RGBA pixels)
inline std::vector<uint8_t> zlib_stored(const std::vector<uint8_t>& raw)
{
    std::vector<uint8_t> z;
    z.push_back(0x78); z.push_back(0x01); // zlib header
    size_t pos = 0;
    while (pos < raw.size()) {
        size_t block = std::min<size_t>(65535, raw.size() - pos);
        bool final = (pos + block) >= raw.size();
        z.push_back(final ? 1 : 0);
        z.push_back(block & 0xff);
        z.push_back((block >> 8) & 0xff);
        uint16_t nlen = ~(uint16_t)block;
        z.push_back(nlen & 0xff);
        z.push_back((nlen >> 8) & 0xff);
        z.insert(z.end(), raw.begin() + pos, raw.begin() + pos + block);
        pos += block;
    }
    // adler32 over raw
    uint32_t a = 1, b = 0;
    for (uint8_t byte : raw) { a = (a + byte) % 65521; b = (b + a) % 65521; }
    uint32_t adler = (b << 16) | a;
    put_u32(z, adler);
    return z;
}

inline bool write_rgba(const std::string& path, int w, int h, const uint8_t* rgba)
{
    std::vector<uint8_t> raw;
    raw.reserve((size_t)h * (1 + w * 4));
    for (int y = 0; y < h; ++y) {
        raw.push_back(0); // filter: none
        raw.insert(raw.end(), rgba + (size_t)y * w * 4, rgba + (size_t)(y + 1) * w * 4);
    }

    std::vector<uint8_t> out;
    const uint8_t sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    out.insert(out.end(), sig, sig + 8);

    std::vector<uint8_t> ihdr;
    put_u32(ihdr, (uint32_t)w);
    put_u32(ihdr, (uint32_t)h);
    ihdr.push_back(8);  // bit depth
    ihdr.push_back(6);  // color type RGBA
    ihdr.push_back(0); ihdr.push_back(0); ihdr.push_back(0);
    chunk(out, "IHDR", ihdr);

    chunk(out, "IDAT", zlib_stored(raw));
    chunk(out, "IEND", {});

    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;
    fwrite(out.data(), 1, out.size(), f);
    fclose(f);
    return true;
}

} // namespace png
