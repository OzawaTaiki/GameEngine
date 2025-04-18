#include "AlphaOverLifetime.h"

Easing::EasingFunc AlphaOverLifetime::easingType_ = Easing::EasingFunc::EaseOutExpo;

void AlphaOverLifetime::Apply(Particle* _particle, float _deltaTime)
{
    if (_particle == nullptr)
        return;

    // アルファ値を時間に応じて変化させる
    float alpha = 1.0f - (_particle->GetCurrentTime() / _particle->GetLifeTime());

    // イージング関数を適用
    alpha = Easing::Func(easingType_)(alpha);

    alpha = std::clamp(alpha, 0.0f, 1.0f);

    Vector4 color = _particle->GetColor();
    color.w = alpha;

    _particle->SetColor(color);
}
