#include "SequenceEvent.h"
#include <Math/Easing.h>

#include <Features/Json/JsonBinder.h>
#include <stdexcept>

SequenceEvent::SequenceEvent(const std::string& _label, JsonBinder* _jsonBinder) :
    label_(_label),
    isSelect_(false),
    isDelete_(false),
    jsonBinder_(_jsonBinder)
{
    RegisterVariables();
    InitializeValueFromKeyFrames();
}

SequenceEvent::SequenceEvent(const std::string& _label, ParameterValue _value, JsonBinder* _jsonBinder) :
    label_(_label),
    isSelect_(false),
    isDelete_(false),
    jsonBinder_(_jsonBinder)
{
    useType_ = CheckType(_value);
    RegisterVariables();
    InitializeValueFromKeyFrames();
}

void SequenceEvent::Initialize(const std::string& _label)
{
    if (!_label.empty())
        label_ = _label;
}

void SequenceEvent::Update(float _currentTime)
{
    DeleteMarkedKeyFrame();

    // キーフレームがない場合は何もしない
    if (keyFrames_.empty())
    {
        return;
    }

    // 最後のキーフレームより後の時間の場合
    auto lastKeyFrame = std::prev(keyFrames_.end());
    if (_currentTime >= lastKeyFrame->time)
    {
        isEnd_ = true; // イベントの終了フラグを立てる
    }

    // 型ごとに値を計算
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        value_ = CalculateValueAtTime<T>(_currentTime);
        }, value_);
}

void SequenceEvent::Save()
{
    jsonBinder_->SendVariable(label_ + "_keyFrames", keyFrames_);
}

void SequenceEvent::RegisterVariables()
{
    if (jsonBinder_)
    {
        jsonBinder_->GetVariableValue(label_ + "_keyFrames", keyFrames_);
        jsonBinder_->RegisterVariable(label_ + "_variableType", reinterpret_cast<uint32_t*>(&useType_));
    }
}

void SequenceEvent::SetJsonBinder(JsonBinder* _jsonBinder)
{
    jsonBinder_ = _jsonBinder;
    RegisterVariables();
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

    useType_ = CheckType(_value);

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

void SequenceEvent::EditKeyFrameValue([[maybe_unused]] KeyFrame& _keyFrame)
{
#ifdef _DEBUG
    static float speed = 1.0f;
    std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int32_t>) {
            ImGui::DragInt("Value", &arg);
        }
        else if constexpr (std::is_same_v<T, float>) {
            ImGui::DragFloat("Speed", &speed, 0.01f,0.01f);
            ImGui::DragFloat("Value", &arg,speed, 0.01f);
        }
        else if constexpr (std::is_same_v<T, Vector2>) {
            ImGui::DragFloat("Speed", &speed, 0.01f, 0.01f,10.0f);
            ImGui::DragFloat2("Value", &arg.x, speed, 0.01f);
        }
        else if constexpr (std::is_same_v<T, Vector3>) {
            ImGui::DragFloat("Speed", &speed, 0.01f, 0.01f, 10.0f);
            ImGui::DragFloat3("Value", &arg.x, speed, 0.01f);
        }
        else if constexpr (std::is_same_v<T, Vector4>) {
            static bool color = false;
            ImGui::Checkbox("Color", &color);
            ImGui::DragFloat("Speed", &speed, 0.01f, 0.01f, 10.0f);
            if (color) {
                ImGui::ColorEdit4("Value", &arg.x);
            }
            else
                ImGui::DragFloat4("Value", &arg.x, speed, 0.01f);
        }
        else if constexpr (std::is_same_v<T, Quaternion>) {
            ImGui::DragFloat("Speed", &speed, 0.01f, 0.01f, 10.0f);
            ImGui::DragFloat4("Value", &arg.x, speed, 0.01f);
        }
        else {
            throw std::runtime_error("Invalid type");
        }
        }, _keyFrame.value);
#endif // _DEBUG
}

