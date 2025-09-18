#include "SoundInstance.h"

#include "AudioSystem.h"

#include <Debug/Debug.h>

#include <cassert>

SoundInstance::SoundInstance(uint32_t _soundID, AudioSystem* _audioSystem,float _sampleRate) :
    soundID_(_soundID),
    audioSystem_(_audioSystem),
    sampleRate_(_sampleRate)
{
}

SoundInstance::~SoundInstance()
{
}

std::shared_ptr<VoiceInstance> SoundInstance::GenerateVoiceInstance(float _volume, float _startTime, bool _loop, bool _enableOverlap,  VoiceCallBack* _callback)
{
    //保留
    if (!_enableOverlap) {}

    HRESULT hresult = S_FALSE;

    IXAudio2* xAudio2 = audioSystem_->GetXAudio2().Get();

    IXAudio2SourceVoice* pSourceVoice = nullptr;
    hresult = xAudio2->CreateSourceVoice(
        &pSourceVoice, // Source voice
        &audioSystem_->GetSoundFormat(soundID_), // Wave format
        0, // Flags
        XAUDIO2_DEFAULT_FREQ_RATIO, // Frequency ratio
        _callback,// コールバック関数
        nullptr, // Send list
        nullptr // Effect chain
    );

    if (!SUCCEEDED(hresult))
    {
        if (FAILED(hresult)) {
            //Debug::Log("CreateSourceVoice failed: 0x" + std::to_string(std::hex, hresult));

            switch (hresult) {
            
            case E_INVALIDARG:
                Debug::Log("INVALIDARG - 引数が無効");
                break;
            case E_OUTOFMEMORY:
                Debug::Log("OUT_OF_MEMORY");
                break;
            case XAUDIO2_E_INVALID_CALL:
                Debug::Log("INVALID_CALL - XAudio2が初期化されていない");
                break;
            }
        }

        Debug::Log("Error: Failed to create source voice\n");
        return nullptr;
    }

    UINT32 startSample = static_cast<UINT32>(_startTime * sampleRate_);

    XAUDIO2_BUFFER buf{};
    buf.pAudioData = audioSystem_->GetBuffer(soundID_);
    buf.AudioBytes = static_cast<UINT32>(audioSystem_->GetBufferSize(soundID_));
    buf.PlayBegin = startSample;
    buf.PlayLength = 0; // 最後まで再生
    buf.Flags = XAUDIO2_END_OF_STREAM;

    if (_loop)
        buf.LoopCount = XAUDIO2_LOOP_INFINITE;

    hresult = pSourceVoice->SubmitSourceBuffer(&buf);
    assert(SUCCEEDED(hresult));

    pSourceVoice->SetVolume(_volume);

    auto voiceInstance = std::make_shared<VoiceInstance>(pSourceVoice, _volume, sampleRate_, _startTime);

    voiceInstance_.push_back(voiceInstance);

    return voiceInstance;
}

std::shared_ptr<VoiceInstance> SoundInstance::Play(float _volume, bool _loop, bool _enableOverlap, VoiceCallBack* _callback)
{
    return Play(_volume, 0.0f, _loop, _enableOverlap, _callback);
}

std::shared_ptr<VoiceInstance> SoundInstance::Play(float _volume, float _startTime, bool _loop, bool _enableOverlap, VoiceCallBack* _callback)
{
    auto voiceInstance = GenerateVoiceInstance(_volume, _startTime, _loop, _enableOverlap, _callback);
    if (voiceInstance)
    {
        voiceInstance->Play();
        return voiceInstance;
    }
    else
    {
        Debug::Log("Error: Failed to play sound instance with start time\n");
        return nullptr;
    }
}
//
//std::vector<float> SoundInstance::GetWaveform() const
//{
//    const WAVEFORMATEX& format = audioSystem_->GetSoundFormat(soundID_);
//    const BYTE* buffer = audioSystem_->GetBuffer(soundID_);
//    size_t bufferSize = audioSystem_->GetBufferSize(soundID_);
//
//    return ConvertToFloatSamples(buffer, bufferSize, format);
//}
//
//std::vector<float> SoundInstance::GetWaveform(float _startTime, float _endTime) const
//{
//    const WAVEFORMATEX& format = audioSystem_->GetSoundFormat(soundID_);
//    BYTE* buffer = audioSystem_->GetBuffer(soundID_);
//    unsigned int bufferSize = audioSystem_->GetBufferSize(soundID_);
//
//    // 秒をサンプル数に変換
//    unsigned int startSample = static_cast<unsigned int>(_startTime * format.nSamplesPerSec);
//    unsigned int endSample = static_cast<unsigned int>(_endTime * format.nSamplesPerSec);
//
//    return ConvertToFloatSamples(buffer, bufferSize, format, _startTime, _endTime - _startTime);
//}

float SoundInstance::GetDuration() const
{
    if (audioSystem_)
    {
        size_t bufSize = audioSystem_->GetBufferSize(soundID_);
        auto& wfex = audioSystem_->GetSoundFormat(soundID_);

        if (wfex.nAvgBytesPerSec > 0)
        {
            return static_cast<float>(bufSize) / wfex.nAvgBytesPerSec;
        }
    }
    return 0.0f;
}
