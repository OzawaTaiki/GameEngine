#include "LineDrawer.h"
#include <Core/DXCommon/DXCommon.h>
#include <Math/Vector/VectorFunction.h>
#include <Math/Matrix/MatrixFunction.h>
#include <cassert>
#include <numbers>

namespace
{
LayerID debugLineLayer = 0;
}

LineDrawer* LineDrawer::GetInstance()
{
    static LineDrawer instance;
    return &instance;
}

void LineDrawer::Initialize()
{
    psoFlags_ = PSOFlags::ForLineDrawer();

    color_ = { 0.0f, 0.0f, 0.0f, 1.0f };

    // PSOを取得
    auto pso = PSOManager::GetInstance()->GetPipeLineStateObject(psoFlags_);
    assert(pso.has_value() && pso != nullptr);
    graphicsPipelineState_ = pso.value();

    // RootSignatureを取得
    auto rootSignature = PSOManager::GetInstance()->GetRootSignature(psoFlags_);
    assert(rootSignature.has_value() && rootSignature != nullptr);
    rootSignature_ = rootSignature.value();

    // 常に手前描画用のPSO
    PSOFlags psoFlagsForAlways = PSOFlags::ForLineDrawerAlways();
    auto psoForAlways = PSOManager::GetInstance()->GetPipeLineStateObject(psoFlagsForAlways);
    assert(psoForAlways.has_value() && psoForAlways != nullptr);
    graphicsPipelineStateForAlways_ = psoForAlways.value();

    // 3D用定数バッファ
    resourcesForMat3D_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(ConstantBufferData));
    resourcesForMat3D_->Map(0, nullptr, reinterpret_cast<void**>(&matFor3DConstMap_));

    // 3D用頂点バッファ
    vertexResourceFor3D_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(PointData) * kMaxNum);
    vertexResourceFor3D_->Map(0, nullptr, reinterpret_cast<void**>(&vConstMapFor3D_));

    vertexBufferViewFor3D_.BufferLocation = vertexResourceFor3D_->GetGPUVirtualAddress();
    vertexBufferViewFor3D_.SizeInBytes = sizeof(PointData) * kMaxNum;
    vertexBufferViewFor3D_.StrideInBytes = sizeof(PointData);

    // 3D Always用頂点バッファ
    vertexResourceFor3DAlways_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(PointData) * kMaxNum);
    vertexResourceFor3DAlways_->Map(0, nullptr, reinterpret_cast<void**>(&vConstMapFor3DAlways_));

    vertexBufferViewFor3DAlways_.BufferLocation = vertexResourceFor3DAlways_->GetGPUVirtualAddress();
    vertexBufferViewFor3DAlways_.SizeInBytes = sizeof(PointData) * kMaxNum;
    vertexBufferViewFor3DAlways_.StrideInBytes = sizeof(PointData);

    // 2D用定数バッファ
    resourceForMat2D_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(ConstantBufferData));
    resourceForMat2D_->Map(0, nullptr, reinterpret_cast<void**>(&matFor2DConstMap_));

    // 2D用頂点バッファ
    vertexResourceFor2D_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(PointData) * kMaxNum);
    vertexResourceFor2D_->Map(0, nullptr, reinterpret_cast<void**>(&vConstMapFor2D_));

    vertexBufferViewFor2D_.BufferLocation = vertexResourceFor2D_->GetGPUVirtualAddress();
    vertexBufferViewFor2D_.SizeInBytes = sizeof(PointData) * kMaxNum;
    vertexBufferViewFor2D_.StrideInBytes = sizeof(PointData);

    // 形状頂点初期化
    InitializeShapeVertices();

    defaultCamera2D_.Initialize(CameraType::Orthographic);
    cameraFor2D_ = &defaultCamera2D_;

    debugLineLayer = LayerSystem::CreateLayer("DebugLine", 100000);
}

void LineDrawer::RegisterPoint(const Vector3& _start, const Vector3& _end, bool _frontDraw)
{
    RegisterPoint(_start, _end, color_, _frontDraw);
}

void LineDrawer::RegisterPoint(const Vector3& _start, const Vector3& _end, const Vector4& _color, bool _frontDraw)
{
    // 現在のレイヤーを自動取得
    LayerID currentLayer = LayerSystem::GetCurrentLayerID();

    PointData startPoint = { Vector4(_start.x, _start.y, _start.z, 1.0f), _color };
    PointData endPoint = { Vector4(_end.x, _end.y, _end.z, 1.0f), _color };

    if (_frontDraw)
    {
        layerData_[currentLayer].lines3DAlways_.push_back(startPoint);
        layerData_[currentLayer].lines3DAlways_.push_back(endPoint);
    }
    else
    {
        layerData_[currentLayer].lines3D_.push_back(startPoint);
        layerData_[currentLayer].lines3D_.push_back(endPoint);
    }
}

