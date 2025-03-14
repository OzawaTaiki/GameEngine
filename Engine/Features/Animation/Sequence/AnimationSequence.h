#pragma once

#include <Features/Animation/Sequence/SequenceEvent.h>

#include <list>
#include<memory>

class WorldTransform;
class AnimationSequence
{
public:
    AnimationSequence(const std::string& _label);
    ~AnimationSequence() = default;

    void Update(float _deltaTime);


    void AddSequenceEvent(std::unique_ptr<SequenceEvent> _sequenceEvent);
    void AddTargetWorldTransform(WorldTransform* _worldTransform);

    std::string GetLabel() const { return label_; }

    float GetCurrent() const { return currentTime_; }
    void SetCurrentTime(float _currentTime) { currentTime_ = _currentTime; }

    template<typename T>
    void CreateSequenceEvent(const std::string& _label, T _value, float _startTime = 0.0f, uint32_t _easingType = 0)
    {
        std::unique_ptr<SequenceEvent> sequenceEvent = std::make_unique<SequenceEvent>(_label, _value);
        AddSequenceEvent(std::move(sequenceEvent));
        
    }

    std::list<SequenceEvent*> GetSequenceEvents();
private:

    std::string label_;
    float currentTime_;

    // 値を適用するワールドトランスフォーム
    std::list<WorldTransform*> targetWorldTransforms_;// いらんかも
    std::list<std::unique_ptr<SequenceEvent>>sequenceEvents_;

};
