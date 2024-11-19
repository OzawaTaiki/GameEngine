#include "LineDrawer.h"
#include "PSOManager.h"
#include "DXCommon.h"
#include <cassert>
#include <numbers>
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

void LineDrawer::DrawOBB(const std::array<Vector3, 8>& _vertices)
{
    for (uint32_t index = 1; index < obbIndices_.size(); index += 2)
    {
        uint32_t sIndex = obbIndices_[index - 1];
        uint32_t eIndex = obbIndices_[index];

        Vector3 spos = _vertices[sIndex];
        Vector3 epos = _vertices[eIndex];

        RegisterPoint(spos, epos);
    }
}

void LineDrawer::DrawSphere(const Matrix4x4& _affineMat)
{
    for (uint32_t index = 1; index < sphereIndices_.size(); index += 2)
    {
        uint32_t sIndex = sphereIndices_[index - 1];
        uint32_t eIndex = sphereIndices_[index];

        Vector3 spos = Transform(sphereVertices_[sIndex], _affineMat);
        Vector3 epos = Transform(sphereVertices_[eIndex], _affineMat);

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
    obbVertices_[0] = { 0.5f, 0.5f , 0.5f };
    obbVertices_[1] = { 0.5f, 0.5f ,-0.5f };
    obbVertices_[2] = { 0.5f,-0.5f , 0.5f };
    obbVertices_[3] = { 0.5f,-0.5f ,-0.5f };
    obbVertices_[4] = {-0.5f, 0.5f , 0.5f };
    obbVertices_[5] = {-0.5f, 0.5f ,-0.5f };
    obbVertices_[6] = {-0.5f,-0.5f , 0.5f };
    obbVertices_[7] = {-0.5f,-0.5f ,-0.5f };

    obbIndices_ = { 0,1,0,2,0,4,1,3,1,5,2,6,2,3,3,7,4,5,4,6,5,7,6,7 };


    //sphere頂点の計算
    const float kLatEvery = std::numbers::pi_v<float> / kDivision;
    const float kLonEvery = std::numbers::pi_v<float> *2.0f / kDivision;

    for (uint32_t lat = 0; lat < kDivision; ++lat)
    {
        for (uint32_t lon = 0; lon < kDivision; ++lon)
        {
            float latRad = lat * kLatEvery;
            float lonRad = lon * kLonEvery;

            float x = std::sin(latRad) * std::cos(lonRad);
            float y = std::cos(latRad);
            float z = std::sin(latRad) * std::sin(lonRad);

            sphereVertices_.emplace_back(x, y, z);
        }
    }
    uint32_t div = static_cast<uint32_t> (kDivision);
    for (uint32_t lat = 0; lat < div - 1; ++lat) {
        for (uint32_t lon = 0; lon < div; ++lon) {
            uint32_t current = lat * div + lon;
            uint32_t nextLon = (lon + 1) % div; // 経度方向でループ

            uint32_t nextLat = (lat + 1) * div + lon;
            uint32_t nextLonLat = (lat + 1) * div + nextLon;

            sphereIndices_.push_back(current);
            sphereIndices_.push_back((lat * div + nextLon) % sphereVertices_.size());

            sphereIndices_.push_back(current);
            sphereIndices_.push_back(nextLat);
        }
    }
}
