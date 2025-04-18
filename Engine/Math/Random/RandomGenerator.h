#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <random>

class RandomGenerator {
public:
    static RandomGenerator* GetInstance() { static RandomGenerator instance; return &instance; };

    int GetRandValue(int min, int max);
    float GetRandValue(float _min, float _max);
    Vector2 GetRandValue(const Vector2& _min, const Vector2& _max);
    Vector3 GetRandValue(const Vector3& _min, const Vector3& _max);
    Vector4 GetRandValue(const Vector4& _min, const Vector4& _max);
    Vector4 GetUniformColor();
    float GetUniformAngle();
    double GetUniformDouble();

private:
    RandomGenerator();

    RandomGenerator(const RandomGenerator&) = delete;
    RandomGenerator& operator=(const RandomGenerator&) = delete;

    // 乱数生成エンジン
    std::mt19937 engine;
};
