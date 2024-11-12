#include "Beam.h"
#include "Input.h"
#include <Windows.h>

#include "MatrixFunction.h"
#include "VectorFunction.h"
#include "CollisionManager.h"

void Beam::Initialize()
{
    worldTransform_.Initialize();

    model_ = Model::CreateFromObj("beam/beam.obj");

    pSize_ = &worldTransform_.scale_;
    pRotate_ = &worldTransform_.rotate_;
    pWorldTransform_ = &worldTransform_.matWorld_;


    Collider::SetAtrribute("Player");
    Collider::SetMask({ "Player" });
    Collider::SetBoundingBox(Collider::BoundingBox::OBB_3D);

}

void Beam::Update()
{
    if (worldTransform_.parent_ == nullptr)
    {
        MessageBoxA(nullptr, "Beam Parent is not set", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    CollisionManager::GetInstance()->SetCollider(this);

    Vector3 target = target_ - worldTransform_.parent_->GetWorldPosition();
    float distance = target_.Length();
    Vector3 direction = target_.Normalize();

    //direction = TransformNormal(direction, worldTransform_.parent_->matWorld_);


    worldTransform_.rotate_.y = std::atan2(direction.x, direction.z);
    float horizontalDistance = std::sqrt(direction.x * direction.x + direction.z * direction.z);
    worldTransform_.rotate_.x = -std::atan2(direction.y, horizontalDistance);

    //worldTransform_.rotate_ -= worldTransform_.parent_->rotate_;

    worldTransform_.scale_.z = distance;

    worldTransform_.UpdateData();

#ifdef _DEBUG
    ImGui();
#endif // _DEBUG
}

void Beam::Draw(const Camera* _camera)
{
    model_->Draw(worldTransform_, _camera, 0u);
    if (isDrawBoundingBox_)
        Collider::Draw(worldTransform_.matWorld_);
}

void Beam::OnCollision()
{

}


#ifdef _DEBUG
#include <imgui.h>
void Beam::ImGui()
{
    ImGui::BeginTabBar("Player");
    if (ImGui::BeginTabItem("Beam"))
    {
        ImGui::DragFloat3("Position", &worldTransform_.transform_.x, 0.01f);
        ImGui::DragFloat3("Rotation", &worldTransform_.rotate_.x, 0.01f);
        ImGui::DragFloat3("Scale", &worldTransform_.scale_.x, 0.01f);
        ImGui::DragFloat3("Target", &target_.x, 0.01f);
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();

}
#endif // _DEBUG
