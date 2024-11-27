#include "ObjectModel.h"
#include "DXCommon.h"
#include "ModelManager.h"
#include "MatrixFunction.h"
#include "../Collider/CollisionManager.h"

void ObjectModel::Initialize(const std::string& _filePath)
{
    model_ = Model::CreateFromObj(_filePath);

    worldTransform_.Initialize();
    objectColor_ = std::make_unique<ObjectColor>();
    objectColor_->Initialize();
}

void ObjectModel::Update()
{
#ifdef _DEBUG
    ImGui();
#endif // _DEBUG
    worldTransform_.transform_ = translate_;
    worldTransform_.scale_ = scale_;
    worldTransform_.rotate_ = rotate_;
    worldTransform_.UpdateData();
}

void ObjectModel::Draw(const Camera* _camera, const Vector4& _color)
{
    objectColor_->SetColor(_color);

    ModelManager::GetInstance()->PreDrawForObjectModel();

    auto commandList = DXCommon::GetInstance()->GetCommandList();
    _camera->QueueCommand(commandList, 0);
    worldTransform_.QueueCommand(commandList, 1);
    objectColor_->QueueCommand(commandList, 3);
    model_->QueueCommandAndDraw(commandList);// BVB IBV MTL2 TEX4 LIGHT567


    //model_->DrawSkeleton(worldTransform_.matWorld_);
}

void ObjectModel::SetModel(const std::string& _filePath)
{
    model_ = Model::CreateFromObj(_filePath);
}

#ifdef _DEBUG
#include <imgui.h>
void ObjectModel::ImGui()
{
    ImGui::PushID(this);
    ImGui::SeparatorText("model");
    ImGui::DragFloat3("Translate", &translate_.x, 0.01f);
    ImGui::DragFloat3("Scale", &scale_.x, 0.01f);
    ImGui::DragFloat3("Rotate", &rotate_.x, 0.01f);
    ImGui::PopID();
}
#endif // _DEBUG
