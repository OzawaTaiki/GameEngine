#include "DebugCamera.h"
#include "Camera.h"
#include "Input.h"
#include "VectorFunction.h"
#include "MatrixFunction.h"

void DebugCamera::Initialize()
{
}

void DebugCamera::Update()
{
    Vector3 move;/*
    Input::GetInstance()->GetMove(move,0.1f);
    translate_.y += move.y;
    move.y = 0;
    Vector3 rot;
    Input::GetInstance()->GetRotate(rot);
    if (Input::GetInstance()->IsMousePressed(1))
        rotation_ += rot;
    Matrix4x4 matRot = MakeRotateMatrix(rotation_);


    Vector3 rotVelo = TransformNormal(move, matRot);
    rotVelo = rotVelo.Normalize() * 0.1f;
    translate_.x += rotVelo.x;
    translate_.z += rotVelo.z;


    matView_ = Inverse(MakeAffineMatrix(scale_, rotation_, translate_));*/

}

void DebugCamera::GetViewProjection() const
{
}
