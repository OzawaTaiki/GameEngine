#include "SequenceEvent.h"

#include <stdexcept>

SequenceEvent::SequenceEvent(const std::string& _label, ParameterValue _value) :
    label_(_label),
    isSelect_(false)
{
    useType_ = CheckType(_value);
}

void SequenceEvent::Update(float _currentTime)
// t を引数でもらってくるべき
{
    DeleteMarkedKeyFrame();
}

void SequenceEvent::ClearSelectKeyFrames()
{
    for (auto& keyFrame : keyFrames_)
    {
        keyFrame.isSelect = false;
    }
}

void SequenceEvent::AddKeyFrame(float _time, ParameterValue _value, uint32_t _easingType)
{
    if (useType_ != CheckType(_value))
    {
        // エラー処理
        throw std::runtime_error("Invalid type");
        return;
    }

    KeyFrame keyFrame;
    keyFrame.time = _time;
    keyFrame.value = _value;
    keyFrame.easingType = _easingType;
    keyFrame.isSelect = false;
    keyFrame.deleteFlag = false;

    keyFrames_.push_back(keyFrame);
}

void SequenceEvent::AddKeyFrame(float _time)
{
    KeyFrame keyFrame;
    keyFrame.time = _time;
    keyFrame.easingType = 0;
    keyFrame.isSelect = false;
    keyFrame.deleteFlag = false;

    if      (useType_ == UseType::Int)          keyFrame.value = 0;
    else if (useType_ == UseType::Float)        keyFrame.value = 0.0f;
    else if (useType_ == UseType::Vector2)      keyFrame.value = Vector2(0.0f, 0.0f);
    else if (useType_ == UseType::Vector3)      keyFrame.value = Vector3(0.0f, 0.0f, 0.0f);
    else if (useType_ == UseType::Vector4)      keyFrame.value = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    else if (useType_ == UseType::Quaternion)   keyFrame.value = Quaternion(0.0f, 0.0f, 0.0f, 0.0f);
    else {
        throw std::runtime_error("Invalid type");
        return;
    }

    keyFrames_.push_back(keyFrame);
}

void SequenceEvent::DeleteMarkedKeyFrame()
{
    for (auto it = keyFrames_.begin(); it != keyFrames_.end();) {
        if (it->deleteFlag)
            it = keyFrames_.erase(it);

        else
            ++it;
    }
}

void SequenceEvent::EditKeyFrameValue(KeyFrame& _keyFrame)
{
    static float speed = 1.0f;
    std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int32_t>) {
            ImGui::DragInt("Value", &arg);
        }
        else if constexpr (std::is_same_v<T, float>) {
            ImGui::DragFloat("speed", &speed, 0.01f);
            ImGui::DragFloat("Value", &arg,speed);
        }
        else if constexpr (std::is_same_v<T, Vector2>) {
            ImGui::DragFloat("speed", &speed, 0.01f);
            ImGui::DragFloat2("Value", &arg.x, speed);
        }
        else if constexpr (std::is_same_v<T, Vector3>) {
            ImGui::DragFloat("speed", &speed, 0.01f);
            ImGui::DragFloat3("Value", &arg.x, speed);
        }
        else if constexpr (std::is_same_v<T, Vector4>) {
            ImGui::DragFloat("speed", &speed, 0.01f);
            ImGui::DragFloat4("Value", &arg.x, speed);
        }
        else if constexpr (std::is_same_v<T, Quaternion>) {
            ImGui::DragFloat("speed", &speed, 0.01f);
            ImGui::DragFloat4("Value", &arg.x, speed);
        }
        else {
            throw std::runtime_error("Invalid type");
        }
        }, _keyFrame.value);
}

SequenceEvent::UseType SequenceEvent::CheckType(ParameterValue _value)
{
    UseType type = UseType::Error;

    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int32_t>) {
            type = UseType::Int;
        }
        else if constexpr (std::is_same_v<T, float>) {
            type = UseType::Float;
        }
        else if constexpr (std::is_same_v<T, Vector2>) {
            type = UseType::Vector2;
        }
        else if constexpr (std::is_same_v<T, Vector3>) {
            type = UseType::Vector3;
        }
        else if constexpr (std::is_same_v<T, Vector4>) {
            type = UseType::Vector4;
        }
        else if constexpr (std::is_same_v<T, Quaternion>) {
            type = UseType::Quaternion;
        }
        else {
            throw std::runtime_error("Invalid type");
            type = UseType::Error;
        }}, _value);

        return type;
}
