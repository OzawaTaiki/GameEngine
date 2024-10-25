#include "Vector2.h"

float Vector2::Length() const
{
    return std::sqrtf(x * x + y * y);
}
