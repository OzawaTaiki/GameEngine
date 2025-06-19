#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <vector>

#include <System/Audio/VoiceInstance.h>

class AudioSystem;

class SoundInstance
{
public:

    SoundInstance(uint32_t _soundID, AudioSystem* _audioSystem, float _sampleRate);
    ~SoundInstance();

    std::shared_ptr<VoiceInstance> Play(float _volume = 1.0f, bool _loop = false, bool _enableOverlap = true);

private:

    AudioSystem* audioSystem_;
    uint32_t soundID_;

    float sampleRate_;

    std::vector<std::weak_ptr<VoiceInstance>> voiceInstance_;


};