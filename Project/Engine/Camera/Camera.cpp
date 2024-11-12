#include "Camera.h"
#include "MatrixFunction.h"
#include "DXCommon.h"
#include "Input.h"
#include "VectorFunction.h"
#include "MatrixFunction.h"
#include <imgui.h>

void Camera::Initialize()
{
    Map();
    UpdateMatrix();

    worldTransform_.Initialize();
}

void Camera::Update()
{
    if(ImGui::BeginTabBar("camera"))
    {
        ImGui::PushID(this);
        if (ImGui::BeginTabItem("camera"))
        {
            ImGui::DragFloat3("translate", &translate_.x, 0.01f);
            ImGui::DragFloat3("rotate", &rotate_.x, 0.01f);
            ImGui::EndTabItem();
        }
        ImGui::PopID();
        ImGui::EndTabBar();
    }

    worldTransform_.transform_ = translate_;
    worldTransform_.rotate_ = rotate_;
    worldTransform_.scale_ = scale_;
    worldTransform_.UpdateData();
    matView_ = Inverse(worldTransform_.matWorld_);

    //Vector3 move;
    //Input::GetInstance()->GetMove(move, 0.1f);
    //Vector3 rot;
    //Input::GetInstance()->GetRotate(rot);

    //Matrix4x4 matRot = MakeRotateMatrix(rotate_ + rot);
    //Vector3 rotVelo = TransformNormal(move, matRot);

    //translate_ += rotVelo;
}

void Camera::Draw()
{
}

void Camera::UpdateMatrix()
{
    matWorld_ = worldTransform_.matWorld_;
    matView_ = Inverse(matWorld_);
    matProjection_ = MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
    matViewProjection_ = matView_ * matProjection_;

    constMap_->pos = translate_;
    constMap_->view = matView_;
    constMap_->proj = matProjection_;
}

void Camera::TransferData()
{
    Matrix4x4 iView = Inverse(matView_);
    //translate_ = { iView.m[3][0],iView.m[3][1],iView.m[3][2] };
    matViewProjection_ = matView_ * matProjection_;

    constMap_->pos = translate_;
    constMap_->view = matView_;
    constMap_->proj = matProjection_;
}

void Camera::Map()
{
    resource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(ConstantBufferDate));
    resource_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));
}
