#include "MatrixFunction.h"
#include <cmath>
#include <VectorFunction.h>



Matrix4x4  Add(const Matrix4x4& _m1, const Matrix4x4& _m2)
{
    Matrix4x4 result;
    result.m[0][0] = _m1.m[0][0] + _m2.m[0][0];
    result.m[0][1] = _m1.m[0][1] + _m2.m[0][1];
    result.m[0][2] = _m1.m[0][2] + _m2.m[0][2];
    result.m[0][3] = _m1.m[0][3] + _m2.m[0][3];

    result.m[1][0] = _m1.m[1][0] + _m2.m[1][0];
    result.m[1][1] = _m1.m[1][1] + _m2.m[1][1];
    result.m[1][2] = _m1.m[1][2] + _m2.m[1][2];
    result.m[1][3] = _m1.m[1][3] + _m2.m[1][3];

    result.m[2][0] = _m1.m[2][0] + _m2.m[2][0];
    result.m[2][1] = _m1.m[2][1] + _m2.m[2][1];
    result.m[2][2] = _m1.m[2][2] + _m2.m[2][2];
    result.m[2][3] = _m1.m[2][3] + _m2.m[2][3];

    result.m[3][0] = _m1.m[3][0] + _m2.m[3][0];
    result.m[3][1] = _m1.m[3][1] + _m2.m[3][1];
    result.m[3][2] = _m1.m[3][2] + _m2.m[3][2];
    result.m[3][3] = _m1.m[3][3] + _m2.m[3][3];

    return result;
}

Matrix4x4  Subtract(const Matrix4x4& _m1, const Matrix4x4& _m2)
{
    Matrix4x4 result;
    result.m[0][0] = _m1.m[0][0] - _m2.m[0][0];
    result.m[0][1] = _m1.m[0][1] - _m2.m[0][1];
    result.m[0][2] = _m1.m[0][2] - _m2.m[0][2];
    result.m[0][3] = _m1.m[0][3] - _m2.m[0][3];

    result.m[1][0] = _m1.m[1][0] - _m2.m[1][0];
    result.m[1][1] = _m1.m[1][1] - _m2.m[1][1];
    result.m[1][2] = _m1.m[1][2] - _m2.m[1][2];
    result.m[1][3] = _m1.m[1][3] - _m2.m[1][3];

    result.m[2][0] = _m1.m[2][0] - _m2.m[2][0];
    result.m[2][1] = _m1.m[2][1] - _m2.m[2][1];
    result.m[2][2] = _m1.m[2][2] - _m2.m[2][2];
    result.m[2][3] = _m1.m[2][3] - _m2.m[2][3];

    result.m[3][0] = _m1.m[3][0] - _m2.m[3][0];
    result.m[3][1] = _m1.m[3][1] - _m2.m[3][1];
    result.m[3][2] = _m1.m[3][2] - _m2.m[3][2];
    result.m[3][3] = _m1.m[3][3] - _m2.m[3][3];

    return result;
}

