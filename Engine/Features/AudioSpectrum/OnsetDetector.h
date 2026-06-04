#pragma once
#include <vector>

namespace Engine { class AudioSpectrum; }

class OnsetDetector
{
public:

    OnsetDetector() = default;
    ~OnsetDetector() = default;

    void Update(Engine::AudioSpectrum* audioSpectrum, float deltaTime);

    bool ConsumeOnset();
    bool ConsumeDrop();

    float GetFlux() const { return flux_; }
    float GetThreshold() const { return threshold_; }

    void SetThreshold(float multiplier) { thresholdMultiplier_ = multiplier; }
    void SetDropFluxScale(float scale) { dropFluxScale_ = scale; }

private:

    std::vector<float> prevSpectrum_;
    std::vector<float> curSpectrum_;

    float flux_ = 0.0f;                 // 今フレームのスペクトラルフラックス
    float threshold_ = 0.0f;            // 動的しきい値（移動平均ベース）

    // フラックスの移動平均用リングバッファ
    std::vector<float> fluxHistory_;
    size_t historyIndex_ = 0;
    size_t historySize_ = 43;           // 約0.7秒分（60fps想定）

    float cooldownTimer_ = 0.0f;        // ドロップ連発抑止
    float dropCooldown_ = 0.5f;         // 最小再発火間隔(秒)

    // 調整パラメータ
    float thresholdMultiplier_ = 1.5f;  // 移動平均×これを超えたらonset
    float dropFluxScale_ = 2.5f;        // onsetかつ flux>threshold*これ でdrop

    bool onsetFlag_ = false;
    bool dropFlag_ = false;
};
