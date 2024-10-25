#include "Vector3.h"

float Vector3::Length() const
{
    return std::sqrtf(x * x + y * y + z * z);

}