Matrix4x4  Multiply(const Matrix4x4& _m1, const Matrix4x4& _m2)
{
    Matrix4x4 result;
    result.m[0][0] = _m1.m[0][0] * _m2.m[0][0] + _m1.m[0][1] * _m2.m[1][0] + _m1.m[0][2] * _m2.m[2][0] + _m1.m[0][3] * _m2.m[3][0];
    result.m[0][1] = _m1.m[0][0] * _m2.m[0][1] + _m1.m[0][1] * _m2.m[1][1] + _m1.m[0][2] * _m2.m[2][1] + _m1.m[0][3] * _m2.m[3][1];
    result.m[0][2] = _m1.m[0][0] * _m2.m[0][2] + _m1.m[0][1] * _m2.m[1][2] + _m1.m[0][2] * _m2.m[2][2] + _m1.m[0][3] * _m2.m[3][2];
    result.m[0][3] = _m1.m[0][0] * _m2.m[0][3] + _m1.m[0][1] * _m2.m[1][3] + _m1.m[0][2] * _m2.m[2][3] + _m1.m[0][3] * _m2.m[3][3];

    result.m[1][0] = _m1.m[1][0] * _m2.m[0][0] + _m1.m[1][1] * _m2.m[1][0] + _m1.m[1][2] * _m2.m[2][0] + _m1.m[1][3] * _m2.m[3][0];
    result.m[1][1] = _m1.m[1][0] * _m2.m[0][1] + _m1.m[1][1] * _m2.m[1][1] + _m1.m[1][2] * _m2.m[2][1] + _m1.m[1][3] * _m2.m[3][1];
    result.m[1][2] = _m1.m[1][0] * _m2.m[0][2] + _m1.m[1][1] * _m2.m[1][2] + _m1.m[1][2] * _m2.m[2][2] + _m1.m[1][3] * _m2.m[3][2];
    result.m[1][3] = _m1.m[1][0] * _m2.m[0][3] + _m1.m[1][1] * _m2.m[1][3] + _m1.m[1][2] * _m2.m[2][3] + _m1.m[1][3] * _m2.m[3][3];

    result.m[2][0] = _m1.m[2][0] * _m2.m[0][0] + _m1.m[2][1] * _m2.m[1][0] + _m1.m[2][2] * _m2.m[2][0] + _m1.m[2][3] * _m2.m[3][0];
    result.m[2][1] = _m1.m[2][0] * _m2.m[0][1] + _m1.m[2][1] * _m2.m[1][1] + _m1.m[2][2] * _m2.m[2][1] + _m1.m[2][3] * _m2.m[3][1];
    result.m[2][2] = _m1.m[2][0] * _m2.m[0][2] + _m1.m[2][1] * _m2.m[1][2] + _m1.m[2][2] * _m2.m[2][2] + _m1.m[2][3] * _m2.m[3][2];
    result.m[2][3] = _m1.m[2][0] * _m2.m[0][3] + _m1.m[2][1] * _m2.m[1][3] + _m1.m[2][2] * _m2.m[2][3] + _m1.m[2][3] * _m2.m[3][3];

    result.m[3][0] = _m1.m[3][0] * _m2.m[0][0] + _m1.m[3][1] * _m2.m[1][0] + _m1.m[3][2] * _m2.m[2][0] + _m1.m[3][3] * _m2.m[3][0];
    result.m[3][1] = _m1.m[3][0] * _m2.m[0][1] + _m1.m[3][1] * _m2.m[1][1] + _m1.m[3][2] * _m2.m[2][1] + _m1.m[3][3] * _m2.m[3][1];
    result.m[3][2] = _m1.m[3][0] * _m2.m[0][2] + _m1.m[3][1] * _m2.m[1][2] + _m1.m[3][2] * _m2.m[2][2] + _m1.m[3][3] * _m2.m[3][2];
    result.m[3][3] = _m1.m[3][0] * _m2.m[0][3] + _m1.m[3][1] * _m2.m[1][3] + _m1.m[3][2] * _m2.m[2][3] + _m1.m[3][3] * _m2.m[3][3];
    return result;
}

