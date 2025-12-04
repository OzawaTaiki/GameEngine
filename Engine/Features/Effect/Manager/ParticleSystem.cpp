#include <Features/Effect/Manager/ParticleSystem.h>
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Camera/Camera/Camera.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Features/Effect/Emitter/ParticleEmitter.h>
#include <Features/Model/Manager/ModelManager.h>

#include <cassert>

// 静的メンバ変数の初期化
uint32_t ParticleSystem::maxInstancesPerGroup = 8192;

ParticleSystem* ParticleSystem::GetInstance()
{
    static ParticleSystem instance;
    return &instance;
}

ParticleSystem::~ParticleSystem()
{
    ClearParticles();

    factory_ = nullptr;

}

void ParticleSystem::Initialize()
{
    srvManager_ = SRVManager::GetInstance();

    auto rootsig = PSOManager::GetInstance()->GetRootSignature(PSOFlags::Type::Particle);
    assert(rootsig.has_value());
    rootSignature_ = rootsig.value();
}

void ParticleSystem::Update(float _deltaTime)
{
    if (camera_ == nullptr)
    {
        throw std::runtime_error("Cameraが設定されていません。ポインタをセットしてください");
        return;
    }

    for (auto& [groupName, particleList] : particles_)
    {
        if (particleList.particles.empty())
            continue;
        particleList.instanceCount = 0;

        auto& particles = particleList.particles;


        auto billboard = particles.front()->GetBillboard();

        Vector3 rot = { 0,0,0 };
            if (billboard[0])                rot.x = camera_->rotate_.x;
            if (billboard[1])                rot.y = camera_->rotate_.y;
            if (billboard[2])                rot.z = camera_->rotate_.z;

        Matrix4x4 billboardMatrix = MakeRotateMatrix(rot);

        if (!particleList.useModifierName.empty())
        {
            for (auto& [name, modifier] : particleList.useModifierName)
            {
                auto it = modifierNames_.find(name);
                if (it == modifierNames_.end())
                {
                    CreateModifier(name);
                    it = modifierNames_.find(name);
                }
                std::list<Particle*> raw_particles;
                for (auto& smart_ptr : particles)
                {
                    raw_particles.push_back(smart_ptr.get());
                }
                it->second->Apply(raw_particles, _deltaTime);
            }
        }

        for (auto it = particles.begin(); it != particles.end();)
        {
            // group上限より大きいとき強制退場
            if (particleList.instanceCount >= maxInstancesPerGroup)
                break;

            auto& particle = *it;

            if (!particle->IsAlive())
            {
                it = particles.erase(it);
                continue;
            }
            particle->Update(_deltaTime);

            Matrix4x4 affineMatrix =
                MakeScaleMatrix(particle->GetScale()) *
                MakeRotateMatrix(particle->GetRotation()) *
                billboardMatrix*
                MakeTranslateMatrix(particle->GetPosition());

            particleList.mappedInstanceBuffer[particleList.instanceCount].worldMatrix = affineMatrix;
            particleList.mappedInstanceBuffer[particleList.instanceCount].color = particle->GetColor();

            particleList.instanceCount++;
            ++it;
        }
    }
}

void ParticleSystem::DrawParticles()
{
    auto cmdList = DXCommon::GetInstance()->GetCommandList();


    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->SetGraphicsRootSignature(rootSignature_);

    for (auto& [key, particleList] : particles_)
    {
        if (particleList.instanceCount == 0)
            continue;

        cmdList->SetPipelineState(psoMap_[particleList.psoIndex]);
        cmdList->IASetVertexBuffers(0, 1, particleList.model->GetMeshPtr()->GetVertexBufferView());
        cmdList->IASetIndexBuffer(particleList.model->GetMeshPtr()->GetIndexBufferView());


        cmdList->SetGraphicsRootConstantBufferView(0, camera_->GetResource()->GetGPUVirtualAddress());
        cmdList->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUSRVDescriptorHandle(particleList.srvIndex));
        cmdList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetGPUHandle(particleList.textureHandle));
        cmdList->DrawIndexedInstanced(particleList.model->GetMeshPtr()->GetIndexNum(), particleList.instanceCount, 0, 0, 0);

        particleList.instanceCount = 0;
    }
}

void ParticleSystem::SetModifierFactory(std::unique_ptr<IParticleMoifierFactory> _factory)
{
    if (factory_ == _factory ||
        _factory == nullptr)
        return;

    if (factory_ != nullptr)
    {
        factory_.reset();
        factory_ = nullptr;
    }

    factory_ = std::move(_factory);
}

