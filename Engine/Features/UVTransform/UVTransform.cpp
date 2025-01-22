#include <Features/UVTransform/UVTransform.h>

#include <Math/Matrix/MatrixFunction.h>

UVTransform::UVTransform():
    uOffset_(0.0f),
    vOffset_(0.0f),
    uScale_(1.0f),
    vScale_(1.0f),
    rotation_(0.0f)
{
}

Matrix4x4 UVTransform::GetMatrix() const
{
    Matrix4x4 mat;

    Vector3 scale = { uScale_,vScale_,1 };
    Vector3 offset = { uOffset_,vOffset_,0 };
    Vector3 rotate = { 0,0,rotation_ };

    mat = MakeAffineMatrix(scale, rotate, offset);

    return mat;
}
