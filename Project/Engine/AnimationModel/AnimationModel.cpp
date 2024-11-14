#include "AnimationModel.h"

#include "ModelManager.h"
#include "DXCommon.h"

void AnimationModel::Initialize(const std::string& _filePath)
{
    model_ = Model::CreateFromObj(_filePath);

    worldTransform_.Initialize();
    objectColor_ = std::make_unique<ObjectColor>();
    objectColor_->Initialize();
}

void AnimationModel::Update()
{
#ifdef _DEBUG
    ImGui();
#endif // _DEBUG


    model_->Update();

    worldTransform_.UpdateData(/*{ model_->GetNodeMatrix(),model_->GetAnimationMatrix()}*/);
}

void AnimationModel::Draw(const Camera* _camera, const Vector4& _color)
{
    objectColor_->SetColor(_color);

    ModelManager::GetInstance()->PreDrawForAnimationModel();

    auto commandList = DXCommon::GetInstance()->GetCommandList();
    _camera->QueueCommand(commandList, 0);
    worldTransform_.QueueCommand(commandList, 1);
    objectColor_->QueueCommand(commandList, 3);
    model_->QueueCommandAndDraw(commandList);// BVB IBV MTL2 TEX4 LIGHT567

    model_->DrawSkeleton(worldTransform_.matWorld_);
}


#ifdef _DEBUG
#include <imgui.h>
void AnimationModel::ImGui()
{
    ImGui::PushID(this);
    ImGui::DragFloat3("Translate", &worldTransform_.transform_.x, 0.01f);
    ImGui::DragFloat3("Scale", &worldTransform_.scale_.x, 0.01f);
    ImGui::DragFloat3("Rotate", &worldTransform_.rotate_.x, 0.01f);
    ImGui::PopID();
}
#endif // _DEBUG
