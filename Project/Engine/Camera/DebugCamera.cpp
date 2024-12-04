#include "DebugCamera.h"
#include "DebugCamera.h"
#include "Camera.h"
#include "Input/Input.h"
#include "Math/VectorFunction.h"
#include "Math/MatrixFunction.h"

void DebugCamera::Initialize()
{
}

void DebugCamera::Update()
{
    Vector3 move;
    // 左シフト押しながら移動
    if (Input::GetInstance()->IsKeyPressed(DIK_LSHIFT))
        Input::GetInstance()->GetMove(move, 0.1f);
    translate_.y += move.y;
    move.y = 0;
    Vector3 rot;

    // ホイールクリックでカメラ回転
    if (Input::GetInstance()->IsMousePressed(2))
        Input::GetInstance()->GetRotate(rot);
        rotation_ += rot;
    Matrix4x4 matRot = MakeRotateMatrix(rotation_);


    Vector3 rotVelo = TransformNormal(move, matRot);
    rotVelo = rotVelo.Normalize() * 0.1f;
    translate_.x += rotVelo.x;
    translate_.z += rotVelo.z;


    matView_ = Inverse(MakeAffineMatrix(scale_, rotation_, translate_));

}

void DebugCamera::GetViewProjection() const
{
}
