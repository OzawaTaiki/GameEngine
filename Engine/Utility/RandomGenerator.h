#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <random>

class RandomGenerator {
public:
    static RandomGenerator* GetInstance() { static RandomGenerator instance; return &instance; };

    int GetUniformInt(int min, int max);
    float GetUniformFloat(float _min, float _max);
    Vector2 GetUniformVec2(const Vector2& _min, const Vector2& _max);
    Vector3 GetUniformVec3(const Vector3& _min, const Vector3& _max);
    Vector4 GetUniformVec4(const Vector4& _min, const Vector4& _max);
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