Matrix4x4  Inverse(const Matrix4x4& _m)
{
    float denominator =
        _m.m[0][0] * _m.m[1][1] * _m.m[2][2] * _m.m[3][3]
        + _m.m[0][0] * _m.m[1][2] * _m.m[2][3] * _m.m[3][1]
        + _m.m[0][0] * _m.m[1][3] * _m.m[2][1] * _m.m[3][2]

        - _m.m[0][0] * _m.m[1][3] * _m.m[2][2] * _m.m[3][1]
        - _m.m[0][0] * _m.m[1][2] * _m.m[2][1] * _m.m[3][3]
        - _m.m[0][0] * _m.m[1][1] * _m.m[2][3] * _m.m[3][2]

        - _m.m[0][1] * _m.m[1][0] * _m.m[2][2] * _m.m[3][3]
        - _m.m[0][2] * _m.m[1][0] * _m.m[2][3] * _m.m[3][1]
        - _m.m[0][3] * _m.m[1][0] * _m.m[2][1] * _m.m[3][2]

        + _m.m[0][3] * _m.m[1][0] * _m.m[2][2] * _m.m[3][1]
        + _m.m[0][2] * _m.m[1][0] * _m.m[2][1] * _m.m[3][3]
        + _m.m[0][1] * _m.m[1][0] * _m.m[2][3] * _m.m[3][2]

        + _m.m[0][1] * _m.m[1][2] * _m.m[2][0] * _m.m[3][3]
        + _m.m[0][2] * _m.m[1][3] * _m.m[2][0] * _m.m[3][1]
        + _m.m[0][3] * _m.m[1][1] * _m.m[2][0] * _m.m[3][2]

        - _m.m[0][3] * _m.m[1][2] * _m.m[2][0] * _m.m[3][1]
        - _m.m[0][2] * _m.m[1][1] * _m.m[2][0] * _m.m[3][3]
        - _m.m[0][1] * _m.m[1][3] * _m.m[2][0] * _m.m[3][2]

        - _m.m[0][1] * _m.m[1][2] * _m.m[2][3] * _m.m[3][0]
        - _m.m[0][2] * _m.m[1][3] * _m.m[2][1] * _m.m[3][0]
        - _m.m[0][3] * _m.m[1][1] * _m.m[2][2] * _m.m[3][0]

        + _m.m[0][3] * _m.m[1][2] * _m.m[2][1] * _m.m[3][0]
        + _m.m[0][2] * _m.m[1][1] * _m.m[2][3] * _m.m[3][0]
        + _m.m[0][1] * _m.m[1][3] * _m.m[2][2] * _m.m[3][0];

    Matrix4x4 result;

    result.m[0][0] =
        (_m.m[1][1] * _m.m[2][2] * _m.m[3][3]
            + _m.m[1][2] * _m.m[2][3] * _m.m[3][1]
            + _m.m[1][3] * _m.m[2][1] * _m.m[3][2]
            - _m.m[1][3] * _m.m[2][2] * _m.m[3][1]
            - _m.m[1][2] * _m.m[2][1] * _m.m[3][3]
            - _m.m[1][1] * _m.m[2][3] * _m.m[3][2])
        / denominator;


    result.m[0][1] =
        (-_m.m[0][1] * _m.m[2][2] * _m.m[3][3]
            - _m.m[0][2] * _m.m[2][3] * _m.m[3][1]
            - _m.m[0][3] * _m.m[2][1] * _m.m[3][2]
            + _m.m[0][3] * _m.m[2][2] * _m.m[3][1]
            + _m.m[0][2] * _m.m[2][1] * _m.m[3][3]
            + _m.m[0][1] * _m.m[2][3] * _m.m[3][2])
        / denominator;

    result.m[0][2] =
        (_m.m[0][1] * _m.m[1][2] * _m.m[3][3]
            + _m.m[0][2] * _m.m[1][3] * _m.m[3][1]
            + _m.m[0][3] * _m.m[1][1] * _m.m[3][2]
            - _m.m[0][3] * _m.m[1][2] * _m.m[3][1]
            - _m.m[0][2] * _m.m[1][1] * _m.m[3][3]
            - _m.m[0][1] * _m.m[1][3] * _m.m[3][2])
        / denominator;
    result.m[0][3] =
        (-_m.m[0][1] * _m.m[1][2] * _m.m[2][3]
            - _m.m[0][2] * _m.m[1][3] * _m.m[2][1]
            - _m.m[0][3] * _m.m[1][1] * _m.m[2][2]
            + _m.m[0][3] * _m.m[1][2] * _m.m[2][1]
            + _m.m[0][2] * _m.m[1][1] * _m.m[2][3]
            + _m.m[0][1] * _m.m[1][3] * _m.m[2][2])
        / denominator;

    result.m[1][0] =
        (-_m.m[1][0] * _m.m[2][2] * _m.m[3][3]
            - _m.m[1][2] * _m.m[2][3] * _m.m[3][0]
            - _m.m[1][3] * _m.m[2][0] * _m.m[3][2]
            + _m.m[1][3] * _m.m[2][2] * _m.m[3][0]
            + _m.m[1][2] * _m.m[2][0] * _m.m[3][3]
            + _m.m[1][0] * _m.m[2][3] * _m.m[3][2])
        / denominator;
    result.m[1][1] =
        (_m.m[0][0] * _m.m[2][2] * _m.m[3][3]
            + _m.m[0][2] * _m.m[2][3] * _m.m[3][0]
            + _m.m[0][3] * _m.m[2][0] * _m.m[3][2]
            - _m.m[0][3] * _m.m[2][2] * _m.m[3][0]
            - _m.m[0][2] * _m.m[2][0] * _m.m[3][3]
            - _m.m[0][0] * _m.m[2][3] * _m.m[3][2])
        / denominator;
    result.m[1][2] =
        (-_m.m[0][0] * _m.m[1][2] * _m.m[3][3]
            - _m.m[0][2] * _m.m[1][3] * _m.m[3][0]
            - _m.m[0][3] * _m.m[1][0] * _m.m[3][2]
            + _m.m[0][3] * _m.m[1][2] * _m.m[3][0]
            + _m.m[0][2] * _m.m[1][0] * _m.m[3][3]
            + _m.m[0][0] * _m.m[1][3] * _m.m[3][2])
        / denominator;
    result.m[1][3] =
        (_m.m[0][0] * _m.m[1][2] * _m.m[2][3]
            + _m.m[0][2] * _m.m[1][3] * _m.m[2][0]
            + _m.m[0][3] * _m.m[1][0] * _m.m[2][2]
            - _m.m[0][3] * _m.m[1][2] * _m.m[2][0]
            - _m.m[0][2] * _m.m[1][0] * _m.m[2][3]
            - _m.m[0][0] * _m.m[1][3] * _m.m[2][2])
        / denominator;

    result.m[2][0] =
        (_m.m[1][0] * _m.m[2][1] * _m.m[3][3]
            + _m.m[1][1] * _m.m[2][3] * _m.m[3][0]
            + _m.m[1][3] * _m.m[2][0] * _m.m[3][1]
            - _m.m[1][3] * _m.m[2][1] * _m.m[3][0]
            - _m.m[1][1] * _m.m[2][0] * _m.m[3][3]
            - _m.m[1][0] * _m.m[2][3] * _m.m[3][1])
        / denominator;
    result.m[2][1] =
        (-_m.m[0][0] * _m.m[2][1] * _m.m[3][3]
            - _m.m[0][1] * _m.m[2][3] * _m.m[3][0]
            - _m.m[0][3] * _m.m[2][0] * _m.m[3][1]
            + _m.m[0][3] * _m.m[2][1] * _m.m[3][0]
            + _m.m[0][1] * _m.m[2][0] * _m.m[3][3]
            + _m.m[0][0] * _m.m[2][3] * _m.m[3][1])
        / denominator;
    result.m[2][2] =
        (_m.m[0][0] * _m.m[1][1] * _m.m[3][3]
            + _m.m[0][1] * _m.m[1][3] * _m.m[3][0]
            + _m.m[0][3] * _m.m[1][0] * _m.m[3][1]
            - _m.m[0][3] * _m.m[1][1] * _m.m[3][0]
            - _m.m[0][1] * _m.m[1][0] * _m.m[3][3]
            - _m.m[0][0] * _m.m[1][3] * _m.m[3][1])
        / denominator;
    result.m[2][3] =
        (-_m.m[0][0] * _m.m[1][1] * _m.m[2][3]
            - _m.m[0][1] * _m.m[1][3] * _m.m[2][0]
            - _m.m[0][3] * _m.m[1][0] * _m.m[2][1]
            + _m.m[0][3] * _m.m[1][1] * _m.m[2][0]
            + _m.m[0][1] * _m.m[1][0] * _m.m[2][3]
            + _m.m[0][0] * _m.m[1][3] * _m.m[2][1])
        / denominator;

    result.m[3][0] =
        (-_m.m[1][0] * _m.m[2][1] * _m.m[3][2]
            - _m.m[1][1] * _m.m[2][2] * _m.m[3][0]
            - _m.m[1][2] * _m.m[2][0] * _m.m[3][1]
            + _m.m[1][2] * _m.m[2][1] * _m.m[3][0]
            + _m.m[1][1] * _m.m[2][0] * _m.m[3][2]
            + _m.m[1][0] * _m.m[2][2] * _m.m[3][1])
        / denominator;
    result.m[3][1] =
        (_m.m[0][0] * _m.m[2][1] * _m.m[3][2]
            + _m.m[0][1] * _m.m[2][2] * _m.m[3][0]
            + _m.m[0][2] * _m.m[2][0] * _m.m[3][1]
            - _m.m[0][2] * _m.m[2][1] * _m.m[3][0]
            - _m.m[0][1] * _m.m[2][0] * _m.m[3][2]
            - _m.m[0][0] * _m.m[2][2] * _m.m[3][1])
        / denominator;
    result.m[3][2] =
        (-_m.m[0][0] * _m.m[1][1] * _m.m[3][2]
            - _m.m[0][1] * _m.m[1][2] * _m.m[3][0]
            - _m.m[0][2] * _m.m[1][0] * _m.m[3][1]
            + _m.m[0][2] * _m.m[1][1] * _m.m[3][0]
            + _m.m[0][1] * _m.m[1][0] * _m.m[3][2]
            + _m.m[0][0] * _m.m[1][2] * _m.m[3][1])
        / denominator;
    result.m[3][3] =
        (_m.m[0][0] * _m.m[1][1] * _m.m[2][2]
            + _m.m[0][1] * _m.m[1][2] * _m.m[2][0]
            + _m.m[0][2] * _m.m[1][0] * _m.m[2][1]
            - _m.m[0][2] * _m.m[1][1] * _m.m[2][0]
            - _m.m[0][1] * _m.m[1][0] * _m.m[2][2]
            - _m.m[0][0] * _m.m[1][2] * _m.m[2][1])
        / denominator;

    return result;
}

