#include <Math/Random/RandomGenerator.h>

#include <numbers>

RandomGenerator::RandomGenerator() : engine(std::random_device{}()) {
    std::random_device rd;
    engine.seed(rd());
}

int RandomGenerator::GetRandValue(int min, int max) {
    std::uniform_int_distribution<> dist(min, max);
    return dist(engine);
}

float RandomGenerator::GetRandValue(float _min, float _max)
{
    if (_min > _max)
        std::swap(_min, _max);
    std::uniform_real_distribution<float> dist(_min, _max);
    return dist(engine);
}

Vector2 RandomGenerator::GetRandValue(const Vector2& _min, const Vector2& _max)
{
    Vector2 result;
    result.x = GetRandValue(_min.x, _max.x);
    result.y = GetRandValue(_min.y, _max.y);
    return result;
}

Vector3 RandomGenerator::GetRandValue(const Vector3& _min, const Vector3& _max)
{
    Vector3 result;
    result.x = GetRandValue(_min.x, _max.x);
    result.y = GetRandValue(_min.y, _max.y);
    result.z = GetRandValue(_min.z, _max.z);
    return result;
}

Vector4 RandomGenerator::GetRandValue(const Vector4& _min, const Vector4& _max)
{
    Vector4 result;
    result.x = GetRandValue(_min.x, _max.x);
    result.y = GetRandValue(_min.y, _max.y);
    result.z = GetRandValue(_min.z, _max.z);
    result.w = GetRandValue(_min.w, _max.w);
    return result;
}

Vector4 RandomGenerator::GetUniformColor()
{
    Vector4 result;
    result = { GetRandValue({0,0,0},{1,1,1}),1 };
    return result;
}

float RandomGenerator::GetUniformAngle()
{
    std::uniform_real_distribution<float> dist(0.0f, std::numbers::pi_v<float> *2.0f);
    return dist(engine);
}


double RandomGenerator::GetUniformDouble() {
    std::uniform_real_distribution<> dist(0.0, 1.0);
    return dist(engine);
}
