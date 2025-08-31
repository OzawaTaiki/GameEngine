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
        Debug::Log("Error: Failed to create source voice\n");
        return nullptr;
    }

    UINT32 startSample = static_cast<UINT32>(_startTime * sampleRate_);

    XAUDIO2_BUFFER buf{};
    buf.pAudioData = audioSystem_->GetBuffer(soundID_);
    buf.AudioBytes = audioSystem_->GetBufferSize(soundID_);
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

std::vector<float> SoundInstance::GetWaveform() const
{
    const WAVEFORMATEX& format = audioSystem_->GetSoundFormat(soundID_);
    BYTE* buffer = audioSystem_->GetBuffer(soundID_);
    unsigned int bufferSize = audioSystem_->GetBufferSize(soundID_);

    return ConvertToFloatSamples(buffer, bufferSize, format);
}

std::vector<float> SoundInstance::GetWaveform(float _startTime, float _endTime) const
{
    const WAVEFORMATEX& format = audioSystem_->GetSoundFormat(soundID_);
    BYTE* buffer = audioSystem_->GetBuffer(soundID_);
    unsigned int bufferSize = audioSystem_->GetBufferSize(soundID_);

    // 秒をサンプル数に変換
    unsigned int startSample = static_cast<unsigned int>(_startTime * format.nSamplesPerSec);
    unsigned int endSample = static_cast<unsigned int>(_endTime * format.nSamplesPerSec);

    return ConvertToFloatSamples(buffer, bufferSize, format, _startTime, _endTime - _startTime);
}

float SoundInstance::GetDuration() const
{
    if (audioSystem_)
    {
        unsigned int bufSize = audioSystem_->GetBufferSize(soundID_);
        auto& wfex = audioSystem_->GetSoundFormat(soundID_);

        if (wfex.nAvgBytesPerSec > 0)
        {
            return static_cast<float>(bufSize) / wfex.nAvgBytesPerSec;
        }
    }
    return 0.0f;
}

std::vector<float> SoundInstance::ConvertToFloatSamples(
    const BYTE* _pBuffer,
    unsigned int _bufferSize,
    const WAVEFORMATEX& _wfex,
    float _startSeconds,
    float _durationSeconds ) const
{
    std::vector<float> samples;
    int bytesPerSample = _wfex.wBitsPerSample / 8;
    unsigned int totalSamples = _bufferSize / bytesPerSample;
    unsigned int sampleRate = _wfex.nSamplesPerSec;

    // 秒をサンプル数に変換
    unsigned int startSample = static_cast<unsigned int>(_startSeconds * sampleRate);
    unsigned int endSample = totalSamples;

    // 範囲チェック
    if (startSample >= totalSamples) {
        return samples; // 空のベクターを返す
    }

    // durationSecondsが指定されている場合
    if (_durationSeconds > 0.0) {
        unsigned int numSamples = static_cast<unsigned int>(_durationSeconds * sampleRate);
        endSample = std::min(startSample + numSamples, (unsigned int)totalSamples);
    }

    unsigned int actualSamples = endSample - startSample;
    samples.reserve(actualSamples);

    // ビット深度に応じて変換
    if (_wfex.wBitsPerSample == 16)
    {
        // 16bit PCM
        const short* shortData = reinterpret_cast<const short*>(_pBuffer);
        for (unsigned int i = startSample; i < endSample; i++)
        {
            samples.push_back(shortData[i] / 32768.0f); // 2^15
        }
    }
    else if (_wfex.wBitsPerSample == 8)
    {
        // 8bit PCM
        for (unsigned int i = startSample; i < endSample; i++)
        {
            samples.push_back((_pBuffer[i] - 128) / 128.0f); // 0-255 -> -1.0 to 1.0
        }
    }
    else if (_wfex.wBitsPerSample == 24)
    {
        // 24bit PCM
        unsigned int startByte = startSample * 3;
        unsigned int endByte = endSample * 3;

        for (unsigned int i = startByte; i < endByte; i += 3)
        {
            // 24bit -> 32bit変換
            int sample = (_pBuffer[i]) |
                (_pBuffer[i + 1] << 8) |
                (_pBuffer[i + 2] << 16);
            // 符号拡張
            if (sample & 0x800000) {
                sample |= 0xFF000000;
            }
            samples.push_back(sample / 8388608.0f); // 2^23
        }
    }
    else if (_wfex.wBitsPerSample == 32)
    {
        // 32bit PCM
        const int* intData = reinterpret_cast<const int*>(_pBuffer);
        for (unsigned int i = startSample; i < endSample; i++)
        {
            samples.push_back(intData[i] / 2147483648.0f); // 2^31
        }
    }

    return samples;
}

