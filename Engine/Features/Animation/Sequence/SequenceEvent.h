#pragma once


#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Quaternion/Quaternion.h>
#include <Math/Easing.h>


#include <list>
#include <variant>
#include <cstdint>
#include <typeindex>

using ParameterValue = std::variant<int32_t, float, Vector2, Vector3, Vector4, Quaternion>;

class JsonBinder;
class SequenceEvent
{
public:

    struct KeyFrame
    {
        float time;
        ParameterValue value;
        uint32_t easingType;
        bool isSelect;
        bool isDelete;
    };

public:
    SequenceEvent() = default;
    SequenceEvent(const std::string& _label,JsonBinder* _jsonBinder = nullptr);
    SequenceEvent(const std::string& _label, ParameterValue _value, JsonBinder* _jsonBinder = nullptr);
    ~SequenceEvent() = default;

    void Initialize(const std::string& _label = "");

    void Update(float _currentTime);

    void Save();
    void RegisterVariables();
    void SetJsonBinder(JsonBinder* _jsonBinder);

    std::string GetLabel() const { return label_; }
    void SetSelect(bool _isSelect) { isSelect_ = _isSelect; }
    bool IsSelect() const { return isSelect_; }

    template<typename T>
    T GetValue() const { return std::get<T>(value_); }
    ParameterValue GetValue() const { return value_; }

    void ClearSelectKeyFrames();

    std::list<KeyFrame>& GetKeyFrames() { return keyFrames_; }
    const std::list<KeyFrame>& GetKeyFrames() const { return keyFrames_; }
    void AddKeyFrame(float _time, ParameterValue _value, uint32_t _easingType);
    void AddKeyFrame(float _time);

    void InsertKeyFrame(const KeyFrame& _keyFrame);
    void SortKeyFrames();

    void DeleteMarkedKeyFrame();

    bool IsEnd() const { return isEnd_; }

    void MarkForDelete();
    bool IsDelete() const { return isDelete_; }

    void SetLabel(const std::string& _label) { label_ = _label; }
    void SetKeyFrames(const std::list<KeyFrame>& _keyFrames) { keyFrames_ = _keyFrames; }

    static void EditKeyFrameValue(KeyFrame& _keyFrame);


private:

    void InitializeValueFromKeyFrames();

private:
    enum class UseType {
        Int,
        Float,
        Vector2,
        Vector3,
        Vector4,
        Quaternion,

        Error
    };

    UseType useType_;

    // 型のチェックを行うため
    UseType CheckType(ParameterValue _value);


    std::string label_;
    bool isSelect_;
    bool isDelete_;
    bool isEnd_ = false; // イベントの終了フラグ
    ParameterValue value_; // 補間した値

    std::list<KeyFrame> keyFrames_;

    JsonBinder* jsonBinder_ = nullptr;

public:
};
