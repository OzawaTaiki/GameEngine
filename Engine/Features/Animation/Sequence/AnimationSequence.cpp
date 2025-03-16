#include "AnimationSequence.h"

AnimationSequence::AnimationSequence(const std::string& _label):
    label_(_label),
    currentTime_(0.0f),
    isLooping_(false)
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

void AnimationSequence::DeleteMarkedSequenceEvent()
{
    for (auto it = sequenceEvents_.begin(); it != sequenceEvents_.end();)
    {
        if ((*it)->IsDelete())
            it = sequenceEvents_.erase(it);

        else
            ++it;
    }
}

void AnimationSequence::MarkEventForDeletion(const std::string& _label)
{
    sequenceEvents_.remove_if([_label](const std::unique_ptr<SequenceEvent>& sequenceEvent)
        {
            return sequenceEvent->GetLabel() == _label;
        });
}

void AnimationSequence::MarkEventForDeletion(SequenceEvent* _sequenceEvent)
{
    sequenceEvents_.remove_if([_sequenceEvent](const std::unique_ptr<SequenceEvent>& sequenceEvent)
        {
            return sequenceEvent.get() == _sequenceEvent;
        });
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
