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
    worldTransform_.transform_ = translate_;
    worldTransform_.scale_ = scale_;
    if (useQuaternion_)
        worldTransform_.quaternion_ = quaternion_.Normalize();
    else
        worldTransform_.rotate_ = euler_;
    worldTransform_.UpdateData(useQuaternion_);

    if (model_->HasAnimation() && animationController_->IsAnimationPlaying())
    {
        animationController_->Update(gameTime_->GetChannel(timeChannel).GetDeltaTime<float>());
    }

}

void ObjectModel::Draw(const Camera* _camera, const Vector4& _color)
{
    objectColor_->SetColor(_color);

    auto lightGroup = LightingSystem::GetInstance()->GetLightGroup();
    if(lightGroup)
    {
        auto pointLights = lightGroup->GetAllPointLights();
        if (!pointLights.empty())
        {
            auto handles = pointLights[0]->GetShadowMapHandles();
            uint32_t handle = handles[0];
            RTVManager::GetInstance()->QueuePointLightShadowMapToSRV(handle, 7);
        }
    }

    RTVManager::GetInstance()->GetRenderTexture("ShadowMap")->QueueCommandDSVtoSRV(6);
    auto commandList = DXCommon::GetInstance()->GetCommandList();
    _camera->QueueCommand(commandList, 0);
    worldTransform_.QueueCommand(commandList, 1);
    objectColor_->QueueCommand(commandList, 3);
    if(animationController_)
        model_->QueueCommandAndDraw(commandList, animationController_->GetMargedMesh());
    else
        model_->QueueCommandAndDraw(commandList, _color);

    if (drawSkeleton_)
        animationController_->DrawSkeleton(worldTransform_.matWorld_);
}

void ObjectModel::Draw(const Camera* _camera, uint32_t _textureHandle, const Vector4& _color)
{
    objectColor_->SetColor(_color);

    auto lightGroup = LightingSystem::GetInstance()->GetLightGroup();

    auto pointLights = lightGroup->GetAllPointLights();
    if (!pointLights.empty())
    {
        auto handles = pointLights[0]->GetShadowMapHandles();
        uint32_t handle = handles[0];
        RTVManager::GetInstance()->QueuePointLightShadowMapToSRV(handle, 7);
    }


    auto commandList = DXCommon::GetInstance()->GetCommandList();
    RTVManager::GetInstance()->GetRenderTexture("ShadowMap")->QueueCommandDSVtoSRV(6);
    _camera->QueueCommand(commandList, 0);
    worldTransform_.QueueCommand(commandList, 1);
    objectColor_->QueueCommand(commandList, 3);

    if (animationController_)
        model_->QueueCommandAndDraw(commandList, _textureHandle, _color, animationController_->GetMargedMesh());
    else
        model_->QueueCommandAndDraw(commandList, _textureHandle, _color);

    if(drawSkeleton_)
        animationController_->DrawSkeleton(worldTransform_.matWorld_);
}

void ObjectModel::LoadAnimation(const std::string& _filePath, const std::string& _name)
{
    model_->LoadAnimation(_filePath, _name);
    if (!animationController_)
    {
        animationController_ = std::make_unique<AnimationController>(model_);
        auto modelManager = ModelManager::GetInstance();
        animationController_->Initialize(modelManager->GetComputePipeline(), modelManager->GetComputeRootSignature());
    }
}

void ObjectModel::SetAnimation(const std::string& _name, bool _isLoop)
{
    if (model_->HasAnimation())
    {
        animationController_->SetAnimation(_name, _isLoop);
    }
}

void ObjectModel::ChangeAnimation(const std::string& _name, float _blendTime, bool _isLoop)
{
    if (model_->HasAnimation())
    {
        animationController_->ChangeAnimation(_name, _blendTime, _isLoop);
    }
}

void ObjectModel::DrawShadow(const Camera* _camera)
{
    auto lightGroup = LightingSystem::GetInstance()->GetLightGroup();
    if (lightGroup.get() == nullptr) return;

    auto commandList = DXCommon::GetInstance()->GetCommandList();

    if (lightGroup->GetDirectionalLight()->IsCastShadow())
    {
        RTVManager::GetInstance()->SetRenderTexture("ShadowMap");

        PSOManager::GetInstance()->SetPipeLineStateObject(PSOFlags::Type::DLShadowMap);
        PSOManager::GetInstance()->SetRootSignature(PSOFlags::Type::DLShadowMap);

        _camera->QueueCommand(commandList, 0);
        worldTransform_.QueueCommand(commandList, 1);
        model_->QueueCommandForShadow(commandList);// BVB IBV LIGHT3
    }

    auto pointLights = lightGroup->GetAllPointLights();


    for (auto& pointLight : pointLights)
    {
        if (!pointLight->IsCastShadow())
            continue;

        PSOManager::GetInstance()->SetPipeLineStateObject(PSOFlags::Type::PLShadowMap);
        PSOManager::GetInstance()->SetRootSignature(PSOFlags::Type::PLShadowMap);

        auto handles = pointLight->GetShadowMapHandles();
        uint32_t handle = handles[0];

        RTVManager::GetInstance()->SetCubemapRenderTexture(handle);

        worldTransform_.QueueCommand(commandList, 0);
        LightingSystem::GetInstance()->QueuePointLightShadowCommand(commandList, 1, pointLight.get());
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

    ImGui::Dummy(ImVec2(0, 10));
    ImGui::Checkbox("DrawSkeleton", &drawSkeleton_);

    ImGui::InputText("use Model", filePathBuffer_, 128);
    if (ImGui::Button("SetModel"))
    {
        SetModel(filePathBuffer_);
    }

    if(animationController_)
    {
        if(ImGui::CollapsingHeader("Skeleton"))
            animationController_->ImGui();
    }

    ImGui::PopID();

#endif // _DEBUG
}

void ObjectModel::InitializeCommon()
{
    worldTransform_.Initialize();

    if (model_->HasAnimation())
    {
        animationController_ = std::make_unique<AnimationController>(model_);
        auto modelManager = ModelManager::GetInstance();
        animationController_->Initialize(modelManager->GetComputePipeline(), modelManager->GetComputeRootSignature());
    }

    objectColor_ = std::make_unique<ObjectColor>();
    objectColor_->Initialize();

    gameTime_ = GameTime::GetInstance();
}