void ParticleSystem::AddParticle(const std::string& _groupName, const std::string& _useModelName, std::unique_ptr<Particle> _particle, ParticleRenderSettings _settings, uint32_t _textureHandle, std::vector<std::string> _modifiers)
{

    ParticleKey key;
    key.modelName = _useModelName;
    key.settings = _settings;

    auto it = particles_.find(_groupName);
    if (it == particles_.end())
    {
        ParticleGroup& group = particles_[_groupName];
        group.key = key;
        group.srvIndex = srvManager_->Allocate();
        group.instanceCount = 0;
        group.particles.push_back(std::move(_particle));

        group.instanceBuffer = DXCommon::GetInstance()->CreateBufferResource(sizeof(ParticleForGPU) * maxInstancesPerGroup);
        group.instanceBuffer->Map(0, nullptr, reinterpret_cast<void**>(&group.mappedInstanceBuffer));

        srvManager_->CreateSRVForStructureBuffer(group.srvIndex, group.instanceBuffer.Get(), maxInstancesPerGroup, sizeof(ParticleForGPU));

        group.model = ModelManager::GetInstance()->FindSameModel(_useModelName);

        PSOFlags psoFlags = _settings.GetPSOFlags();
        psoFlags = psoFlags | PSOFlags::Type::Particle | PSOFlags::DepthMode::Comb_mZero_fLessEqual;

        // PSOFlagsが未登録の場合は登録する
        if (!psoMap_.contains(psoFlags))
            psoMap_[psoFlags] = PSOManager::GetInstance()->GetPipeLineStateObject(psoFlags).value();

        group.psoIndex = psoFlags;

        group.textureHandle = _textureHandle;

        // モディファイアの登録
        for (const std::string& name : _modifiers)
        {
            group.useModifierName[name] = 1;
        }
    }
    else
    {
        PSOFlags psoFlags = _settings.GetPSOFlags();
        psoFlags = psoFlags | PSOFlags::Type::Particle | PSOFlags::DepthMode::Comb_mZero_fLessEqual;

        // PSOFlagsが未登録の場合は登録する
        if (!psoMap_.contains(psoFlags))
            psoMap_[psoFlags] = PSOManager::GetInstance()->GetPipeLineStateObject(psoFlags).value();

        it->second.psoIndex = psoFlags;

        it->second.particles.push_back(std::move(_particle));
    }

    // モディファイアの登録
    for (const std::string& name : _modifiers)
    {
        particles_[_groupName].useModifierName[name] = 1;
        //particles_[key].useModifierName[name] = 1;
    }
}

void ParticleSystem::AddParticles(const std::string& _groupName, const std::string& _useModelName, std::vector<std::unique_ptr<Particle>> _particles, ParticleRenderSettings _settings, uint32_t _textureHandle, std::vector<std::string> _modifiers)
{

    ParticleKey key;
    key.modelName = _useModelName;
    key.settings = _settings;

    auto it = particles_.find(_groupName);
    if (it == particles_.end())
    {
        ParticleGroup& group = particles_[_groupName];
        group.key = key;

        group.srvIndex = srvManager_->Allocate();
        group.instanceCount = 0;

        for (auto& particle : _particles)
        {
            group.particles.push_back(std::move(particle));
        }

        group.instanceBuffer = DXCommon::GetInstance()->CreateBufferResource(sizeof(ParticleForGPU) * maxInstancesPerGroup);
        group.instanceBuffer->Map(0, nullptr, reinterpret_cast<void**>(&group.mappedInstanceBuffer));

        srvManager_->CreateSRVForStructureBuffer(group.srvIndex, group.instanceBuffer.Get(), maxInstancesPerGroup, sizeof(ParticleForGPU));

        group.model = ModelManager::GetInstance()->FindSameModel(_useModelName);
        if (group.model == nullptr)
            throw std::runtime_error("Modelname '" + _useModelName + "'  が無効です。");

        PSOFlags psoFlags = _settings.GetPSOFlags();
        psoFlags = psoFlags | PSOFlags::Type::Particle | PSOFlags::DepthMode::Comb_mZero_fLessEqual;

        // PSOFlagsが未登録の場合は登録する
        if (!psoMap_.contains(psoFlags))
            psoMap_[psoFlags] = PSOManager::GetInstance()->GetPipeLineStateObject(psoFlags).value();

        group.psoIndex = psoFlags;
        group.textureHandle = _textureHandle;
    }
    else
    {
        auto& group = it->second;

        // パーティクルだけを追加
        for (auto& particle : _particles)
        {
            group.particles.push_back(std::move(particle));
        }
        group.textureHandle = _textureHandle;

        PSOFlags psoFlags = _settings.GetPSOFlags();
        psoFlags = psoFlags | PSOFlags::Type::Particle | PSOFlags::DepthMode::Comb_mZero_fLessEqual;

        // PSOFlagsが未登録の場合は登録する
        if (!psoMap_.contains(psoFlags))
            psoMap_[psoFlags] = PSOManager::GetInstance()->GetPipeLineStateObject(psoFlags).value();

        group.psoIndex = psoFlags;

        // モデルが異なる場合のみ更新
        if (group.model == nullptr || group.key.modelName != _useModelName)
        {
            group.model = ModelManager::GetInstance()->FindSameModel(_useModelName);
            if (group.model == nullptr)
                throw std::runtime_error("Modelname '" + _useModelName + "'  が無効です。");
        }
    }

    // モディファイアの登録
    for (const std::string& name : _modifiers)
    {
        particles_[_groupName].useModifierName[name] = 1;
        //particles_[key].useModifierName[name] = 1;
    }
}

void ParticleSystem::ClearParticles()
{
    particles_.clear();
}

void ParticleSystem::ClearParticles(const std::string& _groupName)
{
    auto it = particles_.find(_groupName);
    if (it != particles_.end())
        it->second.particles.clear();
}

void ParticleSystem::CreateModifier(const std::string& _name)
{
    if (factory_ == nullptr)
    {
        throw std::runtime_error("IParticleMoifierFactoryが設定されていません。");
        return;
    }

    if (modifierNames_.find(_name) == modifierNames_.end())
    {
        auto modifier = factory_->CreateModifier(_name);
        if (modifier)
        {
            modifierNames_[_name] = std::move(modifier);
        }
    }
}
