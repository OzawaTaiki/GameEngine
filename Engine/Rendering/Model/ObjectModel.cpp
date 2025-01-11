#include "ObjectModel.h"
#include "ModelManager.h"
#include <Physics/Math/MatrixFunction.h>
#include <Core/DirectX/DXCommon.h>
#include <Physics/Collision/CollisionManager.h>

// TODO : ライトを独立させる
// ライトをstructureにして，ライトの数だけ配列を作る

void ObjectModel::Initialize(const std::string& _filePath, const std::string& _name)
{
    model_ = Model::CreateFromObj(_filePath);

    if (_name.empty())
        name_ = _filePath;
    else
        name_ = _name;

    worldTransform_.Initialize();
    objectColor_ = std::make_unique<ObjectColor>();
    objectColor_->Initialize();

}

void ObjectModel::Update(const bool _showImgui)
{
#ifdef _DEBUG
    if(_showImgui)
        ImGui();
#endif // _DEBUG
    worldTransform_.transform_ = translate_;
    worldTransform_.scale_ = scale_;
    if (useQuaternion_)
        worldTransform_.quaternion_ = quaternion_;
    else
        worldTransform_.rotate_ = rotate_;
    worldTransform_.UpdateData(useQuaternion_);
    UpdateUVTransform();
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

}

void ObjectModel::SetModel(const std::string& _filePath)
{
    model_ = Model::CreateFromObj(_filePath);
}

void ObjectModel::UpdateUVTransform()
{
    model_->SetUVTransform(uvTransform_, 0);
    model_->SetUVScale(uvScale_, 0);
    model_->SetUVRotation(uvRotation_, 0);
}

#ifdef _DEBUG
#include <imgui.h>
void ObjectModel::ImGui()
{
    ImGui::PushID(this);
    ImGui::SeparatorText(name_.c_str());
    ImGui::DragFloat3("Translate", &translate_.x, 0.01f);
    ImGui::DragFloat3("Scale", &scale_.x, 0.01f);
    ImGui::DragFloat3("Rotate", &rotate_.x, 0.01f);
    ImGui::PopID();
}
#endif // _DEBUG
