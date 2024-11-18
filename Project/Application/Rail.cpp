#include "Rail.h"
#include "PSOManager.h"
#include "SRVManager.h"
#include "DXCommon.h"
#include "CatmulRomSpline.h"
#include "MatrixFunction.h"

void Rail::Initialize()
{
    auto pso = PSOManager::GetInstance()->GetPipeLineStateObject("Particle");
    pipelineState_ = pso.value();
    assert(pso.has_value());

    auto rootSignature = PSOManager::GetInstance()->GetRootSignature("Particle");
    assert(rootSignature.has_value());
    rootsignature_ = rootSignature.value();

    const uint32_t kGroupMaxInstance = 2048;
    srvIndex_ = SRVManager::GetInstance()->Allocate();
    resource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(constantBufferData) * kGroupMaxInstance);
    resource_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));

    SRVManager::GetInstance()->CreateSRVForStructureBuffer(srvIndex_, resource_.Get(), kGroupMaxInstance, sizeof(constantBufferData));
    instanceNum_ = 0;

    model_ = Model::CreateFromObj("plane/plane.obj");
}

void Rail::Update()
{
    for (instanceNum_ = 0; instanceNum_ < trans_.size(); ++instanceNum_)
    {
        constMap_[instanceNum_].matWorld = MakeAffineMatrix({ scale_ }, rot_[instanceNum_], trans_[instanceNum_]);
        constMap_[instanceNum_].color = { 1.0f,1.0f,1.0f,1.0f };
    }
}

void Rail::Draw(const Camera* _camera)
{

    ID3D12GraphicsCommandList* commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->SetPipelineState(pipelineState_);
    commandList->SetGraphicsRootSignature(rootsignature_);

    commandList->IASetVertexBuffers(0, 1, model_->GetMeshPtr()->GetVertexBufferView());
    commandList->IASetIndexBuffer(model_->GetMeshPtr()->GetIndexBufferView());

    commandList->SetGraphicsRootConstantBufferView(0, _camera->GetResource()->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(srvIndex_));
    commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetGPUHandle(particles.textureHandle));

    commandList->DrawIndexedInstanced(particles.model->GetMeshPtr()->GetIndexNum(), particles.instanceNum, 0, 0, 0);


}

void Rail::CalculateRail(CatmulRomSpline* _edit)
{
    _edit->GetTransAndRot(trans_, rot_);
}


#ifdef _DEBUG
#include <imgui.h>
void Rail::ImGui()
{

}
#endif // _DEBUG