Matrix4x4  Transpose(const Matrix4x4& _m)
{
    Matrix4x4 result;

    result.m[0][0] = _m.m[0][0];
    result.m[0][1] = _m.m[1][0];
    result.m[0][2] = _m.m[2][0];
    result.m[0][3] = _m.m[3][0];

    result.m[1][0] = _m.m[0][1];
    result.m[1][1] = _m.m[1][1];
    result.m[1][2] = _m.m[2][1];
    result.m[1][3] = _m.m[3][1];

    result.m[2][0] = _m.m[0][2];
    result.m[2][1] = _m.m[1][2];
    result.m[2][2] = _m.m[2][2];
    result.m[2][3] = _m.m[3][2];

    result.m[3][0] = _m.m[0][3];
    result.m[3][1] = _m.m[1][3];
    result.m[3][2] = _m.m[2][3];
    result.m[3][3] = _m.m[3][3];

    return result;
}

Matrix4x4  MakeIdentity4x4()
{
    Matrix4x4 result =
    {
        {
            {1,0,0,0},
            {0,1,0,0},
            {0,0,1,0},
            {0,0,0,1}
        }
    };

    return result;
}

Matrix4x4  MakeScaleMatrix(const Vector3& _scale)
{
    Matrix4x4 result =
    {
        {
            {_scale.x,0,0,0},
            {0,_scale.y,0,0},
            {0,0,_scale.z,0},
            {0,0,0,1}
        }
    };

    return result;
}

