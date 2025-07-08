#pragma once

#include <cmath>
#include <algorithm>

// 3D Simplex Noise C++ Implementation
// Converted from Unity shader based on webgl-noise by Ashima Arts
// Original work Copyright (C) 2011 Ashima Arts
// Translation and modification by Keijiro Takahashi
// C++ conversion maintains the same algorithm and constants

struct Vec3 {
    float x, y, z;
    
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }
    
    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }
    
    Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }
    
    float dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
};

struct Vec4 {
    float x, y, z, w;
    
    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    
    Vec4 operator+(const Vec4& other) const {
        return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
    }
    
    Vec4 operator-(const Vec4& other) const {
        return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
    }
    
    Vec4 operator*(float scalar) const {
        return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
    }
    
    Vec4 operator*(const Vec4& other) const {
        return Vec4(x * other.x, y * other.y, z * other.z, w * other.w);
    }
    
    float dot(const Vec4& other) const {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }
};

// Helper functions
inline Vec3 mod289(const Vec3& x) {
    return Vec3(
        x.x - std::floor(x.x / 289.0f) * 289.0f,
        x.y - std::floor(x.y / 289.0f) * 289.0f,
        x.z - std::floor(x.z / 289.0f) * 289.0f
    );
}

inline Vec4 mod289(const Vec4& x) {
    return Vec4(
        x.x - std::floor(x.x / 289.0f) * 289.0f,
        x.y - std::floor(x.y / 289.0f) * 289.0f,
        x.z - std::floor(x.z / 289.0f) * 289.0f,
        x.w - std::floor(x.w / 289.0f) * 289.0f
    );
}

inline Vec4 permute(const Vec4& x) {
    return mod289(Vec4(
        (x.x * 34.0f + 1.0f) * x.x,
        (x.y * 34.0f + 1.0f) * x.y,
        (x.z * 34.0f + 1.0f) * x.z,
        (x.w * 34.0f + 1.0f) * x.w
    ));
}

inline Vec4 taylorInvSqrt(const Vec4& r) {
    return Vec4(
        1.79284291400159f - r.x * 0.85373472095314f,
        1.79284291400159f - r.y * 0.85373472095314f,
        1.79284291400159f - r.z * 0.85373472095314f,
        1.79284291400159f - r.w * 0.85373472095314f
    );
}

inline Vec3 step(const Vec3& edge, const Vec3& x) {
    return Vec3(
        x.x >= edge.x ? 1.0f : 0.0f,
        x.y >= edge.y ? 1.0f : 0.0f,
        x.z >= edge.z ? 1.0f : 0.0f
    );
}

inline Vec4 step(const Vec4& edge, const Vec4& x) {
    return Vec4(
        x.x >= edge.x ? 1.0f : 0.0f,
        x.y >= edge.y ? 1.0f : 0.0f,
        x.z >= edge.z ? 1.0f : 0.0f,
        x.w >= edge.w ? 1.0f : 0.0f
    );
}

inline Vec3 min(const Vec3& a, const Vec3& b) {
    return Vec3(
        std::min(a.x, b.x),
        std::min(a.y, b.y),
        std::min(a.z, b.z)
    );
}

inline Vec3 max(const Vec3& a, const Vec3& b) {
    return Vec3(
        std::max(a.x, b.x),
        std::max(a.y, b.y),
        std::max(a.z, b.z)
    );
}

inline Vec4 max(const Vec4& a, float b) {
    return Vec4(
        std::max(a.x, b),
        std::max(a.y, b),
        std::max(a.z, b),
        std::max(a.w, b)
    );
}

inline Vec4 floor(const Vec4& x) {
    return Vec4(
        std::floor(x.x),
        std::floor(x.y),
        std::floor(x.z),
        std::floor(x.w)
    );
}

inline Vec4 abs(const Vec4& x) {
    return Vec4(
        std::abs(x.x),
        std::abs(x.y),
        std::abs(x.z),
        std::abs(x.w)
    );
}

