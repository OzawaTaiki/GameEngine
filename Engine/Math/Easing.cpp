#include <Math/Easing.h>

#include <cmath>
#include <numbers>



namespace Engine {

constexpr float pi_f = std::numbers::pi_v<float>;

const char* Easing::easingFuncs[] = {
    "Linear",

    "HalfThresholdStep",
    "FullThresholdStep",

    "EaseInsine",
    "EaseInQuad",
    "EaseInCubic",
    "EaseInQuart",
    "EaseInQuint",
    "EaseInExpo",
    "EaseInCirc",
    "EaseInBack",
    "EaseInElastic",
    "EaseInBounce",

    "EaseOutSine",
    "EaseOutQuad",
    "EaseOutCubic",
    "EaseOutQuart",
    "EaseOutQuint",
    "EaseOutExpo",
    "EaseOutCirc",
    "EaseOutBack",
    "EaseOutElastic",
    "EaseOutBounce",

    "EaseInOutSine",
    "EaseInOutQuad",
    "EaseInOutCubic",
    "EaseInOutQuart",
    "EaseInOutQuint",
    "EaseInOutExpo",
    "EaseInOutCirc",
    "EaseInOutBack",
    "EaseInOutElastic",
    "EaseInOutBounce"
};

float (*Easing::pEasingFunc[])(float) = {
    &Linear,
    &HalfThresholdStep,
    &fullThresholdStep,

    &EaseInsine,
    &EaseInQuad,
    &EaseInCubic,
    &EaseInQuart,
    &EaseInQuint,
    &EaseInExpo,
    &EaseInCirc,
    &EaseInBack,
    &EaseInElastic,
    &EaseInBounce,

    &EaseOutSine,
    &EaseOutQuad,
    &EaseOutCubic,
    &EaseOutQuart,
    &EaseOutQuint,
    &EaseOutExpo,
    &EaseOutCirc,
    &EaseOutBack,
    &EaseOutElastic,
    &EaseOutBounce,

    &EaseInOutSine,
    &EaseInOutQuad,
    &EaseInOutCubic,
    &EaseInOutQuart,
    &EaseInOutQuint,
    &EaseInOutExpo,
    &EaseInOutCirc,
    &EaseInOutBack,
    &EaseInOutElastic,
    &EaseInOutBounce
};


float Easing::Linear(float _t)
{
    return _t;
}

float Easing::HalfThresholdStep(float _t)
{
    return (_t < 0.5f) ? 0.0f : 1.0f;
}

float Easing::fullThresholdStep(float _t)
{
    return (_t >= 1.0f) ? 1.0f : 0.0f;
}


float Easing::EaseInsine(float _t)
{
    return 1.0f - std::cosf(_t * pi_f * 0.5f);
}

float Easing::EaseInQuad(float _t)
{
    return _t * _t;
}

float Easing::EaseInCubic(float _t)
{
    return _t * _t * _t;
}

float Easing::EaseInQuart(float _t)
{
    return _t * _t * _t * _t;
}

float Easing::EaseInQuint(float _t)
{
    return _t * _t * _t * _t * _t;
}

float Easing::EaseInExpo(float _t)
{
    if (_t == 0.0f)
        return 0.0f;

    else
        return std::pow(2.0f, 10.0f * _t - 10.0f);
}

float Easing::EaseInCirc(float _t)
{
    return 1.0f - std::sqrt(1.0f - _t * _t);
}

float Easing::EaseInBack(float _t)
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;

    return c3 * _t * _t * _t - c1 * _t * _t;
}

float Easing::EaseInElastic(float _t)
{
    const float c4 = (2.0f * pi_f) / 3.0f;

    if (_t == 0.0f)
        return 0.0f;

    else if (_t == 1.0f)
        return 1.0f;

    else
        return -std::pow(2.0f, 10.0f * _t - 10.0f) * std::sinf((_t * 10.0f - 10.75f) * c4);
}

float Easing::EaseInBounce(float _t)
{
    return 1.0f - EaseOutBounce(1.0f - _t);
}

float Easing::EaseOutSine(float _t)
{
    return std::sinf(_t * pi_f * 0.5f);
}

float Easing::EaseOutQuad(float _t)
{
    const float t = 1.0f - _t;
    return 1.0f - t * t;
}

float Easing::EaseOutCubic(float _t)
{
    const float t = 1.0f - _t;
    return 1.0f - t * t * t;
}

float Easing::EaseOutQuart(float _t)
{
    const float t = 1.0f - _t;
    return 1.0f - t * t * t * t;
}

float Easing::EaseOutQuint(float _t)
{
    const float t = 1.0f - _t;
    return 1.0f - t * t * t * t * t;
}

float Easing::EaseOutExpo(float _t)
{
    if (_t == 1.0f)
        return 1.0f;

    else
        return 1.0f - std::pow(2.0f, -10.0f * _t);
}

float Easing::EaseOutCirc(float _t)
{
    const float t = _t - 1.0f;
    return std::sqrt(1.0f - t * t);
}

float Easing::EaseOutBack(float _t)
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    const float t = _t - 1.0f;

    return 1.0f + c3 * t * t * t + c1 * t * t;
}

