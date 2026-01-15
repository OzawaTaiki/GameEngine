#include "AnimationSequence.h"



namespace Engine {

AnimationSequence::AnimationSequence(const std::string& _label) :
    label_(_label),
    currentTime_(0.0f),
    isLooping_(false),
    maxPlayTime_(1.0f),
    jsonBinder_(nullptr),
    sequenceSize_(0)
{
}

AnimationSequence::~AnimationSequence()
{
    for (auto& sequenceEvent : sequenceEvents_)
    {
        delete sequenceEvent.second;
    }
    sequenceEvents_.clear();
}

void AnimationSequence::Initialize(const std::string& _filepath)
{
    if (!_filepath.empty())
    {

        jsonBinder_ = std::make_unique<JsonBinder>(label_,_filepath);
        jsonBinder_->RegisterVariable("SequenceSize", &sequenceSize_);
        jsonBinder_->RegisterVariable("IsLooping", &isLooping_);
        jsonBinder_->RegisterVariable("MaxPlayTime", &maxPlayTime_);
        jsonBinder_->RegisterVariable("labels", &eventLabels_);

        for (auto label : eventLabels_)
        {
            SequenceEvent* sequenceEvent = new SequenceEvent(label, jsonBinder_.get());
            AddSequenceEvent(sequenceEvent);
        }

    }
}

void AnimationSequence::Update(float _deltaTime)
{
    currentTime_ += _deltaTime;
    currentTime_ = std::clamp(currentTime_, 0.0f, maxPlayTime_);
    for (auto& sequenceEvent : sequenceEvents_)
    {
        sequenceEvent.second->Update(currentTime_);
    }

}

void AnimationSequence::Save()
{
    for (auto& sequenceEvent : sequenceEvents_)
    {
        sequenceEvent.second->Save();
    }
    sequenceSize_ = static_cast<uint32_t>(sequenceEvents_.size());
    jsonBinder_->Save();

}

bool AnimationSequence::HasEvent(const std::string& _label) const
{
    return sequenceEvents_.contains(_label);
}

void AnimationSequence::AddSequenceEvent(SequenceEvent* _sequenceEvent)
{
    if (_sequenceEvent)
    {
        std::string label = _sequenceEvent->GetLabel();
        sequenceEvents_[label] = _sequenceEvent;
        sequenceEvents_[label]->SetJsonBinder(jsonBinder_.get());

        auto it = std::find(eventLabels_.begin(), eventLabels_.end(), label);
        if (it == eventLabels_.end())
            eventLabels_.push_back(label);
    }
}

bool AnimationSequence::IsEnd() const
{
    if (isLooping_)
    {
        return false; // ループ中は常にfalse
    }

    if (sequenceEvents_.empty())
    {
        return true; // イベントがない場合は終了
    }

    if (currentTime_ < maxPlayTime_)
    {
        return false; // 現在の時間が最大再生時間未満ならfalse
    }

    for (auto& sequenceEvent : sequenceEvents_)
    {
        if (!sequenceEvent.second->IsEnd())
        {
            return false; // 1つでも終了していないイベントがあればfalse
        }
    }

    return true; // 全てのイベントが終了していればtrue
}

void AnimationSequence::DeleteMarkedSequenceEvent()
{
    for (auto it = sequenceEvents_.begin(); it != sequenceEvents_.end();)
    {
        if (it->second->IsDelete())
        {
            std::string label = it->second->GetLabel();
            it = sequenceEvents_.erase(it);

            for (auto it2 = eventLabels_.begin(); it2 != eventLabels_.end();)
            {
                if (*it2 == label)
                {
                    it2 = eventLabels_.erase(it2);
                    break;
                }
                else
                    ++it2;
            }
        }

        else
            ++it;
    }
}

void AnimationSequence::MarkEventForDeletion(const std::string& _label)
{
    if(sequenceEvents_.contains(_label))
        sequenceEvents_.erase(_label);

}

void AnimationSequence::MarkEventForDeletion(SequenceEvent* _sequenceEvent)
{
    MarkEventForDeletion(_sequenceEvent->GetLabel());
}

std::list<SequenceEvent*> AnimationSequence::GetSequenceEvents()
{
    std::list<SequenceEvent*> sequenceEvents;
    for (auto& sequenceEvent : sequenceEvents_)
    {
        sequenceEvents.push_back(sequenceEvent.second);
    }
    return sequenceEvents;
}

} // namespace Engine