void LineDrawer::RegisterPoint(const Vector2& _start, const Vector2& _end)
{
    RegisterPoint(_start, _end, color_);
}

void LineDrawer::RegisterPoint(const Vector2& _start, const Vector2& _end, const Vector4& _color)
{
    // 現在のレイヤーを自動取得
    LayerID currentLayer = LayerSystem::GetCurrentLayerID();

    PointData startPoint = { Vector4(_start.x, _start.y, 0.0f, 1.0f), _color };
    PointData endPoint = { Vector4(_end.x, _end.y, 0.0f, 1.0f), _color };

    layerData_[currentLayer].lines2D_.push_back(startPoint);
    layerData_[currentLayer].lines2D_.push_back(endPoint);
}

void LineDrawer::DrawOBB(const Matrix4x4& _affineMat, bool _frontDraw)
{
    DrawOBB(_affineMat, color_, _frontDraw);
}

void LineDrawer::DrawOBB(const Matrix4x4& _affineMat, const Vector4& _color, bool _frontDraw)
{
    for (uint32_t index = 1; index < obbIndices_.size(); index += 2)
    {
        uint32_t sIndex = obbIndices_[index - 1];
        uint32_t eIndex = obbIndices_[index];
        Vector3 spos = Transform(obbVertices_[sIndex], _affineMat);
        Vector3 epos = Transform(obbVertices_[eIndex], _affineMat);
        RegisterPoint(spos, epos, _color, _frontDraw);
    }
}

void LineDrawer::DrawOBB(const std::array<Vector3, 8>& _vertices, bool _frontDraw)
{
    DrawOBB(_vertices, color_, _frontDraw);
}

void LineDrawer::DrawOBB(const std::array<Vector3, 8>& _vertices, const Vector4& _color, bool _frontDraw)
{
    for (uint32_t index = 1; index < obbIndices_.size(); index += 2)
    {
        uint32_t sIndex = obbIndices_[index - 1];
        uint32_t eIndex = obbIndices_[index];

        Vector3 spos = _vertices[sIndex];
        Vector3 epos = _vertices[eIndex];

        RegisterPoint(spos, epos, _color, _frontDraw);
    }
}

void LineDrawer::DrawSphere(const Matrix4x4& _affineMat, bool _frontDraw)
{
    DrawSphere(_affineMat, color_, _frontDraw);
}

void LineDrawer::DrawSphere(const Matrix4x4& _affineMat, const Vector4& _color, bool _frontDraw)
{
    for (uint32_t index = 1; index < sphereIndices_.size(); index += 2)
    {
        uint32_t sIndex = sphereIndices_[index - 1];
        uint32_t eIndex = sphereIndices_[index];

        Vector3 spos = Transform(sphereVertices_[sIndex], _affineMat);
        Vector3 epos = Transform(sphereVertices_[eIndex], _affineMat);

        RegisterPoint(spos, epos, _color, _frontDraw);
    }
}

void LineDrawer::DrawCircle(const Vector3& _center, float _radius, float _segmentCount, const Vector3& _normal, bool _frontDraw)
{
    DrawCircle(_center, _radius, _segmentCount, _normal, color_, _frontDraw);
}

void LineDrawer::DrawCircle(const Vector3& _center, float _radius, float _segmentCount, const Vector3& _normal, const Vector4& _color, bool _frontDraw)
{
    // 法線を正規化
    Vector3 normal = _normal.Normalize();

    // 法線ベクトルに垂直な2つのベクトルを見つける
    Vector3 tangent1, tangent2;

    // 法線ベクトルが上向きベクトル(0,1,0)と近い場合は別の基準ベクトルを使用
    if (std::abs(normal.Dot(Vector3(0, 1, 0))) > 0.99f)
    {
        tangent1 = Vector3(0, 0, 1);
    }
    else
    {
        tangent1 = normal.Cross(Vector3(0, 1, 0)).Normalize();
    }

    tangent2 = normal.Cross(tangent1).Normalize();

    const float kEvery = std::numbers::pi_v<float> *2.0f / _segmentCount;

    for (uint32_t index = 0; index < static_cast<uint32_t>(_segmentCount); ++index)
    {
        float rad = index * kEvery;
        float nextRad = (index + 1) * kEvery;

        Vector3 spos = _center +
            tangent1 * std::cos(rad) * _radius +
            tangent2 * std::sin(rad) * _radius;

        Vector3 epos = _center +
            tangent1 * std::cos(nextRad) * _radius +
            tangent2 * std::sin(nextRad) * _radius;

        RegisterPoint(spos, epos, _color, _frontDraw);
    }
}

