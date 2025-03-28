#include <Features/Effect/Manager/ParticleManager.h>
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Camera/Camera/Camera.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Features/Effect/Effect/ParticleEmitters.h>
#include <cassert>

const uint32_t ParticleManager::kGroupMaxInstance = 1024;

ParticleManager* ParticleManager::GetInstance()
{
    static ParticleManager instance;
    return &instance;
}

ParticleManager::~ParticleManager()
{
    groups_.clear();
}

void ParticleManager::ClearGroup()
{
    for (auto& [name, group] : groups_)
    {
        group.particles.clear();
    }
    groups_.clear();
}

void ParticleManager::ClearGroup(const std::string& _groupName)
{
    if (groups_.contains(_groupName))
    {
        groups_[_groupName].particles.clear();
        groups_.erase(_groupName);
    }
}

void ParticleManager::Initialize()
{
    srvManager_ = SRVManager::GetInstance();

    PSOFlags psoFlags = PSOFlags::Type_Particle | PSOFlags::Blend_Add | PSOFlags::Cull_Back;

    auto pso = PSOManager::GetInstance()->GetPipeLineStateObject(psoFlags);
    assert(pso.has_value());
    pipelineState_[BlendMode::Add] = pso.value();

    auto rootSignature = PSOManager::GetInstance()->GetRootSignature(psoFlags);
   assert(rootSignature.has_value());
   rootsignature_ = rootSignature.value();

   gameTime_ = GameTime::GetInstance();
}


void ParticleManager::Update(const Vector3& _cRotate)
{
    Vector3 cRotate = {};
    Vector3 pRotate = {};
    Matrix4x4 billboradMat = Matrix4x4::Identity();

    for (auto& [name, group] : groups_)
    {
        group.instanceNum = 0;
        std::string timeChannel{};
        if (group.emitterPtr)
            group.emitterPtr->GetTimeChannel();
        else
            timeChannel = "default";

        float deltaTime = gameTime_->GetChannel(timeChannel).GetDeltaTime<float>();

        for (auto it = group.particles.begin(); it != group.particles.end();)
        {
            it->Update(deltaTime);
            if (!it->IsAlive())
            {
                it = group.particles.erase(it);
                continue;
            }
            if(group.emitterPtr)
            {
                auto axes = group.emitterPtr->GetBillboardAxes();

                for (uint32_t index = 0; index < 3; ++index)
                {
                    if (axes[index])
                    {
                        cRotate[index] = _cRotate[index];
                    }
                }
                billboradMat = MakeAffineMatrix({ 1,1,1 }, cRotate, { 0,0,0 });

                Matrix4x4 mat = MakeScaleMatrix(it->GetScale());
                if (group.emitterPtr->EnableBillboard())
                    mat = mat * billboradMat;
                if (group.emitterPtr->ShouldFaceDirection())
                    mat = mat * it->GetDirectionMatrix();
                mat = mat * MakeRotateMatrix(it->GetRotation());
                mat = mat * MakeTranslateMatrix(it->GetPosition());
                group.constMap[group.instanceNum].matWorld = mat;
            }
            else
            {
                Matrix4x4 mat = MakeAffineMatrix(it->GetScale(), it->GetRotation(), it->GetPosition());
                group.constMap[group.instanceNum].matWorld = mat;
            }
            group.constMap[group.instanceNum].color = it->GetColor();
            group.instanceNum++;
            ++it;
        }
    }
}

void ParticleManager::Draw(const Camera* _camera)
{
    PreDraw();

    ID3D12GraphicsCommandList* commandList = DXCommon::GetInstance()->GetCommandList();
    for (auto [name, particles] : groups_)
    {
        if (particles.instanceNum == 0)
            continue;

        commandList->SetPipelineState(pipelineState_[particles.blendMode]);

        commandList->IASetVertexBuffers(0, 1, particles.model->GetMeshPtr()->GetVertexBufferView());
        commandList->IASetIndexBuffer(particles.model->GetMeshPtr()->GetIndexBufferView());

        commandList->SetGraphicsRootConstantBufferView(0, _camera->GetResource()->GetGPUVirtualAddress());
        commandList->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUSRVDescriptorHandle(particles.srvIndex));
        commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetGPUHandle(particles.textureHandle));

        commandList->DrawIndexedInstanced(particles.model->GetMeshPtr()->GetIndexNum(), particles.instanceNum, 0, 0, 0);
    }
}

