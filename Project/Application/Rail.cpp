#include "Rail.h"
#include "PSOManager.h"
#include "SRVManager.h"
#include "DXCommon.h"
#include "CatmulRomSpline.h"
#include "TextureManager.h"
#include "MatrixFunction.h"

#include <numbers>

void Rail::Initialize(CatmulRomSpline* _edit)
{
    auto pso = PSOManager::GetInstance()->GetPipeLineStateObject("instansing");
    pipelineState_ = pso.value();
    assert(pso.has_value());

    auto rootSignature = PSOManager::GetInstance()->GetRootSignature("instansing");
    assert(rootSignature.has_value());
    rootsignature_ = rootSignature.value();

    const uint32_t kGroupMaxInstance = 2048;
    srvIndex_ = SRVManager::GetInstance()->Allocate();
    resource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(constantBufferData) * kGroupMaxInstance);
    resource_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));

    SRVManager::GetInstance()->CreateSRVForStructureBuffer(srvIndex_, resource_.Get(), kGroupMaxInstance, sizeof(constantBufferData));
    instanceNum_ = 0;

    model_ = Model::CreateFromObj("plane/plane.obj");
    textureHandle_ = TextureManager::GetInstance()->Load("rail.png");
    edit_ = _edit;
}

void Rail::Update()
{
}

void Rail::Draw(const Camera* _camera)
{
#ifdef _DEBUG
    ImGui();
#endif // _DEBUG

    ID3D12GraphicsCommandList* commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->SetPipelineState(pipelineState_);
    commandList->SetGraphicsRootSignature(rootsignature_);

    commandList->IASetVertexBuffers(0, 1, model_->GetMeshPtr()->GetVertexBufferView());
    commandList->IASetIndexBuffer(model_->GetMeshPtr()->GetIndexBufferView());

    commandList->SetGraphicsRootConstantBufferView(0, _camera->GetResource()->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(srvIndex_));
    commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetGPUHandle(textureHandle_));

    commandList->DrawIndexedInstanced(model_->GetMeshPtr()->GetIndexNum(), instanceNum_, 0, 0, 0);

}

void Rail::CalculateRail()
{
    trans_.clear();
    rot_.clear();
    edit_->GetTransAndRot(trans_, rot_, speed_);

    for (instanceNum_ = 0; instanceNum_ < trans_.size(); ++instanceNum_)
    {
        rot_[instanceNum_].x += std::numbers::pi_v<float> / 2.0f;
        constMap_[instanceNum_].matWorld = MakeAffineMatrix({ scale_ }, rot_[instanceNum_], trans_[instanceNum_]);
        constMap_[instanceNum_].color = { 1.0f,1.0f,1.0f,1.0f };
    }

}


#ifdef _DEBUG
#include <imgui.h>
void Rail::ImGui()
{
    ImGui::Begin("Rail");
    ImGui::DragFloat("scale", &scale_, 0.01f, 0.0f, 10.0f);
    ImGui::DragFloat("speed", &speed_, 0.01f, 0.0f, 100.0f);
    if(ImGui::Button("Calculate"))
    {
        CalculateRail();
    }
    ImGui::End();
}
#endif // _DEBUG
