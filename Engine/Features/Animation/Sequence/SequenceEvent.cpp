#include "SequenceEvent.h"

SequenceEvent::SequenceEvent(const std::string& _label) :
    label_(_label),
    isSelect_(false)
{
}

void SequenceEvent::Update(float _currentTime)
// t を引数でもらってくるべき
{

}

void SequenceEvent::AddKeyFrame(float _time, ParameterValue _value, uint32_t _easingType)
{
    KeyFrame keyFrame;
    keyFrame.time = _time;
    keyFrame.value = _value;
    keyFrame.easingType = _easingType;
    keyFrame.isSelect = false;

    keyFrames_.push_back(keyFrame);
}

bool SequenceEvent::ImDragValue()
{
    ImGui::PushID(this);
    bool hasChanged = false;


    ImGui::PopID();

    return hasChanged;
}


