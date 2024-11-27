#include "Camera.h"
#include "MatrixFunction.h"
#include "DXCommon.h"
#include "Input.h"
#include "VectorFunction.h"
#include "MatrixFunction.h"
#include "RandomGenerator.h"
#ifdef _DEBUG
#include <imgui.h>
#endif // _DEBUG


void Camera::Initialize()
{
    Map();
    UpdateMatrix();
}

void Camera::Update(bool _showImGui)
{
#ifdef _DEBUG
    if(_showImGui)
    {
        if (ImGui::BeginTabBar("camera"))
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
#endif // _DEBUG

    if (shaking_)
    {
        UpdateShake();
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
    // ビュー行列の逆行列を計算してカメラの位置を抽出
    Matrix4x4 iView = Inverse(matView_);
    translate_ = { iView.m[3][0], iView.m[3][1], iView.m[3][2] };

    // ビュー・プロジェクション行列を計算
    matViewProjection_ = matView_ * matProjection_;

    // 定数バッファへのデータ転送
    constMap_->pos = translate_;
    constMap_->view = matView_;
    constMap_->proj = matProjection_;
}

void Camera::UpdateMatrix()
{
    matWorld_ = MakeAffineMatrix(scale_, rotate_, translate_);
    matView_ = Inverse(matWorld_);
    matProjection_ = MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
    matViewProjection_ = matView_ * matProjection_;


    constMap_->pos = translate_;
    constMap_->view = matView_;
    constMap_->proj = matProjection_;

}

void Camera::Shake(float _time, const Vector2& _RangeMin, const Vector2& _RangeMax)
{
    shaking_ = true;
    shakeTime_ = _time;
    shakeTimer_ = 0.0f;
    shakeRangeMin_ = _RangeMin;
    shakeRangeMax_ = _RangeMax;
}

void Camera::ShakeParametaerSettingFromImGui()
{
#ifdef _DEBUG

    ImGui::SeparatorText("Shake");
    ImGui::DragFloat("time", &shakeTime_, 0.01f);
    ImGui::DragFloat2("rangeMin", &shakeRangeMin_.x, 0.01f);
    ImGui::DragFloat2("rangeMax", &shakeRangeMax_.x, 0.01f);
    if (ImGui::Button("Shake"))
    {
        Shake(shakeTime_, shakeRangeMin_, shakeRangeMax_);
    }
#endif // _DEBUG
}

void Camera::QueueCommand(ID3D12GraphicsCommandList* _cmdList, UINT _index) const
{
    _cmdList->SetGraphicsRootConstantBufferView(_index, resource_->GetGPUVirtualAddress());
}

void Camera::UpdateShake()
{
    shakeTimer_ += 1.0f / 60.0f;
    if (shakeTimer_ >= shakeTime_)
    {
        shaking_ = false;
        shakeTimer_ = 0.0f;
        shakeOffset_ = { 0.0f,0.0f,0.0f };
    }
    else
    {
        shakeOffset_ = RandomGenerator::GetInstance()->GetUniformVec2(shakeRangeMin_, shakeRangeMax_);
    }
}

void Camera::Map()
{
    resource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(ConstantBufferDate));
    resource_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));
}