// Main simplex noise function
inline float snoise(const Vec3& v) {
    const float C1 = 1.0f / 6.0f;
    const float C2 = 1.0f / 3.0f;
    const Vec3 C = Vec3(C1, C1, C1);
    
    // First corner
    Vec3 i = Vec3(
        std::floor(v.x + (v.x + v.y + v.z) * C2),
        std::floor(v.y + (v.x + v.y + v.z) * C2),
        std::floor(v.z + (v.x + v.y + v.z) * C2)
    );
    
    Vec3 x0 = v - i + Vec3(
        (i.x + i.y + i.z) * C1,
        (i.x + i.y + i.z) * C1,
        (i.x + i.y + i.z) * C1
    );
    
    // Other corners
    Vec3 g = step(Vec3(x0.y, x0.z, x0.x), Vec3(x0.x, x0.y, x0.z));
    Vec3 l = Vec3(1.0f, 1.0f, 1.0f) - g;
    Vec3 i1 = min(Vec3(g.x, g.y, g.z), Vec3(l.z, l.x, l.y));
    Vec3 i2 = max(Vec3(g.x, g.y, g.z), Vec3(l.z, l.x, l.y));
    
    Vec3 x1 = x0 - i1 + C;
    Vec3 x2 = x0 - i2 + Vec3(C2, C2, C2);
    Vec3 x3 = x0 - Vec3(0.5f, 0.5f, 0.5f);
    
    // Permutations
    i = mod289(i);
    Vec4 p = permute(permute(permute(
        Vec4(i.z, i.z + i1.z, i.z + i2.z, i.z + 1.0f)) + 
        Vec4(i.y, i.y + i1.y, i.y + i2.y, i.y + 1.0f)) + 
        Vec4(i.x, i.x + i1.x, i.x + i2.x, i.x + 1.0f));
    
    // Gradients: 7x7 points over a square, mapped onto an octahedron
    Vec4 j = p - Vec4(49.0f, 49.0f, 49.0f, 49.0f) * floor(p * (1.0f / 49.0f));
    
    Vec4 x_ = floor(j * (1.0f / 7.0f));
    Vec4 y_ = floor(j - Vec4(7.0f, 7.0f, 7.0f, 7.0f) * x_);
    
    Vec4 x = (x_ * 2.0f + Vec4(0.5f, 0.5f, 0.5f, 0.5f)) * (1.0f / 7.0f) - Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    Vec4 y = (y_ * 2.0f + Vec4(0.5f, 0.5f, 0.5f, 0.5f)) * (1.0f / 7.0f) - Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    
    Vec4 h = Vec4(1.0f, 1.0f, 1.0f, 1.0f) - abs(x) - abs(y);
    
    Vec4 b0 = Vec4(x.x, x.y, y.x, y.y);
    Vec4 b1 = Vec4(x.z, x.w, y.z, y.w);
    
    Vec4 s0 = floor(b0) * 2.0f + Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    Vec4 s1 = floor(b1) * 2.0f + Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    Vec4 sh = Vec4(0.0f, 0.0f, 0.0f, 0.0f) - step(Vec4(0.0f, 0.0f, 0.0f, 0.0f), h);
    
    Vec4 a0 = Vec4(b0.x, b0.z, b0.y, b0.w) + Vec4(s0.x, s0.z, s0.y, s0.w) * Vec4(sh.x, sh.x, sh.y, sh.y);
    Vec4 a1 = Vec4(b1.x, b1.z, b1.y, b1.w) + Vec4(s1.x, s1.z, s1.y, s1.w) * Vec4(sh.z, sh.z, sh.w, sh.w);
    
    Vec3 g0 = Vec3(a0.x, a0.y, h.x);
    Vec3 g1 = Vec3(a0.z, a0.w, h.y);
    Vec3 g2 = Vec3(a1.x, a1.y, h.z);
    Vec3 g3 = Vec3(a1.z, a1.w, h.w);
    
    // Normalize gradients
    Vec4 norm = taylorInvSqrt(Vec4(g0.dot(g0), g1.dot(g1), g2.dot(g2), g3.dot(g3)));
    g0 = g0 * norm.x;
    g1 = g1 * norm.y;
    g2 = g2 * norm.z;
    g3 = g3 * norm.w;
    
    // Mix final noise value
    Vec4 m = max(Vec4(0.6f, 0.6f, 0.6f, 0.6f) - Vec4(x0.dot(x0), x1.dot(x1), x2.dot(x2), x3.dot(x3)), 0.0f);
    m = m * m;
    m = m * m;
    
    Vec4 px = Vec4(x0.dot(g0), x1.dot(g1), x2.dot(g2), x3.dot(g3));
    return 42.0f * m.dot(px);
}

// Convenience function for easier usage
inline float snoise(float x, float y, float z) {
    return snoise(Vec3(x, y, z));
}
