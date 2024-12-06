#include "ParticleManager.h"
#include "Core/DirectX/DXCommon.h"
#include "Render/PSOManager.h"
#include "Render/SRVManager.h"
#include "TextureManager/TextureManager.h"
#include "Camera/Camera.h"
#include "Math/MatrixFunction.h"
#include "Particle/ParticleEmitters.h"
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

void ParticleManager::Initialize()
{
    srvManager_ = SRVManager::GetInstance();

    auto pso = PSOManager::GetInstance()->GetPipeLineStateObject("Particle",PSOManager::BlendMode::Add);
    pipelineState_ = pso.value();
    assert(pso.has_value());

   auto rootSignature = PSOManager::GetInstance()->GetRootSignature("Particle");
   assert(rootSignature.has_value());
   rootsignature_ = rootSignature.value();
}

void ParticleManager::Update(const Vector3& _cRotate)
{
    // billBordはshaderで計算したい

    Vector3 cRotate = {};
    Matrix4x4 billboradMat = Matrix4x4::Identity();

    for (auto& [name, group] : groups_)
    {
        group.instanceNum = 0;
        for (auto it = group.particles.begin(); it != group.particles.end();)
        {
            it->Update();
            if (!it->IsAlive())
            {
                it = group.particles.erase(it);
                continue;
            }

            auto axes = group.emitterPtr->GetBillboardAxes();

            for (uint32_t index = 0; index < 3; ++index)
            {
                if (axes[index])
                {
                    cRotate[index] = _cRotate[index];
                }
            }
            billboradMat = MakeAffineMatrix({ 1,1,1 }, cRotate, { 0,0,0 });
            //billboradMat = Inverse(billboradMat);

            Matrix4x4 mat = MakeScaleMatrix(it->GetScale());
            if (group.emitterPtr->EnableBillboard())
                mat = mat * billboradMat;
            if (group.emitterPtr->ShouldFaceDirection())
                mat = mat * it->GetDirectionMatrix();
            //mat = mat * MakeRotateMatrix(it->GetRotation());
            mat = mat * MakeTranslateMatrix(it->GetPosition());
            group.constMap[group.instanceNum].matWorld = mat;
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

        commandList->IASetVertexBuffers(0, 1, particles.model->GetMeshPtr()->GetVertexBufferView());
        commandList->IASetIndexBuffer(particles.model->GetMeshPtr()->GetIndexBufferView());

        commandList->SetGraphicsRootConstantBufferView(0, _camera->GetResource()->GetGPUVirtualAddress());
        commandList->SetGraphicsRootDescriptorTable(1, srvManager_->GetGPUSRVDescriptorHandle(particles.srvIndex));
        commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetGPUHandle(particles.textureHandle));

        commandList->DrawIndexedInstanced(particles.model->GetMeshPtr()->GetIndexNum(), particles.instanceNum, 0, 0, 0);
    }
}

void ParticleManager::CreateParticleGroup(const std::string& _groupName, const std::string& _modelPath, ParticleEmitter* _emitterPtr, uint32_t _textureHandle)
{
    if (groups_.contains(_groupName))
        return;

    if (!_emitterPtr)
        throw std::runtime_error("emitterPtr == nullPtr");

    Group& group = groups_[_groupName];
    group.model = Model::CreateFromObj(_modelPath);
    if (_textureHandle == UINT32_MAX)
        group.textureHandle = group.model->GetMaterialPtr()->GetTexturehandle();
    else
        group.textureHandle = _textureHandle;

    group.srvIndex = srvManager_->Allocate();
    group.resource = DXCommon::GetInstance()->CreateBufferResource(sizeof(constantBufferData) * kGroupMaxInstance);
    group.resource->Map(0, nullptr, reinterpret_cast<void**>(&group.constMap));

    srvManager_->CreateSRVForStructureBuffer(group.srvIndex, group.resource.Get(), kGroupMaxInstance, sizeof(constantBufferData));

    group.instanceNum = 0;

    group.emitterPtr = _emitterPtr;

}

void ParticleManager::SetGroupModel(const std::string& _groupName, const std::string& _modelPath)
{
    if (!groups_.contains(_groupName))
        throw std::runtime_error("not find particleGroup! name:" + '\"' + _groupName + '\"');

    groups_[_groupName].model = Model::CreateFromObj(_modelPath);
}

void ParticleManager::SetGroupTexture(const std::string& _groupName, uint32_t _textureHandle)
{
    if (!groups_.contains(_groupName))
        throw std::runtime_error("not find particleGroup! name:" + '\"' + _groupName + '\"');
    groups_[_groupName].textureHandle = _textureHandle;
}


void ParticleManager::AddParticleToGroup(const std::string& _groupName, const std::vector<Particle>& _particles)
{
    if (!groups_.contains(_groupName))
    {
        std::string err = "not find particleGroup! name:" + '\"' + _groupName + '\"';
        throw std::runtime_error(err);
    }

    for(const auto& particle:_particles)
    {
        groups_[_groupName].particles.push_back(particle);
    }

}

void ParticleManager::PreDraw()
{
    ID3D12GraphicsCommandList* commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->SetPipelineState(pipelineState_);
    commandList->SetGraphicsRootSignature(rootsignature_);
}

