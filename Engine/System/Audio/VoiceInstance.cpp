#include "VoiceInstance.h"

#include <Debug/Debug.h>

VoiceInstance::VoiceInstance(IXAudio2SourceVoice* _sourceVoice, float _volume) :
    sourceVoice_(_sourceVoice),
    volume_(_volume)
{
    if (!sourceVoice_)
    {
        Debug::Log("Error: sourceVoice is null");
        throw std::runtime_error("Error: sourceVoice is null");
        return;
    }

    if (volume_ < 0.0f || volume_ > 1.0f)
    {
        Debug::Log("Error: Volume must be between 0.0 and 1.0");
        volume_ = 1.0f;
    }

    sourceVoice_->SetVolume(volume_);
}

void VoiceInstance::Play()
{
    if (sourceVoice_)
    {
        if (isPaused_)
        {
            sourceVoice_->Start();
            isPaused_ = false;
        }
        else if (isPlaying_)
        {
            sourceVoice_->Start();
        }
    }
}

void VoiceInstance::Stop()
{
    if (sourceVoice_)
    {
        sourceVoice_->Stop();
        sourceVoice_->FlushSourceBuffers();
        isPlaying_ = false;
    }
}

void VoiceInstance::Pause()
{
    if (sourceVoice_)
    {
        sourceVoice_->Stop();
        isPaused_ = true;
    }
}

void VoiceInstance::Resume()
{
    if (sourceVoice_ && isPaused_)
    {
        sourceVoice_->Start();
        isPaused_ = false;
    }
}

void VoiceInstance::FadeIn(float _fadeTime)
{

}

void VoiceInstance::FadeOut(float _fadeTime)
{
}

void VoiceInstance::SetVolume(float _volume)
{
    if (sourceVoice_)
    {
        sourceVoice_->SetVolume(_volume);
        volume_ = _volume;
    }
}
