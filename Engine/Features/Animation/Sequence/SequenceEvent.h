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
public:
    SequenceEvent();
    ~SequenceEvent();

    void Update(float _currentTime);

private:
    using ParameterValue = std::variant<int32_t, float, Vector2, Vector3, Vector4, Quaternion>;

    float startTime_;
    float duration_;
    ParameterValue value_;
    uint32_t easingType_;





};