Matrix4x4  MakeTranslateMatrix(const Vector3& _translate)
{
    Matrix4x4 result =
    {
        {
            {1,0,0,0},
            {0,1,0,0},
            {0,0,1,0},
            {_translate.x,_translate.y,_translate.z,1}
        }
    };

    return result;
}

Matrix4x4  MakeRotateXMatrix(float _radian)
{
    Matrix4x4 result =
    {
        {
            {1,0,0,0},
            {0,std::cos(_radian),std::sin(_radian),0},
            {0,-std::sin(_radian),std::cos(_radian),0},
            {0,0,0,1}
        }
    };
    return result;
}

Matrix4x4  MakeRotateYMatrix(float _radian)
{
    Matrix4x4 result =
    {
        {
            {std::cos(_radian),0,-std::sin(_radian),0},
            {0,1,0,0},
            {std::sin(_radian),0,std::cos(_radian),0},
            {0,0,0,1}
        }
    };
    return result;
}

Matrix4x4  MakeRotateZMatrix(float _radian)
{
    Matrix4x4 result =
    {
        {
            {std::cos(_radian),std::sin(_radian),0,0},
            {-std::sin(_radian),std::cos(_radian),0,0},
            {0,0,1,0},
            {0,0,0,1}
        }
    };
    return result;
}

