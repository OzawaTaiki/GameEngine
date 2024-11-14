#pragma once

#include "Vector3.h"
#include "Quaternion.h"

struct QuaternionTransform
{
    Vector3 translate;
    Quaternion rotation;
    Vector3 scale;
};