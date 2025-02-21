#include <Features/Model/ObjectModel.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Core/DXCommon/DXCommon.h>
#include <Features/Collision/Manager/CollisionManager.h>
#include <Debug/ImGuiDebugManager.h>
#include <Core/DXCommon/RTV/RTVManager.h>



ObjectModel::ObjectModel(const std::string& _name)
{
    name_ = ImGuiDebugManager::GetInstance()->AddDebugWindow(_name, [&]() {ImGui(); });
}

ObjectModel::~ObjectModel()
{
    ImGuiDebugManager::GetInstance()->RemoveDebugWindow(name_);
}

void ObjectModel::Initialize(const std::string& _filePath)
{
    model_ = Model::CreateFromObj(_filePath);

    worldTransform_.Initialize();
    objectColor_ = std::make_unique<ObjectColor>();
    objectColor_->Initialize();

}

void ObjectModel::Update()
{
    worldTransform_.transform_ = translate_;
    worldTransform_.scale_ = scale_;
    if (useQuaternion_)
        worldTransform_.quaternion_ = quaternion_;
    else
        worldTransform_.rotate_ = euler_;
    worldTransform_.UpdateData();
}

void ObjectModel::Draw(const Camera* _camera, const Vector4& _color)
{
    objectColor_->SetColor(_color);

    ModelManager::GetInstance()->PreDrawForObjectModel();

    RTVManager::GetInstance()->GetRenderTexture("ShadowMap")->QueueCommandDSVtoSRV(6);
    auto commandList = DXCommon::GetInstance()->GetCommandList();
    _camera->QueueCommand(commandList, 0);
    worldTransform_.QueueCommand(commandList, 1);
    objectColor_->QueueCommand(commandList, 3);
    model_->QueueCommandAndDraw(commandList);// BVB IBV MTL2 TEX4 LIGHT567

}

void ObjectModel::Draw(const Camera* _camera, uint32_t _textureHandle, const Vector4& _color)
{
    objectColor_->SetColor(_color);

    ModelManager::GetInstance()->PreDrawForObjectModel();

    auto commandList = DXCommon::GetInstance()->GetCommandList();
    RTVManager::GetInstance()->GetRenderTexture("ShadowMap")->QueueCommandDSVtoSRV(6);
    _camera->QueueCommand(commandList, 0);
    worldTransform_.QueueCommand(commandList, 1);
    objectColor_->QueueCommand(commandList, 3);
    model_->QueueCommandAndDraw(commandList, _textureHandle);// BVB IBV MTL2 TEX4 LIGHT567
}

void ObjectModel::DrawShadow(const Camera* _camera)
{
    PSOManager::GetInstance()->SetPipeLineStateObject(PSOFlags::Type_ShadowMap);
    PSOManager::GetInstance()->SetRootSignature(PSOFlags::Type_ShadowMap);


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
}

void ObjectModel::ImGui()
{
#ifdef _DEBUG

    ImGui::PushID(this);
    ImGui::DragFloat3("Translate", &translate_.x, 0.01f);
    ImGui::DragFloat3("Scale", &scale_.x, 0.01f);
    if (useQuaternion_)
        ImGui::DragFloat4("Quaternion", &quaternion_.x, 0.01f);
    else
        ImGui::DragFloat3("Rotate", &euler_.x, 0.01f);

    ImGui::Checkbox("UseQuaternion", &useQuaternion_);
    ImGui::PopID();

#endif // _DEBUG
}
