#include "Camera.h"
#include "MatrixFunction.h"
#include "DXCommon.h"
#include <imgui.h>

void Camera::Initialize()
{
    Map();
    TransferData();
}

void Camera::Update()
{
    if(ImGui::BeginTabBar("camera"))
    {
        if (ImGui::BeginTabItem("camera"))
        {
            ImGui::DragFloat3("translate", &translate_.x, 0.01f);
            ImGui::DragFloat3("rotate", &rotate_.x, 0.01f);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void Camera::Draw()
{
}

void Camera::TransferData()
{
    matWorld_ = MakeAffineMatrix(scale_, rotate_, translate_);
    matView_ = Inverse(matWorld_);
    matProjection_ = MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
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
