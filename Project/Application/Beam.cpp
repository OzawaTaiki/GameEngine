#include "Beam.h"
#include "Input.h"
#include <Windows.h>

#include "MatrixFunction.h"
#include "VectorFunction.h"
#include "../Collider/CollisionManager.h"

void Beam::Initialize()
{
    worldTransform_.Initialize();

    model_ = Model::CreateFromObj("beam/beam.obj");

    collider_= std::make_unique<Collider>();
    collider_->SetBoundingBox(Collider::BoundingBox::OBB_3D);
    collider_->SetShape(model_->GetMin(), model_->GetMax());
    collider_->SetAtrribute("Beam");
    collider_->SetMask("Beam");
    collider_->SetGetWorldMatrixFunc([this]() {return worldTransform_.matWorld_; });
    collider_->SetOnCollisionFunc([this]() {OnCollision(); });



}

void Beam::Update()
{
    if (worldTransform_.parent_ == nullptr)
    {
        MessageBoxA(nullptr, "Beam Parent is not set", "Error", MB_OK | MB_ICONERROR);
        return;
    }


    Vector3 target = target_ - worldTransform_.parent_->GetWorldPosition();
    float distance = target_.Length();
    Vector3 direction = target_.Normalize();

    //direction = TransformNormal(direction, worldTransform_.parent_->matWorld_);


    worldTransform_.rotate_.y = std::atan2(direction.x, direction.z);
    float horizontalDistance = std::sqrt(direction.x * direction.x + direction.z * direction.z);
    worldTransform_.rotate_.x = -std::atan2(direction.y, horizontalDistance);

    //worldTransform_.rotate_ -= worldTransform_.parent_->rotate_;

    worldTransform_.scale_.z = distance;

    CollisionManager::GetInstance()->RegisterCollider(collider_.get());

    worldTransform_.UpdateData();

#ifdef _DEBUG
    ImGui();
#endif // _DEBUG
}

void Beam::Draw(const Camera* _camera)
{
    model_->Draw(worldTransform_, _camera, 0u);
#ifdef _DEBUG
    if (isDrawBoundingBox_)
    {
        collider_->Draw();
    }
#endif // _DEBUG
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