void SequenceEvent::InitializeValueFromKeyFrames()
{
    if (!keyFrames_.empty())
    {
        value_ = keyFrames_.front().value;
        useType_ = CheckType(value_);
    }
    else
    {
        switch (useType_)
        {
        case UseType::Int:
            value_ = 0;
            break;
        case UseType::Float:
            value_ = 0.0f;
            break;
        case UseType::Vector2:
            value_ = Vector2(0.0f, 0.0f);
            break;
        case UseType::Vector3:
            value_ = Vector3(0.0f, 0.0f, 0.0f);
            break;
        case UseType::Vector4:
            value_ = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
            break;
        case UseType::Quaternion:
            value_ = Quaternion(0.0f, 0.0f, 0.0f, 1.0f); // 単位クォータニオン
            break;
        default:
            value_ = int32_t(0);
            useType_ = UseType::Int; // デフォルトは整数型
            break;
        }

    }



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

template<typename T>
T SequenceEvent::CalculateValueAtTime(float _time) const
{
    // キーフレームがない場合はデフォルト値を返す
    if (keyFrames_.empty())
    {
        return T();
    }

    // 最初のキーフレームより前の時間の場合
    auto firstKeyFrame = keyFrames_.begin();
    if (_time <= firstKeyFrame->time)
    {
        return std::get<T>(firstKeyFrame->value);
    }

    // 最後のキーフレームより後の時間の場合
    auto lastKeyFrame = std::prev(keyFrames_.end());
    if (_time >= lastKeyFrame->time)
    {
        return std::get<T>(lastKeyFrame->value);
    }

    // 2つのキーフレーム間の補間
    const KeyFrame* prevKeyFrame = nullptr;
    const KeyFrame* nextKeyFrame = nullptr;

    // 現在の時間を挟む2つのキーフレームを探す
    for (auto it = keyFrames_.begin(); it != keyFrames_.end(); ++it)
    {
        if (it->time > _time)
        {
            nextKeyFrame = &(*it);
            prevKeyFrame = &(*std::prev(it));
            break;
        }
    }

    if (!prevKeyFrame || !nextKeyFrame)
    {
        return T(); // エラー時はデフォルト値を返す
    }

    // 2つのキーフレーム間の補間係数を計算
    float totalTime = nextKeyFrame->time - prevKeyFrame->time;
    float t = (_time - prevKeyFrame->time) / totalTime;

    // イージング関数を適用
    t = Easing::SelectFuncPtr(nextKeyFrame->easingType)(t);

    // 型ごとに補間処理
    const T& prevValue = std::get<T>(prevKeyFrame->value);
    const T& nextValue = std::get<T>(nextKeyFrame->value);

    if constexpr (std::is_same_v<T, int32_t>) {
        // 整数値の補間（四捨五入）
        return static_cast<int32_t>(std::round(prevValue + (nextValue - prevValue) * t));
    }
    else if constexpr (std::is_same_v<T, float>) {
        // 浮動小数点の線形補間
        return prevValue + (nextValue - prevValue) * t;
    }
    else if constexpr (std::is_same_v<T, Vector2>) {
        // Vector2の補間
        return Vector2(
            prevValue.x + (nextValue.x - prevValue.x) * t,
            prevValue.y + (nextValue.y - prevValue.y) * t
        );
    }
    else if constexpr (std::is_same_v<T, Vector3>) {
        // Vector3の補間
        return Vector3(
            prevValue.x + (nextValue.x - prevValue.x) * t,
            prevValue.y + (nextValue.y - prevValue.y) * t,
            prevValue.z + (nextValue.z - prevValue.z) * t
        );
    }
    else if constexpr (std::is_same_v<T, Vector4>) {
        // Vector4の補間
        return Vector4(
            prevValue.x + (nextValue.x - prevValue.x) * t,
            prevValue.y + (nextValue.y - prevValue.y) * t,
            prevValue.z + (nextValue.z - prevValue.z) * t,
            prevValue.w + (nextValue.w - prevValue.w) * t
        );
    }
    else if constexpr (std::is_same_v<T, Quaternion>) {
        // Quaternionの球面線形補間
        return Quaternion::Slerp(prevValue, nextValue, t);
    }
}

// 明示的なテンプレートインスタンス化
template int32_t SequenceEvent::CalculateValueAtTime<int32_t>(float) const;
template float SequenceEvent::CalculateValueAtTime<float>(float) const;
template Vector2 SequenceEvent::CalculateValueAtTime<Vector2>(float) const;
template Vector3 SequenceEvent::CalculateValueAtTime<Vector3>(float) const;
template Vector4 SequenceEvent::CalculateValueAtTime<Vector4>(float) const;
template Quaternion SequenceEvent::CalculateValueAtTime<Quaternion>(float) const;