void LineDrawer::Draw()
{
    // データが何もなければ早期リターン
    bool hasAnyData = false;
    for (const auto& [layerID, data] : layerData_)
    {
        if (data.HasData())
        {
            hasAnyData = true;
            break;
        }
    }
    if (!hasAnyData) return;

    // コマンドリスト設定
    auto commandList = DXCommon::GetInstance()->GetCommandList();
    commandList->SetGraphicsRootSignature(rootSignature_);
    commandList->SetPipelineState(graphicsPipelineState_);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    // カメラ行列転送
    TransferCameraMatrix3D();
    TransferCameraMatrix2D();

    // 各種描画
    Draw3DLines();
    Draw3DLinesAlways();
    Draw2DLines();

    // 描画後にクリア
    for (auto& [layerID, data] : layerData_)
    {
        data.Clear();
    }
}

void LineDrawer::DebugDraw([[maybe_unused]] const Vector2& start, [[maybe_unused]] const Vector2& end, [[maybe_unused]] const Vector4& color)
{
#ifdef _DEBUG
    LayerID id = LayerSystem::GetCurrentLayerID();
    bool needChange = id != debugLineLayer;

    if (needChange)
        LayerSystem::SetLayer("DebugLine");

    RegisterPoint(start, end, color);

    if(needChange)
        LayerSystem::SetLayer(id);
#endif //_DEBUG
}

void LineDrawer::DebugDrawCircle([[maybe_unused]] const Vector2& center, [[maybe_unused]]float radius, [[maybe_unused]] const Vector4& color)
{
#ifdef _DEBUG
    LayerID id = LayerSystem::GetCurrentLayerID();
    LayerSystem::SetLayer("DebugLine");

    const float kEvery = std::numbers::pi_v<float> *2.0f / 32.0f; // 32分割
    for (uint32_t index = 0; index < 32; ++index)
    {
        float rad = index * kEvery;
        float nextRad = (index + 1) * kEvery;
        Vector2 spos = center + Vector2{ std::cos(rad) * radius, std::sin(rad) * radius };
        Vector2 epos = center + Vector2{ std::cos(nextRad) * radius, std::sin(nextRad) * radius };
        DebugDraw(spos, epos, color);
    }
    LayerSystem::SetLayer(id);
#endif //_DEBUG
}

void LineDrawer::Draw3DLines()
{
    // Step1: オフセット計算 + GPU転送
    uint32_t offset = 0;
    std::vector<std::pair<LayerID, uint32_t>> offsets;

    for (auto& [layerID, data] : layerData_)
    {
        if (data.lines3D_.empty()) continue;

        assert(offset + data.lines3D_.size() <= kMaxNum && "Too many 3D line vertices");

        offsets.push_back({ layerID, offset });
        std::memcpy(vConstMapFor3D_ + offset,
                    data.lines3D_.data(),
                    data.lines3D_.size() * sizeof(PointData));
        offset += static_cast<uint32_t>(data.lines3D_.size());
    }

    if (offsets.empty()) return;
    assert(cameraFor3D_ != nullptr && "3D Camera is not set");

    // Step2: レイヤーごとに描画
    auto commandList = DXCommon::GetInstance()->GetCommandList();
    commandList->IASetVertexBuffers(0, 1, &vertexBufferViewFor3D_);
    commandList->SetGraphicsRootConstantBufferView(0, resourcesForMat3D_->GetGPUVirtualAddress());

    for (size_t i = 0; i < offsets.size(); ++i)
    {
        auto [layerID, startOffset] = offsets[i];
        uint32_t vertexCount = (i + 1 < offsets.size())
            ? offsets[i + 1].second - startOffset
            : offset - startOffset;

        LayerSystem::SetLayer(layerID);
        commandList->DrawInstanced(vertexCount, vertexCount / 2, startOffset, 0);
    }
}

void LineDrawer::Draw3DLinesAlways()
{
    // Step1: オフセット計算 + GPU転送
    uint32_t offset = 0;
    std::vector<std::pair<LayerID, uint32_t>> offsets;

    for (auto& [layerID, data] : layerData_)
    {
        if (data.lines3DAlways_.empty()) continue;

        assert(offset + data.lines3DAlways_.size() <= kMaxNum && "Too many 3D Always line vertices");

        offsets.push_back({ layerID, offset });
        std::memcpy(vConstMapFor3DAlways_ + offset,
                    data.lines3DAlways_.data(),
                    data.lines3DAlways_.size() * sizeof(PointData));
        offset += static_cast<uint32_t>(data.lines3DAlways_.size());
    }

    if (offsets.empty()) return;
    assert(cameraFor3D_ != nullptr && "3D Camera is not set");

    // Step2: レイヤーごとに描画（深度テスト無効のPSO）
    auto commandList = DXCommon::GetInstance()->GetCommandList();
    commandList->SetPipelineState(graphicsPipelineStateForAlways_);
    commandList->IASetVertexBuffers(0, 1, &vertexBufferViewFor3DAlways_);
    commandList->SetGraphicsRootConstantBufferView(0, resourcesForMat3D_->GetGPUVirtualAddress());

    for (size_t i = 0; i < offsets.size(); ++i)
    {
        auto [layerID, startOffset] = offsets[i];
        uint32_t vertexCount = (i + 1 < offsets.size())
            ? offsets[i + 1].second - startOffset
            : offset - startOffset;

        LayerSystem::SetLayer(layerID);
        commandList->DrawInstanced(vertexCount, vertexCount / 2, startOffset, 0);
    }

    // PSOを戻す
    commandList->SetPipelineState(graphicsPipelineState_);
}

