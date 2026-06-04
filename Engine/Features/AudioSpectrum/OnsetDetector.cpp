#include "OnsetDetector.h"

#include <Features/AudioSpectrum/AudioSpectrum.h>

#ifdef _DEBUG
#include <Debug/ImGuiDebugManager.h>
#endif

#include <algorithm>
#include <numeric>

using namespace Engine;

void OnsetDetector::Update(Engine::AudioSpectrum* audioSpectrum, float deltaTime)
{
    if (!audioSpectrum)
        return;

    // --- 1) 現フレームのスペクトルを curSpectrum_ に取り込む ---
    // 低～高域を広く拾う（キャッシュ済みのスペクトルを参照）
    size_t begin = 0, end = 0;
    audioSpectrum->GetSpectrumIndexRange(60.0f, 16000.0f, begin, end);
    audioSpectrum->GetAmplitudesInRange(begin, end, curSpectrum_);

    // 初回は前フレームが無いので保存だけして終了
    if (prevSpectrum_.size() != curSpectrum_.size())
    {
        prevSpectrum_ = curSpectrum_;
        return;
    }

    // --- 2) スペクトラルフラックス（正の差分の総和）---
    float flux = 0.0f;
    for (size_t i = 0; i < curSpectrum_.size(); ++i)
    {
        float diff = curSpectrum_[i] - prevSpectrum_[i];
        if (diff > 0.0f)
            flux += diff;
    }
    flux_ = flux;
    prevSpectrum_ = curSpectrum_;

    // --- 3) 移動平均でしきい値を作る ---
    if (fluxHistory_.size() < historySize_)
        fluxHistory_.resize(historySize_, 0.0f);

    fluxHistory_[historyIndex_] = flux;
    historyIndex_ = (historyIndex_ + 1) % historySize_;

    float avg = std::accumulate(fluxHistory_.begin(), fluxHistory_.end(), 0.0f)
                / static_cast<float>(fluxHistory_.size());
    threshold_ = avg * thresholdMultiplier_;

    // --- 4) クールダウン更新 ---
    if (cooldownTimer_ > 0.0f)
        cooldownTimer_ -= deltaTime;

    // --- 5) 判定 ---
    onsetFlag_ = false;
    dropFlag_  = false;

    if (flux > threshold_ && threshold_ > 0.0001f)
    {
        onsetFlag_ = true;

        // ドロップ：onsetかつフラックスが特に大きく、クールダウン明け
        if (flux > threshold_ * dropFluxScale_ && cooldownTimer_ <= 0.0f)
        {
            dropFlag_ = true;
            cooldownTimer_ = dropCooldown_;
        }
    }

#ifdef _DEBUG
    if (ImGuiDebugManager::GetInstance()->Begin("OnsetDetector"))
    {
        ImGui::Text("Flux: %.4f", flux_);
        ImGui::Text("Threshold: %.4f", threshold_);
        ImGui::Text("Onset: %s  Drop: %s",
                    onsetFlag_ ? "YES" : "-",
                    dropFlag_  ? "YES" : "-");
        ImGui::DragFloat("ThresholdMul", &thresholdMultiplier_, 0.01f, 1.0f, 5.0f);
        ImGui::DragFloat("DropFluxScale", &dropFluxScale_, 0.01f, 1.0f, 8.0f);
        ImGui::DragFloat("DropCooldown", &dropCooldown_, 0.01f, 0.0f, 2.0f);
        ImGui::End();
    }
#endif
}

bool OnsetDetector::ConsumeOnset()
{
    bool r = onsetFlag_;
    onsetFlag_ = false;
    return r;
}

bool OnsetDetector::ConsumeDrop()
{
    bool r = dropFlag_;
    dropFlag_ = false;
    return r;
}
