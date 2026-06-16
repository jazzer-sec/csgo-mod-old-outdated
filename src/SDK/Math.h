#pragma once
#include <cmath>
#include <algorithm>

namespace sdk {

struct Vec3 {
    float x = 0, y = 0, z = 0;
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    float length2d() const { return std::sqrt(x * x + y * y); }
};

inline float deg2rad(float d) { return d * 3.14159265f / 180.f; }

// wrap an angle into (-180, 180]
inline float normalize_yaw(float a) {
    while (a > 180.f) a -= 360.f;
    while (a < -180.f) a += 360.f;
    return a;
}

} // namespace sdk
