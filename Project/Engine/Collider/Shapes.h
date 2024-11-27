#pragma once

#include "Vector3.h"
#include "Matrix4x4.h"

#include <array>
#include <algorithm>

struct Sphere
{
    float radius;
    Vector3 center;
    Vector3 referencePoint;
};

struct AABB
{
    Vector3 min;
    Vector3 max;
    Vector3 referencePoint;
    Vector3 center;
    std::array <Vector3, 8> vertices;
    Matrix4x4 worldMat;

    AABB() = default;
    AABB(const float* _min, const float* _max) : referencePoint{ 0.0f }, center{ 0.0f }, vertices{ 0.0f }
    {
        localMin = _min;
        localMax = _max;

        min = localMin;
        max = localMax;

        for (int i = 0; i < 3; ++i)
        {
            if (min[0] > max[0])
            {
                std::swap(min[i], max[i]);
            }
        }


    };
    void Calculate(const Matrix4x4& _worldMat);

private:
    // center基準のmin
    Vector3 localMin;
    // center基準のmax
    Vector3 localMax;

    void CalculateMinMax();
    void CalculateVertices();
};

struct OBB
{
    // 最小値
    Vector3 min;
    // 最大値
    Vector3 max;
    // 基準点
    Vector3 referencePoint;
    // 各軸の方向ベクトル
    std::array <Vector3, 3> orientations;
    Vector3 center;
    Vector3 size;

    Matrix4x4 worldMat;

    std::array <Vector3, 8>vertices;

    OBB() = default;
    OBB(const float* _min, const float* _max) : referencePoint{ 0.0f, 0.0f, 0.0f },
        orientations{
        Vector3(1.0f, 0.0f, 0.0f),
        Vector3(0.0f, 1.0f, 0.0f),
        Vector3(0.0f, 0.0f, 1.0f) },
        vertices{ 0.0f }
    {
        localMin = _min;
        localMax = _max;
        size = localMax - localMin;
        min = localMin;
        max = localMax;
        center = (min + max) * 0.5f;
        Calculate(Matrix4x4::Identity());
    };

    void Calculate(const Matrix4x4& _worldMat);

private:
    // center基準のmin
    Vector3 localMin;
    // center基準のmax
    Vector3 localMax;

    void CalculateOrientations();

    void CalculateMinMax();

    void CalculateVertices();
};