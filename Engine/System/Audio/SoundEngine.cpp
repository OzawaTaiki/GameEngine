#include <System/Audio/SoundEngine.h>

#include <System/Audio/AudioSystem.h>
#include <System/Audio/SoundInstance.h>
#include <System/Audio/VoiceInstance.h>
#include <System/Audio/SubmixVoice.h>
#include <System/Audio/AudioEffectManager.h>

#include <Features/Json/JsonSerializers.h>
#include <Debug/Debug.h>

#include <fstream>
#include <cassert>

namespace Engine
{

SoundEngine* SoundEngine::GetInstance()
{
    static SoundEngine instance;
    return &instance;
}

void SoundEngine::Initialize()
{
    soundDefs_.clear();
    loadedInstances_.clear();
    playingSounds_.clear();
    nextHandle_ = 0;
}

void SoundEngine::Finalize()
{
    StopAll();
    playingSounds_.clear();
    loadedInstances_.clear();
    soundDefs_.clear();
}

void SoundEngine::LoadSoundData(const std::string& jsonPath)
{
    json jsonData = JsonFileIO::Load(jsonPath, "");
    if (jsonData.empty())
    {
        Debug::Log("Failed to load sound data from: " + jsonPath + "\n");
        return;
    }


    if (!jsonData.contains("sounds"))
        return;

    for (const auto& entry : jsonData["sounds"])
    {
        SoundDef def = entry.get<SoundDef>(); // from_json(SoundDef) を使用

        if (def.id.empty() || def.filePath.empty())
            continue;

        soundDefs_[def.id] = def;

        auto instance = AudioSystem::GetInstance()->Load(def.filePath);
        if (instance)
        {
            loadedInstances_[def.id] = instance;
        }
    }
}

void SoundEngine::LoadEventData(const std::string& jsonPath)
{
    json jsonData = JsonFileIO::Load(jsonPath, "");
    if (jsonData.empty())
    {
        Debug::Log("Failed to load sound event data from: " + jsonPath + "\n");
        return;
    }

    if (!jsonData.contains("events"))
        return;

    for (const auto& entry : jsonData["events"])
    {
        SoundEventDef eventDef = entry.get<SoundEventDef>(); // from_json(SoundEventDef) を使用
        if (eventDef.name.empty())
            continue;

        eventDefs_[eventDef.name] = eventDef;
    }
}

void SoundEngine::PostEvent(const std::string& eventName)
{
    auto it = eventDefs_.find(eventName);
    if (it == eventDefs_.end())
        return;

    const SoundEventDef& eventDef = it->second;
    for (const auto& action : eventDef.actions)
    {
        switch (action.type)
        {
            case SoundEventType::Play:
            {
                if (action.effects.empty())
                    Play(action.soundId, action.volume, action.loop);
                else
                    Play(action.soundId, action.effects, action.volume, action.loop);
                break;
            }
            case SoundEventType::Stop:
            {
                std::vector<SoundHandle> targets;
                for (const auto& [handle, ps] : playingSounds_)
                    if (ps.soundId == action.soundId)
                        targets.push_back(handle);
                for (auto h : targets)
                    Stop(h);
                break;
            }
            case SoundEventType::Pause:
            {
                std::vector<SoundHandle> targets;
                for (const auto& [handle, ps] : playingSounds_)
                    if (ps.soundId == action.soundId)
                        targets.push_back(handle);
                for (auto h : targets)
                    Pause(h);
                break;
            }
            case SoundEventType::Resume:
            {
                std::vector<SoundHandle> targets;
                for (const auto& [handle, ps] : playingSounds_)
                    if (ps.soundId == action.soundId)
                        targets.push_back(handle);
                for (auto h : targets)
                    Resume(h);
                break;
            }
            case SoundEventType::SetVolume:
            {

                std::vector<SoundHandle> targets;
                for (const auto& [handle, ps] : playingSounds_)
                    if (ps.soundId == action.soundId)
                        targets.push_back(handle);
                for (auto h : targets)
                    SetVolume(h, action.volume);
                break;
            }
            default:
                break;
        }
    }
}

SoundHandle SoundEngine::Play(const std::string& soundId,
                              float volume,
                              bool  loop,
                              float startTime)
{
    auto instIt = loadedInstances_.find(soundId);
    if (instIt == loadedInstances_.end())
        return kInvalidHandle;

    auto defIt = soundDefs_.find(soundId);
    if (defIt == soundDefs_.end())
        return kInvalidHandle;

    const SoundDef& def           = defIt->second;
    auto& soundInstance = instIt->second;

    // BGM / SE に応じて submix を選択
    SubmixVoice* submix = (def.type == "BGM")
        ? AudioSystem::GetInstance()->GetBGMSubmix()
        : AudioSystem::GetInstance()->GetSESubmix();

    auto voice = soundInstance->GenerateVoiceInstance(
        volume,              // 音量
        startTime,           // 再生開始位置（秒）
        loop,                // ループ
        def.enableOverlap,   // 重複再生（SoundDef で指定）
        nullptr,             // コールバック
        submix,              // submix
        nullptr              // エフェクトチェーン（なし）
    );

    if (!voice)
        return kInvalidHandle;

    voice->Play();

    SoundHandle handle = GenerateHandle();
    playingSounds_[handle] = PlayingSound{ soundInstance, voice, soundId };
    return handle;
}

SoundHandle SoundEngine::Play(const std::string& soundId,
                              const std::vector<std::string>& effects,
                              float volume,
                              bool loop,
                              float startTime)
{
    auto instIt = loadedInstances_.find(soundId);
    if (instIt == loadedInstances_.end())
        return kInvalidHandle;

    auto defIt = soundDefs_.find(soundId);
    if (defIt == soundDefs_.end())
        return kInvalidHandle;

    const SoundDef& def           = defIt->second;
    auto& soundInstance = instIt->second;

    // BGM / SE に応じて submix を選択
    SubmixVoice* submix = (def.type == "BGM")
        ? AudioSystem::GetInstance()->GetBGMSubmix()
        : AudioSystem::GetInstance()->GetSESubmix();

    // effects からエフェクトチェーンを構築
    auto effectChain = AudioEffectManager::GetInstance()->BuildEffectChain(effects);

    auto voice = soundInstance->GenerateVoiceInstance(
        volume,              // 音量
        startTime,           // 再生開始位置（秒）
        loop,                // ループ
        def.enableOverlap,   // 重複再生（SoundDef で指定）
        nullptr,             // コールバック
        submix,              // submix
        effectChain.BuildChain()              // エフェクトチェーン
    );

    if (!voice)
        return kInvalidHandle;

    voice->Play();

    SoundHandle handle = GenerateHandle();
    playingSounds_[handle] = PlayingSound{ soundInstance, voice, soundId };
    return handle;
}

void SoundEngine::Stop(SoundHandle handle)
{
    auto it = playingSounds_.find(handle);
    if (it == playingSounds_.end())
        return;

    if (it->second.voiceInstance)
        it->second.voiceInstance->Stop();

    playingSounds_.erase(it);
}

void SoundEngine::StopAll()
{
    for (auto& [handle, ps] : playingSounds_)
    {
        if (ps.voiceInstance)
            ps.voiceInstance->Stop();
    }
    playingSounds_.clear();
}

void SoundEngine::Pause(SoundHandle handle)
{
    auto it = playingSounds_.find(handle);
    if (it == playingSounds_.end())
        return;

    if (it->second.voiceInstance)
        it->second.voiceInstance->Pause();
}

void SoundEngine::Resume(SoundHandle handle)
{
    auto it = playingSounds_.find(handle);
    if (it == playingSounds_.end())
        return;

    if (it->second.voiceInstance)
        it->second.voiceInstance->Resume();
}

void SoundEngine::SetVolume(SoundHandle handle, float volume)
{
    auto it = playingSounds_.find(handle);
    if (it == playingSounds_.end())
        return;

    if (it->second.voiceInstance)
        it->second.voiceInstance->SetVolume(volume);
}

bool SoundEngine::IsPlaying(SoundHandle handle) const
{
    auto it = playingSounds_.find(handle);
    if (it == playingSounds_.end())
        return false;

    return it->second.voiceInstance && it->second.voiceInstance->IsPlaying();
}

float SoundEngine::GetElapsedTime(SoundHandle handle) const
{
    auto it = playingSounds_.find(handle);
    if (it == playingSounds_.end())
        return 0.0f;

    if (it->second.voiceInstance)
        return it->second.voiceInstance->GetElapsedTime();

    return 0.0f;
}

float SoundEngine::GetDuration(const std::string& soundId) const
{
    auto it = loadedInstances_.find(soundId);
    if (it == loadedInstances_.end())
        return 0.0f;

    return it->second->GetDuration();
}

void SoundEngine::CleanupStoppedVoices()
{
    for (auto it = playingSounds_.begin(); it != playingSounds_.end();)
    {
        if (!it->second.voiceInstance || !it->second.voiceInstance->IsPlaying())
            it = playingSounds_.erase(it);
        else
            ++it;
    }
}

std::shared_ptr<SoundInstance> SoundEngine::GetSoundInstance(const std::string& soundId)
{
    auto it = loadedInstances_.find(soundId);
    if (it == loadedInstances_.end())
        return nullptr;

    return it->second;
}

SoundHandle SoundEngine::GenerateHandle()
{
    // kInvalidHandle（UINT32_MAX）はスキップ
    if (nextHandle_ == kInvalidHandle)
        nextHandle_ = 0;

    return nextHandle_++;
}

} // namespace Engine
