#include "Beam.h"
#include "Input.h"
#include <Windows.h>

#include "MatrixFunction.h"
#include "VectorFunction.h"

void Beam::Initialize()
{
    worldTransform_.Initialize();

    model_ = Model::CreateFromObj("beam/beam.obj");


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

    worldTransform_.rotate_.y = std::atan2(direction.x, direction.z);
    float horizontalDistance = std::sqrt(direction.x * direction.x + direction.z * direction.z);
    worldTransform_.rotate_.x = -std::atan2(direction.y, horizontalDistance);


    worldTransform_.scale_.z = distance;

    worldTransform_.UpdateData();

#ifdef _DEBUG
    ImGui();
#endif // _DEBUG
}

void Beam::Draw(const Camera* _camera)
{
    model_->Draw(worldTransform_, _camera, 0u);
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