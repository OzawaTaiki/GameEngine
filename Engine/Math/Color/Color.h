#pragma once

#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>

enum class Color
{
    White,
    Black,
    Red,
    Green,
    Blue,
    Yellow,
    Cyan,
    Magenta,

    Transparent
};

struct HSVA;
struct RGBA;

namespace ColorConverter
{
RGBA ToRGBA(const HSVA& hsva);
HSVA ToHSVA(const RGBA& rgba);
};

struct RGBA
{
    RGBA() : r(0), g(0), b(0), a(1) {}
    RGBA(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) :
        r(static_cast<float>(red) / 255.0f),
        g(static_cast<float>(green) / 255.0f),
        b(static_cast<float>(blue) / 255.0f),
        a(static_cast<float>(alpha) / 255.0f)
    {
    }
    RGBA(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
    RGBA(const Vector4& vec4) : r(vec4.x), g(vec4.y), b(vec4.z), a(vec4.w) {}
    RGBA(Color color);
    RGBA(Color color, float alpha);
    RGBA(const HSVA& hsva);
    RGBA(const Vector3& vec3) : r(vec3.x), g(vec3.y), b(vec3.z), a(1.0f) {}
    RGBA(const Vector3& vec3, float alpha) : r(vec3.x), g(vec3.y), b(vec3.z), a(alpha) {}

    Vector4 ToVector4() const { return Vector4(r, g, b, a); }
    explicit operator Vector4() const { return Vector4(r, g, b, a); }

    float r, g, b, a; // 赤、緑、青、アルファ
};

struct HSVA
{
    HSVA() : h(0), s(0), v(0), a(1) {}
    HSVA(float h, float s, float v, float a) : h(h), s(s), v(v), a(a) {}
    HSVA(const RGBA& rgba);
    HSVA(const Vector4& vec4) : h(vec4.x), s(vec4.y), v(vec4.z), a(vec4.w) {}
    HSVA(const Vector3& vec3) : h(vec3.x), s(vec3.y), v(vec3.z), a(1.0f) {}
    HSVA(const Vector3& vec3, float alpha) : h(vec3.x), s(vec3.y), v(vec3.z), a(alpha) {}
    HSVA(Color color);
    HSVA(Color color, float alpha);

    Vector4 ToVector4() const { return Vector4(RGBA(*this).ToVector4()); }
    explicit operator Vector4() const { return ToVector4(); }

    float h, s, v, a; // 色相、彩度、明度、アルファ
};