Matrix4x4  MakeRotateMatrix(const Vector3& _rotate)
{
    Matrix4x4 rotateX = MakeRotateXMatrix(_rotate.x);
    Matrix4x4 rotateY = MakeRotateYMatrix(_rotate.y);
    Matrix4x4 rotateZ = MakeRotateZMatrix(_rotate.z);

    Matrix4x4 result = Multiply(rotateX, Multiply(rotateY, rotateZ));

    return result;
}

Matrix4x4  MakeAffineMatrix(const Vector3& _scale, const Vector3& _rotate, const Vector3& _translate)
{
    Matrix4x4 _worldMat = MakeRotateMatrix(_rotate);

    Matrix4x4 result =
    {
        {
            {_scale.x * _worldMat.m[0][0],_scale.x * _worldMat.m[0][1],_scale.x * _worldMat.m[0][2],0},
            {_scale.y * _worldMat.m[1][0],_scale.y * _worldMat.m[1][1],_scale.y * _worldMat.m[1][2],0},
            {_scale.z * _worldMat.m[2][0],_scale.z * _worldMat.m[2][1],_scale.z * _worldMat.m[2][2],0},
            {_translate.x,_translate.y,_translate.z,1}
        }
    };

    return result;
}

Matrix4x4 MakeAffineMatrix(const Vector3& _scale, const Quaternion& _rotate, const Vector3& _translate)
{
    Matrix4x4 result = MakeIdentity4x4();

    result *= MakeScaleMatrix(_scale);
    result *= _rotate.ToMatrix();
    result *= MakeTranslateMatrix(_translate);

    return result;
}

Matrix4x4  MakePerspectiveFovMatrix(float _fovY, float _aspectRatio, float _nearClip, float _farClip)
{
    Matrix4x4 result =
    {
        {
            {1.0f / _aspectRatio * std::cosf(_fovY / 2.0f) / std::sinf(_fovY / 2.0f),0,0,0},
            {0,std::cosf(_fovY / 2.0f) / std::sinf(_fovY / 2.0f),0,0},
            {0,0,_farClip / (_farClip - _nearClip),1},
            {0,0,(-_nearClip * _farClip) / (_farClip - _nearClip),0}
        }
    };

    return result;
}

Matrix4x4  MakeOrthographicMatrix(float _left, float _top, float _right, float _bottom, float _nearClip, float _farClip)
{
    Matrix4x4 result =
    {
        {
            {2.0f / (_right - _left),0,0,0},
            {0,2.0f / (_top - _bottom),0,0},
            {0,0,1.0f / (_farClip - _nearClip),0},
            {(_left + _right) / (_left - _right),(_top + _bottom) / (_bottom - _top),_nearClip / (_nearClip - _farClip),1}
        }
    };

    return result;
}

