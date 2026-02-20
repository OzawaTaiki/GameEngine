#pragma once

#include <xaudio2.h>

#include <System/Audio/AudioEffect.h>


namespace Engine
{

class SubmixVoice
{
public:

    SubmixVoice() = default;
    ~SubmixVoice() = default;

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="xAudio2">XAudio2インスタンス</param>
    /// <param name="inputChannels">入力チャンネル数</param>
    /// <param name="sampleRate">サンプルレート</param>
    /// <param name="processingStage"> 処理ステージ。0が最初のステージで、数値が大きいほど後のステージになる。デフォルトは0。</param>
    /// <param name="sendTarget"> 送信先のサブミックスボイス。nullptrの場合はマスターボイスに送信される。デフォルトはnullptr。</param>
    void Initialize(IXAudio2* xAudio2, uint32_t inputChannels, float sampleRate, uint32_t processingStage = 0, SubmixVoice* sendTarget = nullptr);

    void Finalize();

    IXAudio2SubmixVoice* GetSubmixVoice() const { return submixVoice_; }

    void SetVolume(float volume);
    float GetVolume() const { return volume_; }

    AudioEffectChain& GetEffectChain() { return effectChain_; }

private:

    IXAudio2SubmixVoice* submixVoice_ = nullptr;
    AudioEffectChain effectChain_ = {};
    float volume_ = 1.0f;

};

};// namespace Engine