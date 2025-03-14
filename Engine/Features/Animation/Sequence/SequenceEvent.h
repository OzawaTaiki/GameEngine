#pragma once


#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Quaternion/Quaternion.h>
#include <Math/Easing.h>

#include <list>
#include <variant>
#include <cstdint>

class SequenceEvent
{
private:
    using ParameterValue = std::variant<int32_t, float, Vector2, Vector3, Vector4, Quaternion>;
public:

    struct KeyFrame
    {
        float time;
        ParameterValue value;
        uint32_t easingType;
        bool isSelect;
        bool deleteFlag;
    };

public:
    SequenceEvent() = default;
    SequenceEvent(const std::string& _label, ParameterValue _value);
    ~SequenceEvent() = default;

    void Update(float _currentTime);

    std::string GetLabel() const { return label_; }
    void SetSelect(bool _isSelect) { isSelect_ = _isSelect; }
    bool IsSelect() const { return isSelect_; }

    void ClearSelectKeyFrames();

    std::list<KeyFrame>& GetKeyFrames() { return keyFrames_; }
    void AddKeyFrame(float _time, ParameterValue _value, uint32_t _easingType);
    void AddKeyFrame(float _time);

    void DeleteMarkedKeyFrame();

    static void EditKeyFrameValue(KeyFrame& _keyFrame);


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



    std::list<KeyFrame> keyFrames_;

};
