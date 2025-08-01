#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <vector>

#include <System/Audio/VoiceInstance.h>
#include <System/Audio/VoiceCallback.h>

class AudioSystem;

class SoundInstance
{
public:

    SoundInstance(uint32_t _soundID, AudioSystem* _audioSystem, float _sampleRate);
    ~SoundInstance();

    std::shared_ptr<VoiceInstance> GenerateVoiceInstance(float _volume = 1.0f, float _startTime = 0.0f, bool _loop = false, bool _enableOverlap = true, VoiceCallBack* _callback = nullptr);

    std::shared_ptr<VoiceInstance> Play(float _volume, bool _loop = false, bool _enableOverlap = true, VoiceCallBack* _callback = nullptr);
    std::shared_ptr<VoiceInstance> Play(float _volume, float _startTime, bool _loop = false, bool _enableOverlap = true, VoiceCallBack* _callback = nullptr);

    /// <summary>
    /// 音声ファイルの再生時間を取得
    /// </summary>
    /// <returns>再生時間(秒)</returns>
    float GetDuration() const;

private:

    AudioSystem* audioSystem_;
    uint32_t soundID_;

    float sampleRate_;

    std::vector<std::weak_ptr<VoiceInstance>> voiceInstance_;


};