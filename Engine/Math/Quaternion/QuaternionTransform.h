#pragma once

#include <Math/Vector/Vector3.h>
#include <Math/Quaternion/Quaternion.h>

namespace Engine {

struct QuaternionTransform
{
    Vector3 translate;
    Quaternion rotation;
    Vector3 scale;
};

} // namespace Engine
