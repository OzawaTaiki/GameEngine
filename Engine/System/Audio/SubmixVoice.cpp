#include "SubmixVoice.h"

#include <Debug/Debug.h>
#include <Constants/MathConstants.h>

#include <cassert>
#include <algorithm>


using namespace Engine;

HRESULT SubmixVoice::Initialize(IXAudio2* xAudio2, uint32_t inputChannels, float sampleRate, uint32_t processingStage, SubmixVoice* sendTarget)
{
    XAUDIO2_SEND_DESCRIPTOR sendDesc{};
    sendDesc.Flags = 0;
    sendDesc.pOutputVoice = sendTarget ? sendTarget->GetSubmixVoice() : nullptr;

    XAUDIO2_VOICE_SENDS sendList{};
    sendList.SendCount = 1;
    sendList.pSends = &sendDesc;

    const XAUDIO2_EFFECT_CHAIN* chainPtr = effectChain_.IsEmpty() ? nullptr : effectChain_.BuildChain();

    HRESULT hr = xAudio2->CreateSubmixVoice(
        &submixVoice_,
        inputChannels,
        static_cast<UINT32>(sampleRate),
        XAUDIO2_VOICE_USEFILTER,
        processingStage,
        sendTarget ? &sendList : nullptr,
        chainPtr);

    if (FAILED(hr))
    {
        Debug::Log("Failed to create submix voice: " + std::to_string(hr));
        assert(false && "Failed to create submix voice");
        return hr;
    }

    effectChain_.AttachToVoice(submixVoice_);
    submixVoice_->SetVolume(volume_);
    sampleRate_ = sampleRate;

    sendDesc_ = XAUDIO2_SEND_DESCRIPTOR{};
    sendDesc_.Flags = 0;
    sendDesc_.pOutputVoice = submixVoice_;

    sendList_ = XAUDIO2_VOICE_SENDS{};
    sendList_.SendCount = 1;
    sendList_.pSends = &sendDesc_;

    return hr;
}

void SubmixVoice::Finalize()
{
    effectChain_.DetachFromVoice();
    if (submixVoice_)
    {
        submixVoice_->DestroyVoice();
        submixVoice_ = nullptr;
    }
}

void SubmixVoice::SetVolume(float volume)
{
    volume_ = volume;
    if (submixVoice_)
    {
        submixVoice_->SetVolume(volume_);
        // 第二引数が存在
        // 即時化まとめてかを設定できるらしいが現状は不要なのでセットしない
    }
}

void Engine::SubmixVoice::SetFilter(XAUDIO2_FILTER_TYPE type, float cutoffHz, float oneOverQ)
{
    float omega = 2.0f * std::sinf(MathConstants::kPi * cutoffHz / sampleRate_);

    omega = std::clamp(omega, 0.0f, 1.0f);

    XAUDIO2_FILTER_PARAMETERS params{};
    params.Type = type;
    params.Frequency = omega;
    params.OneOverQ = oneOverQ;

    submixVoice_->SetFilterParameters(&params);

}

void Engine::SubmixVoice::ClearFilter()
{
    XAUDIO2_FILTER_PARAMETERS params{};
    params.Type = XAUDIO2_FILTER_TYPE::LowPassFilter;
    params.Frequency = 1.0f;
    params.OneOverQ = 1.0f;

    submixVoice_->SetFilterParameters(&params);

}
