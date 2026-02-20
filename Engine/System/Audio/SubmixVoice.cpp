#include "SubmixVoice.h"

#include <Debug/Debug.h>
#include <cassert>


using namespace Engine;

void SubmixVoice::Initialize(IXAudio2* xAudio2, uint32_t inputChannels, float sampleRate, uint32_t processingStage, SubmixVoice* sendTarget)
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
        0,
        processingStage,
        sendTarget ? &sendList : nullptr,
        chainPtr);

    if (FAILED(hr))
    {
        Debug::Log("Failed to create submix voice: " + std::to_string(hr));
        assert(false && "Failed to create submix voice");
        return;
    }

    effectChain_.AttachToVoice(submixVoice_);
    submixVoice_->SetVolume(volume_);
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