Matrix4x4  MakeViewportMatrix(float _left, float _top, float _width, float _height, float _minDepth, float _maxDepth)
{
    Matrix4x4 result =
    {
        {
            {_width / 2.0f,0,0,0},
            {0,-_height / 2.0f,0,0},
            {0,0,_maxDepth - _minDepth,0},
            {_left + _width / 2.0f,_top + _height / 2.0f,_minDepth,1}
        }
    };

    return result;
}

Matrix4x4 MakeRotateAxisAngle(const Vector3& _axis, float _angle)
{
    Matrix4x4 result;

    result.m[0][0] = std::cos(_angle) + _axis.x * _axis.x * (1 - std::cos(_angle));
    result.m[0][1] = _axis.x * _axis.y * (1 - std::cos(_angle)) + _axis.z * std::sin(_angle);
    result.m[0][2] = _axis.x * _axis.z * (1 - std::cos(_angle)) - _axis.y * std::sin(_angle);
    result.m[0][3] = 0;

    result.m[1][0] = _axis.x * _axis.y * (1 - std::cos(_angle)) - _axis.z * std::sin(_angle);
    result.m[1][1] = std::cos(_angle) + _axis.y * _axis.y * (1 - std::cos(_angle));
    result.m[1][2] = _axis.y * _axis.z * (1 - std::cos(_angle)) + _axis.x * std::sin(_angle);
    result.m[1][3] = 0;

    result.m[2][0] = _axis.x * _axis.z * (1 - std::cos(_angle)) + _axis.y * std::sin(_angle);
    result.m[2][1] = _axis.y * _axis.z * (1 - std::cos(_angle)) - _axis.x * std::sin(_angle);
    result.m[2][2] = std::cos(_angle) + _axis.z * _axis.z * (1 - std::cos(_angle));
    result.m[2][3] = 0;

    result.m[3][0] = 0;
    result.m[3][1] = 0;
    result.m[3][2] = 0;
    result.m[3][3] = 1;


    return result;
}

Matrix4x4 DirectionToDirection(const Vector3& _from, const Vector3& _to)
{
    Vector3 axis;
    if (_from == -_to)
    {
        if (_from.x != 0 || _from.y != 0)
            axis = Vector3(_from.y, -_from.x, 0);
        else if (_from.x != 0 || _from.z != 0)
            axis = Vector3(_from.z, 0, -_from.x);
    }
    else
        axis = _from.Cross(_to);

    axis = axis.Normalize();
    float cosAngle = _from.Dot(_to);
    float sinAngle = _from.Cross(_to).Length();

    Matrix4x4 result;

    result.m[0][0] = cosAngle + axis.x * axis.x * (1 - cosAngle);
    result.m[0][1] = axis.x * axis.y * (1 - cosAngle) + axis.z * sinAngle;
    result.m[0][2] = axis.x * axis.z * (1 - cosAngle) - axis.y * sinAngle;
    result.m[0][3] = 0;

    result.m[1][0] = axis.x * axis.y * (1 - cosAngle) - axis.z * sinAngle;
    result.m[1][1] = cosAngle + axis.y * axis.y * (1 - cosAngle);
    result.m[1][2] = axis.y * axis.z * (1 - cosAngle) + axis.x * sinAngle;
    result.m[1][3] = 0;

    result.m[2][0] = axis.x * axis.z * (1 - cosAngle) + axis.y * sinAngle;
    result.m[2][1] = axis.y * axis.z * (1 - cosAngle) - axis.x * sinAngle;
    result.m[2][2] = cosAngle + axis.z * axis.z * (1 - cosAngle);
    result.m[2][3] = 0;

    result.m[3][0] = 0;
    result.m[3][1] = 0;
    result.m[3][2] = 0;
    result.m[3][3] = 1;

    return result;
}

Matrix4x4 operator*(const Matrix4x4& _mat1, const Matrix4x4& _mat2)
{
    return Matrix4x4(Multiply(_mat1, _mat2));
}