float Easing::EaseOutElastic(float _t)
{
    const float c4 = (2.0f * pi_f) / 3.0f;

    if (_t == 0.0f)
        return 0.0f;

    else if (_t == 1.0f)
        return 1.0f;

    else
        return std::pow(2.0f, -10.0f * _t) * std::sinf((_t * 10.0f - 0.75f) * c4) + 1.0f;
}

float Easing::EaseOutBounce(float _t)
{
    const float n1 = 2.5625f;
    const float d1 = 2.75f;

    if (_t < 1.0f / d1)
    {
        return n1 * _t * _t;
    }
    else if (_t < 2.0f / d1)
    {
        return n1 * (_t -= 1.5f / d1) * _t + 0.75f;
    }
    else if (_t < 2.5f / d1)
    {
        return n1 * (_t -= 2.25f / d1) * _t + 0.9375f;
    }
    else
    {
        return n1 * (_t -= 2.625f / d1) * _t + 0.984375f;
    }
}

float Easing::EaseInOutSine(float _t)
{
    return -0.5f * (std::cosf(pi_f * _t) - 1.0f);
}

float Easing::EaseInOutQuad(float _t)
{
    if (_t < 0.5f)
        return 2.0f * _t * _t;

    else
        return 1.0f - (-2.0f * _t + 2.0f) * (-2.0f * _t + 2.0f) * 0.5f;

}

float Easing::EaseInOutCubic(float _t)
{
    if (_t < 0.5f)
        return 4.0f * _t * _t * _t;

    else
        return 1.0f - (-2.0f * _t + 2.0f) * (-2.0f * _t + 2.0f) * (-2.0f * _t + 2.0f) * 0.5f;
}

float Easing::EaseInOutQuart(float _t)
{
    if (_t < 0.5f)
        return 8.0f * _t * _t * _t * _t;

    else
        return 1.0f - std::powf(-2.0f * _t + 2.0f, 4.0f) * 0.5f;
}

float Easing::EaseInOutQuint(float _t)
{
    if (_t < 0.5f)
        return 16.0f * _t * _t * _t * _t * _t;
    else
        return 1.0f - std::powf(-2.0f * _t + 2.0f, 5.0f) * 0.5f;
}

float Easing::EaseInOutExpo(float _t)
{
    if (_t == 0.0f)
        return 0.0f;

    else if (_t == 1.0f)
        return 1.0f;

    else if (_t < 0.5f)
        return 0.5f * std::powf(2.0f, 20.0f * _t - 10.0f);

    else
        return (2.0f - std::powf(2.0f, -20.0f * _t + 10.0f)) * 0.5f;
}

float Easing::EaseInOutCirc(float _t)
{
    if (_t < 0.5f)
        return (1.0f - std::sqrtf(1.0f - std::powf(2.0f * _t, 2.0f))) * 0.5f;

    else
        return (std::sqrtf(1.0f - std::powf(-2.0f * _t + 2.0f, 2.0f)) + 1.0f) * 0.5f;
}

float Easing::EaseInOutBack(float _t)
{
    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;
    if (_t < 0.5f)
        return (std::powf(2.0f * _t, 2.0f) * ((c2 + 1.0f) * 2.0f * _t - c2)) * 0.5f;

    else
        return (std::powf(2.0f * _t - 2.0f, 2.0f) * ((c2 + 1.0f) * (2.0f * _t - 2.0f) + c2) + 2.0f) * 0.5f;
}

float Easing::EaseInOutElastic(float _t)
{
    const float c5 = (2.0f * pi_f) / 4.5f;
    if (_t == 0.0f)
        return 0.0f;

    else if (_t == 1.0f)
        return 1.0f;

    else if (_t < 0.5f)
        return -(std::powf(2.0f, 20.0f * _t - 10.0f) * std::sinf((20.0f * _t - 11.125f) * c5)) * 0.5f;

    else
        return std::powf(2.0f, -20.0f * _t + 10.0f) * std::sinf((20.0f * _t - 11.125f) * c5) * 0.5f + 1.0f;
}

float Easing::EaseInOutBounce(float _t)
{
    if (_t < 0.5f)
        return 1.0f - EaseOutBounce(1.0f - 2.0f * _t) * 0.5f;

    else
        return 1.0f + EaseOutBounce(2.0f * _t - 1.0f) * 0.5f;
}

std::string Easing::GetEasingFuncName(int _funcNum)
{
#ifdef _DEBUG
    if (_funcNum < 0 || _funcNum >= IM_ARRAYSIZE(easingFuncs))
        return "";
#endif // _DEBUG
    return easingFuncs[_funcNum];
}

std::function<float(float)> Easing::Func(EasingFunc _type)
{
    int num = static_cast<int>(_type);

    return pEasingFunc[num];
}

std::function<float(float)> Easing::SelectFuncPtr(int _funcNum)
{
#ifdef _DEBUG
    if (_funcNum < 0 || _funcNum >= IM_ARRAYSIZE(easingFuncs))
        return &Linear;

#endif // _DEBUG
    return pEasingFunc[_funcNum];
}

int Easing::SelectEasingFunc(int _funcNum)
{
#ifdef _DEBUG
    ImGui::Combo("EasingFunc", &_funcNum, easingFuncs, IM_ARRAYSIZE(easingFuncs));
#endif // _DEBUG
    return _funcNum;
}

} // namespace Engine
