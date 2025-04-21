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
    model_ = Model::CreateFromFile(_filePath);

    InitializeCommon();

}

void ObjectModel::Initialize(std::unique_ptr<Mesh>  _mesh)
{
    model_ = Model::CreateFromMesh(std::move(_mesh));

    InitializeCommon();
}

void ObjectModel::Initialize(Model* _model)
{
    model_ = _model;

    InitializeCommon();
}

void ObjectModel::Update()
{
    model_->Update(gameTime_->GetChannel(timeChannel).GetDeltaTime<float>());


    worldTransform_.transform_ = translate_;
    worldTransform_.scale_ = scale_;
    if (useQuaternion_)
        worldTransform_.quaternion_ = quaternion_.Normalize();
    else
        worldTransform_.rotate_ = euler_;
    worldTransform_.UpdateData(useQuaternion_);
}

void ObjectModel::Draw(const Camera* _camera, const Vector4& _color)
{
    objectColor_->SetColor(_color);


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


    auto commandList = DXCommon::GetInstance()->GetCommandList();
    RTVManager::GetInstance()->GetRenderTexture("ShadowMap")->QueueCommandDSVtoSRV(6);
    _camera->QueueCommand(commandList, 0);
    worldTransform_.QueueCommand(commandList, 1);
    objectColor_->QueueCommand(commandList, 3);
    model_->QueueCommandAndDraw(commandList, _textureHandle);// BVB IBV MTL2 TEX4 LIGHT567
}

void ObjectModel::SetAnimation(const std::string& _name, bool _isLoop)
{
    model_->SetAnimation(_name, _isLoop);
}

void ObjectModel::ChangeAnimation(const std::string& _name, float _blendTime, bool _isLoop)
{
    model_->ChangeAnimation(_name, _blendTime, _isLoop);
}

void ObjectModel::DrawShadow(const Camera* _camera,  uint32_t _id)
{
    auto lightGroup = LightingSystem::GetInstance()->GetLightGroup();
    if (lightGroup.get() == nullptr) return;

    if (idForGPU == nullptr)
        CreateIDResource();

    *idForGPU = _id;

    auto commandList = DXCommon::GetInstance()->GetCommandList();

    if (lightGroup->GetDirectionalLight()->IsCastShadow())
    {
        RTVManager::GetInstance()->SetRenderTexture("ShadowMap");

        PSOManager::GetInstance()->SetPipeLineStateObject(PSOFlags::Type_DLShadowMap);
        PSOManager::GetInstance()->SetRootSignature(PSOFlags::Type_DLShadowMap);

        _camera->QueueCommand(commandList, 0);
        worldTransform_.QueueCommand(commandList, 1);
        commandList->SetGraphicsRootConstantBufferView(2, idResource_->GetGPUVirtualAddress());
        model_->QueueCommandForShadow(commandList);// BVB IBV LIGHT3
    }

    auto pointLights = lightGroup->GetAllPointLights();


    for (auto& pointLight : pointLights)
    {
        if (!pointLight->IsCastShadow())
            continue;

        PSOManager::GetInstance()->SetPipeLineStateObject(PSOFlags::Type_PLShadowMap);
        PSOManager::GetInstance()->SetRootSignature(PSOFlags::Type_PLShadowMap);

        auto handles = pointLight->GetShadowMapHandles();
        uint32_t handle = handles[0];

        RTVManager::GetInstance()->SetCubemapRenderTexture(handle);

        worldTransform_.QueueCommand(commandList, 0);
        LightingSystem::GetInstance()->QueuePointLightShadowCommand(commandList, 1, pointLight.get());
        commandList->SetGraphicsRootConstantBufferView(2, idResource_->GetGPUVirtualAddress());
        model_->GetMeshPtr()->QueueCommand(commandList);//

        commandList->DrawIndexedInstanced(model_->GetMeshPtr()->GetIndexNum(), 1, 0, 0, 0);

    }
}

void ObjectModel::SetModel(const std::string& _filePath)
{
    model_ = Model::CreateFromFile(_filePath);
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

    ImGui::InputText("use Model", filePathBuffer_, 128);
    if (ImGui::Button("SetModel"))
    {
        SetModel(filePathBuffer_);
    }
    ImGui::PopID();

#endif // _DEBUG
}

void ObjectModel::InitializeCommon()
{
    worldTransform_.Initialize();
    objectColor_ = std::make_unique<ObjectColor>();
    objectColor_->Initialize();
    gameTime_ = GameTime::GetInstance();
}

void ObjectModel::CreateIDResource()
{
    idResource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t));

    idResource_->Map(0, nullptr, (void**)&idForGPU);
}
