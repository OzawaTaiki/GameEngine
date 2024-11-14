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

void Camera::QueueCommand(ID3D12GraphicsCommandList* _cmdList, UINT _index) const
{
    _cmdList->SetGraphicsRootConstantBufferView(_index, resource_->GetGPUVirtualAddress());
}

void Camera::Map()
{
    resource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(ConstantBufferDate));
    resource_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));
}
