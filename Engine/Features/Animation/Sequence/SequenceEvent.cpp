#include "SequenceEvent.h"
#include <Math/Easing.h>

#include <stdexcept>

SequenceEvent::SequenceEvent(const std::string& _label, ParameterValue _value) :
    label_(_label),
    isSelect_(false)
{
    useType_ = CheckType(_value);
}

void SequenceEvent::Update(float _currentTime)
{
    DeleteMarkedKeyFrame();

    // キーフレームがない場合は何もしない
    if (keyFrames_.empty())
    {
        return;
    }

    // 最初のキーフレームより前の時間の場合
    auto firstKeyFrame = keyFrames_.begin();
    if (_currentTime <= firstKeyFrame->time)
    {
        value_ = firstKeyFrame->value;
        return;
    }

    // 最後のキーフレームより後の時間の場合
    auto lastKeyFrame = std::prev(keyFrames_.end());
    if (_currentTime >= lastKeyFrame->time)
    {
        value_ = lastKeyFrame->value;
        return;
    }

    // 2つのキーフレーム間の補間
    KeyFrame* prevKeyFrame = nullptr;
    KeyFrame* nextKeyFrame = nullptr;

    // 現在の時間を挟む2つのキーフレームを探す
    for (auto it = keyFrames_.begin(); it != keyFrames_.end(); ++it)
    {
        if (it->time > _currentTime)
        {
            nextKeyFrame = &(*it);
            prevKeyFrame = &(*std::prev(it));
            break;
        }
    }

    if (!prevKeyFrame || !nextKeyFrame)
    {
        return; // エラー時は何もしない
    }

    // 2つのキーフレーム間の補間係数を計算
    float totalTime = nextKeyFrame->time - prevKeyFrame->time;
    float t = (_currentTime - prevKeyFrame->time) / totalTime;

    // イージング関数を適用
    t = Easing::SelectFuncPtr(nextKeyFrame->easingType)(t);

    // 型ごとに補間処理
    std::visit([&](auto&& prevValue) {
        using T = std::decay_t<decltype(prevValue)>;

        auto& nextValue = std::get<T>(nextKeyFrame->value);

        if constexpr (std::is_same_v<T, int32_t>) {
            // 整数値の補間（四捨五入）
            value_ = static_cast<int32_t>(std::round(prevValue + (nextValue - prevValue) * t));
        }
        else if constexpr (std::is_same_v<T, float>) {
            // 浮動小数点の線形補間
            value_ = prevValue + (nextValue - prevValue) * t;
        }
        else if constexpr (std::is_same_v<T, Vector2>) {
            // Vector2の補間
            value_ = Vector2(
                prevValue.x + (nextValue.x - prevValue.x) * t,
                prevValue.y + (nextValue.y - prevValue.y) * t
            );
        }
        else if constexpr (std::is_same_v<T, Vector3>) {
            // Vector3の補間
            value_ = Vector3(
                prevValue.x + (nextValue.x - prevValue.x) * t,
                prevValue.y + (nextValue.y - prevValue.y) * t,
                prevValue.z + (nextValue.z - prevValue.z) * t
            );
        }
        else if constexpr (std::is_same_v<T, Vector4>) {
            // Vector4の補間
            value_ = Vector4(
                prevValue.x + (nextValue.x - prevValue.x) * t,
                prevValue.y + (nextValue.y - prevValue.y) * t,
                prevValue.z + (nextValue.z - prevValue.z) * t,
                prevValue.w + (nextValue.w - prevValue.w) * t
            );
        }
        else if constexpr (std::is_same_v<T, Quaternion>) {
            // Quaternionの球面線形補間
            value_ = Quaternion::Slerp(prevValue, nextValue, t);
        }
        }, prevKeyFrame->value);
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
    keyFrame.isDelete = false;

    InsertKeyFrame(keyFrame);
    //keyFrames_.push_back(keyFrame);
}

void SequenceEvent::AddKeyFrame(float _time)
{
    KeyFrame keyFrame;
    keyFrame.time = _time;
    keyFrame.easingType = 0;
    keyFrame.isSelect = false;
    keyFrame.isDelete = false;

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
    InsertKeyFrame(keyFrame);

    //keyFrames_.push_back(keyFrame);
}

void SequenceEvent::InsertKeyFrame(const KeyFrame& _keyFrame)
{
    if(keyFrames_.empty())
    {
        keyFrames_.push_back(_keyFrame);
        return;
    }


    // 並びが正しいか
    float prevTime = 0.0f;

    // キーフレームの挿入
    for (auto it = keyFrames_.begin(); it != keyFrames_.end(); ++it)
    {
        // ひとつ前の要素の時間が現在の要素の時間より大きい場合
        if (it->time < prevTime)
        {
            keyFrames_.push_back(_keyFrame);
            SortKeyFrames();
            return;
        }

        if (it->time > _keyFrame.time)
        {
            keyFrames_.insert(it, _keyFrame);
            return;
        }

        prevTime = it->time;
    }

    keyFrames_.push_back(_keyFrame);

}

void SequenceEvent::SortKeyFrames()
{
    keyFrames_.sort([](const KeyFrame& _a, const KeyFrame& _b) {
        return _a.time < _b.time;
        });
}


void SequenceEvent::DeleteMarkedKeyFrame()
{
    for (auto it = keyFrames_.begin(); it != keyFrames_.end();) {
        if (it->isDelete)
            it = keyFrames_.erase(it);

        else
            ++it;
    }
}

void SequenceEvent::MarkForDelete()
{
    isDelete_ = true;
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
            ImGui::DragFloat("Speed", &speed, 0.01f,0.01f);
            ImGui::DragFloat("Value", &arg,speed);
        }
        else if constexpr (std::is_same_v<T, Vector2>) {
            ImGui::DragFloat("Speed", &speed, 0.01f, 0.01f,10.0f);
            ImGui::DragFloat2("Value", &arg.x, speed);
        }
        else if constexpr (std::is_same_v<T, Vector3>) {
            ImGui::DragFloat("Speed", &speed, 0.01f, 0.01f, 10.0f);
            ImGui::DragFloat3("Value", &arg.x, speed);
        }
        else if constexpr (std::is_same_v<T, Vector4>) {
            ImGui::DragFloat("Speed", &speed, 0.01f, 0.01f, 10.0f);
            ImGui::DragFloat4("Value", &arg.x, speed);
        }
        else if constexpr (std::is_same_v<T, Quaternion>) {
            ImGui::DragFloat("Speed", &speed, 0.01f, 0.01f, 10.0f);
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
