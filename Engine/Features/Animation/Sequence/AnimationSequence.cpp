#include "AnimationSequence.h"

AnimationSequence::AnimationSequence(const std::string& _label):
    label_(_label),
    currentTime_(0.0f)
{
}

void AnimationSequence::Update(float _deltaTime)
{
    currentTime_ += _deltaTime;
    for (auto& sequenceEvent : sequenceEvents_)
    {
        sequenceEvent->Update(currentTime_);
    }

}

void AnimationSequence::AddSequenceEvent(std::unique_ptr<SequenceEvent> _sequenceEvent)
{
    if (_sequenceEvent)
    {
        sequenceEvents_.push_back(std::move(_sequenceEvent));
    }
}

void AnimationSequence::AddTargetWorldTransform(WorldTransform* _worldTransform)
{
    if (_worldTransform)
    {
        targetWorldTransforms_.push_back(_worldTransform);
    }
}

std::list<SequenceEvent*> AnimationSequence::GetSequenceEvents()
{
    std::list<SequenceEvent*> sequenceEvents;
    for (auto& sequenceEvent : sequenceEvents_)
    {
        sequenceEvents.push_back(sequenceEvent.get());
    }
    return sequenceEvents;
}
