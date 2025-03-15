#pragma once

#include <Features/Animation/Sequence/SequenceEvent.h>

#include <list>
#include<memory>

class AnimationSequence
{
public:
    AnimationSequence(const std::string& _label);
    ~AnimationSequence() = default;

    void Update(float _deltaTime);


    void AddSequenceEvent(std::unique_ptr<SequenceEvent> _sequenceEvent);

    void DeleteMarkedSequenceEvent();

    std::string GetLabel() const { return label_; }

    float GetCurrent() const { return currentTime_; }
    void SetCurrentTime(float _currentTime) { currentTime_ = _currentTime; }

    void MarkEventForDeletion(const std::string& _label);
    void MarkEventForDeletion(SequenceEvent* _sequenceEvent);

    template<typename T>
    void CreateSequenceEvent(const std::string& _label, T _value, float _startTime = 0.0f, uint32_t _easingType = 0)
    {
        std::unique_ptr<SequenceEvent> sequenceEvent = std::make_unique<SequenceEvent>(_label, _value);
        AddSequenceEvent(std::move(sequenceEvent));

    }

    template<typename T>
    T GetValue(const std::string& _label) const {
        for (auto& sequenceEvent : sequenceEvents_)
        {
            if (sequenceEvent->GetLabel() == _label)
            {
                return sequenceEvent->GetValue<T>();
            }
        }
        throw std::runtime_error("Invalid label");
        return T();
    }

    std::list<SequenceEvent*> GetSequenceEvents();

private:

    std::string label_;
    float currentTime_;

    std::list<std::unique_ptr<SequenceEvent>>sequenceEvents_;

};
