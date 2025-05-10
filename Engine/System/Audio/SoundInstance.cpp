#include "SoundInstance.h"

#include "AudioSystem.h"

#include <Debug/Debug.h>

#include <cassert>

SoundInstance::SoundInstance(uint32_t _soundID, AudioSystem* _audioSystem) :
    soundID_(_soundID),
    audioSystem_(_audioSystem)
{
}

SoundInstance::~SoundInstance()
{
}

std::shared_ptr<VoiceInstance> SoundInstance::Play(float _volume, bool _loop, bool _enableOverlap)
{
    //保留
    if (!_enableOverlap) {}

    HRESULT hresult = S_FALSE;

    IXAudio2* xAudio2 = audioSystem_->GetXAudio2().Get();

    IXAudio2SourceVoice* pSourceVoice = nullptr;
    hresult = xAudio2->CreateSourceVoice(&pSourceVoice, &audioSystem_->GetSoundFormat(soundID_));

    if (!SUCCEEDED(hresult))
    {
        Debug::Log("Error: Failed to create source voice\n");
        return nullptr;
    }

    XAUDIO2_BUFFER buf{};
    buf.pAudioData = audioSystem_->GetBuffer(soundID_);
    buf.AudioBytes = audioSystem_->GetBufferSize(soundID_);
    buf.Flags = XAUDIO2_END_OF_STREAM;

    if (_loop)
        buf.LoopCount = XAUDIO2_LOOP_INFINITE;

    hresult = pSourceVoice->SubmitSourceBuffer(&buf);
    assert(SUCCEEDED(hresult));

    hresult = pSourceVoice->Start();
    assert(SUCCEEDED(hresult));

    pSourceVoice->SetVolume(_volume);

    auto voiceInstance = std::make_shared<VoiceInstance>(pSourceVoice, _volume);
    voiceInstance->Play();

    voiceInstance_.push_back(voiceInstance);

    return voiceInstance;
}
