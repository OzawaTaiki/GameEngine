#pragma once

#include <xaudio2.h>

class VoiceInstance
{
public:
    VoiceInstance(IXAudio2SourceVoice* _sourceVoice, float _volume = 1.0f, float _sampleRate = 44100.0f);
    ~VoiceInstance() = default;

    void Play();
    void Stop();
    void Pause();
    void Resume();

    void FadeIn(float _fadeTime);
    void FadeOut(float _fadeTime);

    void SetVolume(float _volume);
    float GetVolume() const { return volume_; }

    bool IsPlaying() const { return isPlaying_; }
    bool IsPaused() const { return isPaused_; }

    float GetElapsedTime() const;


private:

    bool isPaused_ = false;
    bool isPlaying_ = false;

    bool isFadingIn_ = false;

    float volume_ = 1.0f;
    float sampleRate_ = 44100.0f; // サンプルレート

    IXAudio2SourceVoice* sourceVoice_ = nullptr;

};