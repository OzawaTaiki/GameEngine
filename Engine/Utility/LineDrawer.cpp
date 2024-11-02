#include "LineDrawer.h"
#include "PSOManager.h"
#include "DXCommon.h"
#include <cassert>
#include "VectorFunction.h"
#include "MatrixFunction.h"
LineDrawer* LineDrawer::GetInstance()
{
    static LineDrawer instance;
    return &instance;
}

void LineDrawer::Initialize()
{
    index = 0u;
    color_ = { 0.0f,0.0f,0.0f,1.0f };

    /// PSOを取得
    auto pso = PSOManager::GetInstance()->GetPipeLineStateObject("LineDrawer", PSOManager::BlendMode::Normal);
    // PSOが生成されているか確認
    assert(pso.has_value() && pso != nullptr);
    graphicsPipelineState_ = pso.value();

    /// RootSingnatureを取得
    auto rootSignature = PSOManager::GetInstance()->GetRootSignature("LineDrawer");
    // 生成されているか確認
    assert(rootSignature.has_value() && rootSignature != nullptr);
    rootSignature_ = rootSignature.value();

    // まｐ
    resources_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(ConstantBufferData));
    resources_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));

    vertexResource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(PointData) * kMaxNum);
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vConstMap_));

    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(PointData) * kMaxNum;
    vertexBufferView_.StrideInBytes = sizeof(PointData);

    SetVerties();
}

void LineDrawer::RegisterPoint(const Vector3& _start, const Vector3& _end)
{
    assert(index + 2 < kMaxNum && "The line instance is too large");

    vConstMap_[index++].position = { _start, 1.0f };
    vConstMap_[index++].position = { _end, 1.0f };
}

void LineDrawer::Draw()
{
    assert(cameraptr_ != nullptr);

    TransferData();
    auto commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->SetGraphicsRootSignature(rootSignature_);
    commandList->SetPipelineState(graphicsPipelineState_);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

    commandList->SetGraphicsRootConstantBufferView(0, resources_->GetGPUVirtualAddress());
    commandList->DrawInstanced(index, index / 2, 0, 0);

    index = 0u;
}

void LineDrawer::DrawOBB(const Matrix4x4& _affineMat)
{
    for (uint32_t index = 1; index < obbIndices_.size(); index += 2)
    {
        uint32_t sIndex = obbIndices_[index - 1];
        uint32_t eIndex = obbIndices_[index];

        Vector3 spos = Transform(obbVertices_[sIndex], _affineMat);
        Vector3 epos = Transform(obbVertices_[eIndex], _affineMat);

        RegisterPoint(spos, epos);
    }

}

void LineDrawer::TransferData()
{
    constMap_->color = color_;
    constMap_->vp = cameraptr_->GetViewProjection();
}

void LineDrawer::SetVerties()
{
    obbVertices_[0] = {-0.5f, 0.5f ,-0.5f };
    obbVertices_[1] = {-0.5f,-0.5f ,-0.5f };
    obbVertices_[2] = { 0.5f,-0.5f ,-0.5f };
    obbVertices_[3] = { 0.5f, 0.5f ,-0.5f };
    obbVertices_[4] = {-0.5f, 0.5f , 0.5f };
    obbVertices_[5] = {-0.5f,-0.5f , 0.5f };
    obbVertices_[6] = { 0.5f,-0.5f , 0.5f };
    obbVertices_[7] = { 0.5f, 0.5f , 0.5f };

    obbIndices_ = { 0,1,1,2,2,3,3,0,4,5,5,6,6,7,7,4,0,4,1,5,2,6,3,7 };
}
