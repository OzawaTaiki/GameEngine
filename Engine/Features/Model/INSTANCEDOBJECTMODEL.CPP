#include "InstancedObjectModel.h"
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/RTV/RTVManager.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Debug/Debug.h>

Engine::InstancedObjectModel::~InstancedObjectModel()
{
    if (srvIndex_ != kInvalidIndex)
    {
        SRVManager::GetInstance()->Free(srvIndex_);
    }
}

void Engine::InstancedObjectModel::Initialize(const std::string& modelPath, uint32_t maxInstances)
{
    model_ = Model::CreateFromFile(modelPath);
    if (!model_)
    {
        Debug::Log("Failed to load model: " + modelPath + "\n");
        return;
    }

    maxInstances_ = maxInstances;

    instanceResource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(InstanceData) * maxInstances_);
    // インスタンスデータのマッピング
    instanceResource_->Map(0, nullptr, reinterpret_cast<void**>(&instanceMap_));

    srvIndex_ = SRVManager::GetInstance()->Allocate();
    SRVManager::GetInstance()->CreateSRVForStructureBuffer(srvIndex_, instanceResource_.Get(), maxInstances_, sizeof(InstanceData));
    srvHandle_ = SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(srvIndex_);
}

void Engine::InstancedObjectModel::AddInstance(const Matrix4x4& worldMatrix, const Vector4& color)
{
    if (instanceCount_ >= maxInstances_)
    {
        Debug::Log("Exceeded maximum instance count\n");
        return;
    }

    instanceMap_[instanceCount_].world = worldMatrix;
    instanceMap_[instanceCount_].worldInverseTranspose =Transpose(Inverse(worldMatrix));
    instanceMap_[instanceCount_].color = color;
    instanceCount_++;
}

void Engine::InstancedObjectModel::Draw(const Camera* camera)
{
    if (!model_ || instanceCount_ == 0)
        return;

    auto cmd = DXCommon::GetInstance()->GetCommandList();

    PSOManager::GetInstance()->SetRegisterRootSignature("InstancedModel");
    PSOManager::GetInstance()->SetRegisterPSO("InstancedModel");
    cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    auto lightGroup = LightingSystem::GetInstance()->GetLightGroup();
    if (lightGroup)
    {
        auto pointLights = lightGroup->GetAllPointLights();
        if (!pointLights.empty())
        {
            auto handles = pointLights[0]->GetShadowMapHandles();
            RTVManager::GetInstance()->QueuePointLightShadowMapToSRV(handles[0], 6); // [6]
        }
    }
    RTVManager::GetInstance()->GetRenderTexture("ShadowMap")->QueueCommandDSVtoSRV(5); // [5]



    for (auto& mesh : model_->GetMeshes())
    {
        mesh->QueueCommand(cmd);

        // [0] Camera
        cmd->SetGraphicsRootConstantBufferView(0, camera->GetResource()->GetGPUVirtualAddress());
        // [1] InstanceData SRV
        cmd->SetGraphicsRootDescriptorTable(1, srvHandle_);
        // [2] gMaterial
        auto* mat = model_->GetMaterials()[mesh->GetUseMaterialIndex()].get();
        mat->TransferData();
        mat->MaterialQueueCommand(cmd, 2);
        // [3] gTexture
        mat->TextureQueueCommand(cmd, 3);
        // [4] gLightGroup + shadow maps
        model_->QueueLightCommand(cmd, 4);

        cmd->DrawIndexedInstanced(mesh->GetIndexNum(), instanceCount_, 0, 0, 0);
    }
}

void Engine::InstancedObjectModel::Clear()
{
    instanceCount_ = 0;
}