void ParticleManager::CreateParticleGroup(const std::string& _groupName, const std::string& _modelPath,
                                          ParticleEmitter* _emitterPtr, BlendMode _blendMode, uint32_t _textureHandle)
{
    if (groups_.contains(_groupName))
        return;

    std::string groupName = _groupName;
    if (!_emitterPtr)
        groupName += "NoEmitter";

    Group& group = groups_[groupName];
    group.model = Model::CreateFromFile(_modelPath);
    if (_textureHandle == UINT32_MAX)
        group.textureHandle = group.model->GetMaterialPtr()->GetTexturehandle();
    else
        group.textureHandle = _textureHandle;

    group.srvIndex = srvManager_->Allocate();
    group.resource = DXCommon::GetInstance()->CreateBufferResource(sizeof(constantBufferData) * kGroupMaxInstance);
    group.resource->Map(0, nullptr, reinterpret_cast<void**>(&group.constMap));

    srvManager_->CreateSRVForStructureBuffer(group.srvIndex, group.resource.Get(), kGroupMaxInstance, sizeof(constantBufferData));

    group.instanceNum = 0;

    if (!_emitterPtr)
    {
        group.emitterPtr = nullptr;
    }
    else
    {
        group.emitterPtr = _emitterPtr;
    }

    group.blendMode = _blendMode;

    group.psoFlags = PSOFlags::Type_Particle | PSOFlags::Cull_Back | GetBlendMode(_blendMode);
    if (!pipelineState_.contains(_blendMode))
    {

        auto pso = PSOManager::GetInstance()->GetPipeLineStateObject(group.psoFlags);
        assert(pso.has_value());
        pipelineState_[_blendMode] = pso.value();
    }

}

void ParticleManager::CreateParticleGroup(const std::string& _groupName, const std::string& _modelPath, ParticleEmitter* _emitterPtr, PSOFlags _flags, uint32_t _textureHandle)
{
    if (groups_.contains(_groupName))
        return;

    std::string groupName = _groupName;
    if (!_emitterPtr)
        groupName += "NoEmitter";

    Group& group = groups_[groupName];
    group.model = Model::CreateFromFile(_modelPath);
    if (_textureHandle == UINT32_MAX)
        group.textureHandle = group.model->GetMaterialPtr()->GetTexturehandle();
    else
        group.textureHandle = _textureHandle;

    group.srvIndex = srvManager_->Allocate();
    group.resource = DXCommon::GetInstance()->CreateBufferResource(sizeof(constantBufferData) * kGroupMaxInstance);
    group.resource->Map(0, nullptr, reinterpret_cast<void**>(&group.constMap));

    srvManager_->CreateSRVForStructureBuffer(group.srvIndex, group.resource.Get(), kGroupMaxInstance, sizeof(constantBufferData));

    group.instanceNum = 0;

    if (!_emitterPtr)
    {
        group.emitterPtr = nullptr;
    }
    else
    {
        group.emitterPtr = _emitterPtr;
    }

    if (IsValidPSOFlags(_flags))
        group.psoFlags = _flags;
    else
        assert("PSOFlagsが無効です" && false);

    BlendMode blendMode = GetBlendMode(_flags);

    if (!pipelineState_.contains(blendMode))
    {
        auto pso = PSOManager::GetInstance()->GetPipeLineStateObject(group.psoFlags);
        assert(pso.has_value());
        pipelineState_[blendMode] = pso.value();
    }
}

void ParticleManager::DeleteParticleGroup(const std::string& _groupName)
{
    if (!groups_.contains(_groupName))
    {
        if (!groups_.contains(_groupName + "NoEmitter"))
            return;
        groups_.erase(_groupName + "NoEmitter");
    }
    else
        groups_.erase(_groupName);
}

void ParticleManager::SetGroupModel(const std::string& _groupName, const std::string& _modelPath)
{
    if (!groups_.contains(_groupName))
        throw std::runtime_error("not find particleGroup! name:" + '\"' + _groupName + '\"');

    groups_[_groupName].model = Model::CreateFromFile(_modelPath);
}

void ParticleManager::SetGroupTexture(const std::string& _groupName, uint32_t _textureHandle)
{
    if (!groups_.contains(_groupName))
        throw std::runtime_error("not find particleGroup! name:" + '\"' + _groupName + '\"');
    groups_[_groupName].textureHandle = _textureHandle;
}


void ParticleManager::AddParticleToGroup(const std::string& _groupName, const Particle& _particles)
{
    std::string gName = _groupName;
    if (!groups_.contains(_groupName))
    {
        if (!groups_.contains(_groupName + "NoEmitter"))
            return;
        else if(!groups_[_groupName + "NoEmitter"].emitterPtr)
            gName = _groupName + "NoEmitter";
    }

    if (!groups_.contains(gName))
    {
        CreateParticleGroup(gName, "plane/plane.gltf", nullptr);
    }

    if (groups_[gName].particles.size() >= kGroupMaxInstance)
        return;

    groups_[gName].particles.push_back(_particles);
}

void ParticleManager::AddParticleToGroup(const std::string& _groupName, const std::vector<Particle>& _particles)
{
    for(const auto& particle:_particles)
    {
        AddParticleToGroup(_groupName, particle);
    }
}

void ParticleManager::SetAllGroupTimeChannel(const std::string& _channel)
{
    for (auto& [name, group] : groups_)
    {
        group.emitterPtr->SetTimeChannel(_channel);
    }
}

void ParticleManager::SetGroupTimeChannel(const std::string& _groupName, const std::string& _channel)
{
    if (!groups_.contains(_groupName))
        throw std::runtime_error("not find particleGroup! name:" + '\"' + _groupName + '\"');
    groups_[_groupName].emitterPtr->SetTimeChannel(_channel);
}


void ParticleManager::PreDraw()
{
    ID3D12GraphicsCommandList* commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->SetGraphicsRootSignature(rootsignature_);
}

