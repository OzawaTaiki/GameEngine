#include <Features/Effect/Manager/ParticleManager.h>
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Camera/Camera/Camera.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Features/Effect/Emitter/ParticleEmitter.h>
#include <Features/Model/Manager/ModelManager.h>

#include <cassert>

// 静的メンバ変数の初期化
uint32_t ParticleManager::maxInstancesPerGroup = 1024;


ParticleManager* ParticleManager::GetInstance()
{
    static ParticleManager instance;
    return &instance;
}

void ParticleManager::Initialize()
{
    srvManager_ = SRVManager::GetInstance();

    auto rootsig = PSOManager::GetInstance()->GetRootSignature(PSOFlags::Type_Particle);
    assert(rootsig.has_value());
    rootSignature_ = rootsig.value();
}

void ParticleManager::Update(float _deltaTime)
{
    if (camera_ == nullptr)
    {
        throw std::runtime_error("Cameraが設定されていません。ポインタをセットしてください");
        return;
    }

    for (auto& [key, particleList] : particles_)
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

        for (auto it = particles.begin(); it != particles.end();)
        {
            Particle* particle = *it;

            particle->Update(_deltaTime);

            if (!particle->IsAlive())
            {
                delete particle;
                it = particles.erase(it);
                continue;
            }

            Matrix4x4 affineMatrix = MakeScaleMatrix(particle->GetScale());
            affineMatrix *= billboardMatrix;
            affineMatrix *= MakeRotateMatrix(particle->GetRotation());
            affineMatrix *= MakeTranslateMatrix(particle->GetPosition());

            particleList.mappedInstanceBuffer[particleList.instanceCount].worldMatrix = affineMatrix;
            particleList.mappedInstanceBuffer[particleList.instanceCount].color = particle->GetColor();

            particleList.instanceCount++;
            ++it;
        }
    }
}

void ParticleManager::DrawParticles()
{
    auto cmdList = DXCommon::GetInstance()->GetCommandList();

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

void ParticleManager::AddParticle(const std::string& _useModelName, Particle* _particle, ParticleRenderSettings _settings, uint32_t _textureHandle)
{
    ParticleKey key;
    key.modelName = _useModelName;
    key.settings = _settings;

    auto it = particles_.find(key);
    if (it == particles_.end())
    {
        ParticleGroup group;
        group.srvIndex = srvManager_->Allocate();
        group.instanceCount = 0;
        group.particles.push_back(_particle);

        group.instanceBuffer = DXCommon::GetInstance()->CreateBufferResource(sizeof(ParticleForGPU) * maxInstancesPerGroup);
        group.instanceBuffer->Map(0, nullptr, reinterpret_cast<void**>(&group.mappedInstanceBuffer));

        srvManager_->CreateSRVForStructureBuffer(group.srvIndex, group.instanceBuffer.Get(), maxInstancesPerGroup, sizeof(ParticleForGPU));

        group.model = ModelManager::GetInstance()->FindSameModel(_useModelName);

        PSOFlags psoFlags = _settings.GetPSOFlags();
        psoFlags |= PSOFlags::Type_Particle;

        // PSOFlagsが未登録の場合は登録する
        if (!psoMap_.contains(psoFlags))
            psoMap_[psoFlags] = PSOManager::GetInstance()->GetPipeLineStateObject(psoFlags).value();

        group.psoIndex = psoFlags;

        group.textureHandle = _textureHandle;

        particles_[key] = group;
    }
    else
    {
        it->second.particles.push_back(_particle);
    }

}

void ParticleManager::AddParticles(const std::string& _useModelName, std::vector<Particle*> _particles, ParticleRenderSettings _settings, uint32_t _textureHandle)
{
    ParticleKey key;
    key.modelName = _useModelName;
    key.settings = _settings;

    auto it = particles_.find(key);
    if (it == particles_.end())
    {
        ParticleGroup group;
        group.srvIndex = srvManager_->Allocate();
        group.instanceCount = 0;
        group.particles = std::list<Particle*>(_particles.begin(), _particles.end());

        group.instanceBuffer = DXCommon::GetInstance()->CreateBufferResource(sizeof(ParticleForGPU) * maxInstancesPerGroup);
        group.instanceBuffer->Map(0, nullptr, reinterpret_cast<void**>(&group.mappedInstanceBuffer));

        srvManager_->CreateSRVForStructureBuffer(group.srvIndex, group.instanceBuffer.Get(), maxInstancesPerGroup, sizeof(ParticleForGPU));

        group.model = ModelManager::GetInstance()->FindSameModel(_useModelName);
        if (group.model == nullptr)
            throw std::runtime_error("Modelname '" + _useModelName + "'  が無効です。");

        PSOFlags psoFlags = _settings.GetPSOFlags();
        psoFlags |= PSOFlags::Type_Particle;

        // PSOFlagsが未登録の場合は登録する
        if (!psoMap_.contains(psoFlags))
            psoMap_[psoFlags] = PSOManager::GetInstance()->GetPipeLineStateObject(psoFlags).value();

        group.psoIndex = psoFlags;
        group.textureHandle = _textureHandle;

        particles_[key] = group;
    }
    else
    {
        it->second.particles.insert(it->second.particles.end(), _particles.begin(), _particles.end());
    }
}

void ParticleManager::ClearParticles()
{
    for (auto& [key, particleList] : particles_)
    {
        for (auto& particle : particleList.particles)
        {
            delete particle;
        }
        particleList.particles.clear();
    }
    particles_.clear();
}

void ParticleManager::ClearParticles(const std::string& _useModelName)
{
    for (auto& [key, particleList] : particles_)
    {
        if (key.modelName == _useModelName)
        {
            for (auto& particle : particleList.particles)
            {
                delete particle;
            }
            particleList.particles.clear();
            particles_.erase(key);
            break;
        }
    }
}
