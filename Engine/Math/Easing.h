#pragma once

#include <Debug/ImGuiManager.h>

#include <functional>
#include <string>
class Easing
{
public:

    enum class EasingFunc
    {
        Linear,
        HalfThresholdStep,
        FullThresholdStep,
        EaseInsine,
        EaseInQuad,
        EaseInCubic,
        EaseInQuart,
        EaseInQuint,
        EaseInExpo,
        EaseInCirc,
        EaseInBack,
        EaseInElastic,
        EaseInBounce,
        EaseOutSine,
        EaseOutQuad,
        EaseOutCubic,
        EaseOutQuart,
        EaseOutQuint,
        EaseOutExpo,
        EaseOutCirc,
        EaseOutBack,
        EaseOutElastic,
        EaseOutBounce,
        EaseInOutSine,
        EaseInOutQuad,
        EaseInOutCubic,
        EaseInOutQuart,
        EaseInOutQuint,
        EaseInOutExpo,
        EaseInOutCirc,
        EaseInOutBack,
        EaseInOutElastic,
        EaseInOutBounce
    };

    // 定量
    static float Linear(float _t);
    static float HalfThresholdStep(float _t);
    static float fullThresholdStep(float _t);

    // イーズイン
    static float EaseInsine(float _t);
    static float EaseInQuad(float _t);
    static float EaseInCubic(float _t);
    static float EaseInQuart(float _t);
    static float EaseInQuint(float _t);
    static float EaseInExpo(float _t);
    static float EaseInCirc(float _t);
    static float EaseInBack(float _t);
    static float EaseInElastic(float _t);
    static float EaseInBounce(float _t);

    // イーズアウト
    static float EaseOutSine(float _t);
    static float EaseOutQuad(float _t);
    static float EaseOutCubic(float _t);
    static float EaseOutQuart(float _t);
    static float EaseOutQuint(float _t);
    static float EaseOutExpo(float _t);
    static float EaseOutCirc(float _t);
    static float EaseOutBack(float _t);
    static float EaseOutElastic(float _t);
    static float EaseOutBounce(float _t);

    // イーズインアウト
    static float EaseInOutSine(float _t);
    static float EaseInOutQuad(float _t);
    static float EaseInOutCubic(float _t);
    static float EaseInOutQuart(float _t);
    static float EaseInOutQuint(float _t);
    static float EaseInOutExpo(float _t);
    static float EaseInOutCirc(float _t);
    static float EaseInOutBack(float _t);
    static float EaseInOutElastic(float _t);
    static float EaseInOutBounce(float _t);

    static std::string GetEasingFuncName(int _funcNum);

    static std::function<float(float)> Func(EasingFunc _type);
    static std::function<float(float)> SelectFuncPtr(int _funcNum);
    static int SelectEasingFunc(int _funcNum);

private:
    static const char* easingFuncs[];

    // 関数テーブル
    static float (*pEasingFunc[])(float);


};
