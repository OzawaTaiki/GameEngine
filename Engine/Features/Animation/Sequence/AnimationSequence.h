#pragma once

#include <Features/Animation/Sequence/SequenceEvent.h>
#include <Features/Json/JsonBinder.h>

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


    void AddSequenceEvent(SequenceEvent* _sequenceEvent);

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
    void CreateSequenceEvent(const std::string& _label, T _value, float _startTime = 0.0f, uint32_t _easingType = 0)
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
                return sequenceEvent.second->GetValue<T>();
            }
        }
        throw std::runtime_error("Invalid label");
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