void LineDrawer::Draw2DLines()
{
    // Step1: オフセット計算 + GPU転送
    uint32_t offset = 0;
    std::vector<std::pair<LayerID, uint32_t>> offsets;

    for (auto& [layerID, data] : layerData_)
    {
        if (data.lines2D_.empty()) continue;

        assert(offset + data.lines2D_.size() <= kMaxNum && "Too many 2D line vertices");

        offsets.push_back({ layerID, offset });
        std::memcpy(vConstMapFor2D_ + offset,
                    data.lines2D_.data(),
                    data.lines2D_.size() * sizeof(PointData));
        offset += static_cast<uint32_t>(data.lines2D_.size());
    }

    if (offsets.empty()) return;
    assert(cameraFor2D_ != nullptr && "2D Camera is not set");

    // Step2: レイヤーごとに描画
    auto commandList = DXCommon::GetInstance()->GetCommandList();
    commandList->IASetVertexBuffers(0, 1, &vertexBufferViewFor2D_);
    commandList->SetGraphicsRootConstantBufferView(0, resourceForMat2D_->GetGPUVirtualAddress());

    for (size_t i = 0; i < offsets.size(); ++i)
    {
        auto [layerID, startOffset] = offsets[i];
        uint32_t vertexCount = (i + 1 < offsets.size())
            ? offsets[i + 1].second - startOffset
            : offset - startOffset;

        LayerSystem::SetLayer(layerID);
        commandList->DrawInstanced(vertexCount, vertexCount / 2, startOffset, 0);
    }
}

void LineDrawer::TransferCameraMatrix3D()
{
    if (cameraFor3D_)
        matFor3DConstMap_->vp = cameraFor3D_->GetViewProjection();
}

void LineDrawer::TransferCameraMatrix2D()
{
    if (cameraFor2D_)
        matFor2DConstMap_->vp = cameraFor2D_->GetViewProjection();
}

void LineDrawer::InitializeShapeVertices()
{
    // OBB頂点
    obbVertices_[0] = { 0.5f, 0.5f , 0.5f };
    obbVertices_[1] = { 0.5f, 0.5f ,-0.5f };
    obbVertices_[2] = { 0.5f,-0.5f , 0.5f };
    obbVertices_[3] = { 0.5f,-0.5f ,-0.5f };
    obbVertices_[4] = { -0.5f, 0.5f , 0.5f };
    obbVertices_[5] = { -0.5f, 0.5f ,-0.5f };
    obbVertices_[6] = { -0.5f,-0.5f , 0.5f };
    obbVertices_[7] = { -0.5f,-0.5f ,-0.5f };

    obbIndices_ = { 0,1,0,2,0,4,1,3,1,5,2,6,2,3,3,7,4,5,4,6,5,7,6,7 };

    // Sphere頂点の計算
    const float kLatEvery = std::numbers::pi_v<float> / kDivision;
    const float kLonEvery = std::numbers::pi_v<float> *2.0f / kDivision;

    for (uint32_t lat = 0; lat < static_cast<uint32_t>(kDivision); ++lat)
    {
        for (uint32_t lon = 0; lon < static_cast<uint32_t>(kDivision); ++lon)
        {
            float latRad = lat * kLatEvery;
            float lonRad = lon * kLonEvery;

            float x = std::sin(latRad) * std::cos(lonRad);
            float y = std::cos(latRad);
            float z = std::sin(latRad) * std::sin(lonRad);

            sphereVertices_.emplace_back(x, y, z);
        }
    }

    uint32_t div = static_cast<uint32_t>(kDivision);
    for (uint32_t lat = 0; lat < div - 1; ++lat)
    {
        for (uint32_t lon = 0; lon < div; ++lon)
        {
            uint32_t current = lat * div + lon;
            uint32_t nextLon = (lon + 1) % div;
            uint32_t nextLat = (lat + 1) * div + lon;

            sphereIndices_.push_back(current);
            sphereIndices_.push_back((lat * div + nextLon) % static_cast<uint32_t>(sphereVertices_.size()));

            sphereIndices_.push_back(current);
            sphereIndices_.push_back(nextLat);
        }
    }
}
