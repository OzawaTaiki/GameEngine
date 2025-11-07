#pragma once

#include <Features/Animation/Sequence/SequenceEvent.h>
#include <Features/Json/JsonBinder.h>
#include <Debug/Debug.h>

#include <json.hpp>
#include <list>
#include <memory>

class AnimationSequence
{
public:
    AnimationSequence(const std::string& _label);
    ~AnimationSequence();

    void Initialize(const std::string& _filepath = "");
    void Update(float _deltaTime);

    void Save();

    bool HasEvent(const std::string& _label) const;

    void AddSequenceEvent(SequenceEvent* _sequenceEvent);

    bool IsEnd() const;

    void DeleteMarkedSequenceEvent();

    std::string GetLabel() const { return label_; }

    float GetCurrent() const { return currentTime_; }
    void SetCurrentTime(float _currentTime) { currentTime_ = _currentTime; }

    bool IsLooping() const { return isLooping_; }
    void SetLooping(bool _isLooping) { isLooping_ = _isLooping; }

    float GetMaxPlayTime() const { return maxPlayTime_; }
    void SetMaxPlayTime(float _maxPlayTime) { maxPlayTime_ = _maxPlayTime; }

    void MarkEventForDeletion(const std::string& _label);
    void MarkEventForDeletion(SequenceEvent* _sequenceEvent);

    template<typename T>
    void CreateSequenceEvent(const std::string& _label, T _value)
    {
        SequenceEvent* sequenceEvent = new SequenceEvent(_label, _value);
        AddSequenceEvent(sequenceEvent);

    }

    template<typename T>
    T GetValue(const std::string& _label) const {
        for (auto& sequenceEvent : sequenceEvents_)
        {
            if (sequenceEvent.second->GetLabel() == _label)
            {
                // keyframeが空のとき
                if (sequenceEvent.second->GetKeyFrames().empty())
                    return T();

                return sequenceEvent.second->GetValue<T>();
            }
        }
        Debug::Log("SequenceEvent::GetValue Invalid label");
        return T();
    }

    template<typename T>
    T GetValueAtTime(const std::string& _label,float _time) const
    {
        for (auto& sequenceEvent : sequenceEvents_)
        {
            if (sequenceEvent.second->GetLabel() == _label)
            {
                // keyframeが空のとき
                if (sequenceEvent.second->GetKeyFrames().empty())
                    return T();
                return sequenceEvent.second->GetValueAtTime<T>(_time);
            }
        }
        Debug::Log("SequenceEvent::GetValueInvalid label");
        return T();
    }

    std::list<SequenceEvent*> GetSequenceEvents();

private:

    std::string label_;
    float currentTime_;
    bool isLooping_;
    float maxPlayTime_;

    std::unique_ptr<JsonBinder> jsonBinder_ = nullptr;

    uint32_t sequenceSize_;
    std::map<std::string, SequenceEvent*> sequenceEvents_;

    std::vector<std::string> eventLabels_;




};
