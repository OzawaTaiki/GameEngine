#include "Color.h"
#include <cmath>

RGBA::RGBA(Color color)
{
    *this = RGBA(color, 1.0f);
}

RGBA::RGBA(Color color, float alpha)
{
    RGBA rgba;

    switch (color)
    {
        case Color::White:
            rgba = RGBA(1.0f, 1.0f, 1.0f, alpha);
            break;
        case Color::Black:
            rgba = RGBA(0.0f, 0.0f, 0.0f, alpha);
            break;
        case Color::Red:
            rgba = RGBA(1.0f, 0.0f, 0.0f, alpha);
            break;
        case Color::Green:
            rgba = RGBA(0.0f, 1.0f, 0.0f, alpha);
            break;
        case Color::Blue:
            rgba = RGBA(0.0f, 0.0f, 1.0f, alpha);
            break;
        case Color::Yellow:
            rgba = RGBA(1.0f, 1.0f, 0.0f, alpha);
            break;
        case Color::Cyan:
            rgba = RGBA(0.0f, 1.0f, 1.0f, alpha);
            break;
        case Color::Magenta:
            rgba = RGBA(1.0f, 0.0f, 1.0f, alpha);
            break;
        case Color::Transparent:
            rgba = RGBA(0.0f, 0.0f, 0.0f, 0.0f);
            break;
        default: // Default to white
            rgba = RGBA(1.0f, 1.0f, 1.0f, alpha);
            break;
    }

    *this = rgba;
}

RGBA::RGBA(const HSVA& hsva)
{
    *this = ColorConverter::ToRGBA(hsva);
}

RGBA ColorConverter::ToRGBA(const HSVA& hsva)
{
    float h = std::fmod(hsva.h, 360.0f);
    if (h < 0.0f) h += 360.0f;
    float s = hsva.s;
    float v = hsva.v;
    float a = hsva.a;

    float c = v * s;
    float x = c * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;

    float r1 = 0.0f, g1 = 0.0f, b1= 0.0f;


    if (h < 60) { r1 = c; g1 = x; b1 = 0; }
    else if (h < 120) { r1 = x; g1 = c; b1 = 0; }
    else if (h < 180) { r1 = 0; g1 = c; b1 = x; }
    else if (h < 240) { r1 = 0; g1 = x; b1 = c; }
    else if (h < 300) { r1 = x; g1 = 0; b1 = c; }
    else { r1 = c; g1 = 0; b1 = x; }


    return RGBA(r1 + m, g1 + m, b1 + m, a);
}

HSVA ColorConverter::ToHSVA(const RGBA& rgba)
{
    float r = rgba.r;
    float g = rgba.g;
    float b = rgba.b;
    float a = rgba.a;

    float max = std::max({ r, g, b });
    float min = std::min({ r, g, b });

    float delta = max - min;

    float h = 0.0f;
    float s = (max == 0.0f) ? 0.0f : (delta / max);
    float v = max;

    if (delta != 0.0f)
    {
        if (max == r)
        {
            h = 60.0f * (std::fmod(((g - b) / delta), 6.0f));
        }
        else if (max == g)
        {
            h = 60.0f * (((b - r) / delta) + 2.0f);
        }
        else if (max == b)
        {
            h = 60.0f * (((r - g) / delta) + 4.0f);
        }
    }
    if (h < 0.0f)
        h += 360.0f;

    return HSVA(h, s, v, a);
}

HSVA::HSVA(const RGBA& rgba)
{
    *this = ColorConverter::ToHSVA(rgba);
}

HSVA::HSVA(Color color)
{
    *this = ColorConverter::ToHSVA(RGBA(color));
}

HSVA::HSVA(Color color, float alpha)
{
    *this = ColorConverter::ToHSVA(RGBA(color, alpha));
}
