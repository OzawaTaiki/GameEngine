#include <Features/LineDrawer/LineDrawer.h>
#include <Core/DXCommon/DXCommon.h>
#include <cassert>
#include <numbers>
#include <Math/Vector/VectorFunction.h>
#include <Math/Matrix/MatrixFunction.h>

LineDrawer* LineDrawer::GetInstance()
{
    static LineDrawer instance;
    return &instance;
}

void LineDrawer::Initialize()
{
    psoFlags_ = PSOFlags::ForLineDrawer();

    indexFor3d_ = 0u;
    indexFor2d_ = 0u;
    color_ = { 0.0f,0.0f,0.0f,1.0f };

    /// PSOを取得
    auto pso = PSOManager::GetInstance()->GetPipeLineStateObject(psoFlags_);
    // PSOが生成されているか確認
    assert(pso.has_value() && pso != nullptr);
    graphicsPipelineState_ = pso.value();

    /// RootSingnatureを取得
    auto rootSignature = PSOManager::GetInstance()->GetRootSignature(psoFlags_);
    // 生成されているか確認
    assert(rootSignature.has_value() && rootSignature != nullptr);
    rootSignature_ = rootSignature.value();

    // まｐ
    resourcesForMat3D_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(ConstantBufferData));
    resourcesForMat3D_->Map(0, nullptr, reinterpret_cast<void**>(&matFor3dConstMap_));

    vertexResourceFor3D_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(PointData) * kMaxNum);
    vertexResourceFor3D_->Map(0, nullptr, reinterpret_cast<void**>(&vConstMapFor3D_));

    vertexBufferViewFor3D_.BufferLocation = vertexResourceFor3D_->GetGPUVirtualAddress();
    vertexBufferViewFor3D_.SizeInBytes = sizeof(PointData) * kMaxNum;
    vertexBufferViewFor3D_.StrideInBytes = sizeof(PointData);



    resourceForMat2D_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(ConstantBufferData));
    resourceForMat2D_->Map(0, nullptr, reinterpret_cast<void**>(&matFor2dConstMap_));

    vertexResourceFor2D_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(PointData) * kMaxNum);
    vertexResourceFor2D_->Map(0, nullptr, reinterpret_cast<void**>(&vConstMapFor2D_));

    vertexBufferViewFor2D_.BufferLocation = vertexResourceFor2D_->GetGPUVirtualAddress();
    vertexBufferViewFor2D_.SizeInBytes = sizeof(PointData) * kMaxNum;
    vertexBufferViewFor2D_.StrideInBytes = sizeof(PointData);



    SetVerties();
}

void LineDrawer::RegisterPoint(const Vector3& _start, const Vector3& _end)
{
    RegisterPoint(_start, _end, color_);
}

void LineDrawer::RegisterPoint(const Vector3& _start, const Vector3& _end, const Vector4& _color)
{
    assert(indexFor3d_ + 2 < kMaxNum && "The line instance is too large");

    vConstMapFor3D_[indexFor3d_].position = { _start, 1.0f };
    vConstMapFor3D_[indexFor3d_++].color = _color;

    vConstMapFor3D_[indexFor3d_].position = { _end, 1.0f };
    vConstMapFor3D_[indexFor3d_++].color = _color;
}

void LineDrawer::RegisterPoint(const Vector2& _start, const Vector2& _end)
{
    RegisterPoint(_start, _end, color_);
}

void LineDrawer::RegisterPoint(const Vector2& _start, const Vector2& _end, const Vector4& _color)
{
    assert(indexFor2d_ + 2 < kMaxNum && "The line instance is too large");

    vConstMapFor2D_[indexFor2d_].position = { _start.x,_start.y,0.0f, 1.0f };
    vConstMapFor2D_[indexFor2d_++].color = _color;

    vConstMapFor2D_[indexFor2d_].position = { _end.x, _end.y,0.0f,1.0f };
    vConstMapFor2D_[indexFor2d_++].color = _color;
}

void LineDrawer::Draw()
{

    if (indexFor3d_ == 0u && indexFor2d_ == 0u) return;

    TransferData();
    auto commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->SetGraphicsRootSignature(rootSignature_);
    commandList->SetPipelineState(graphicsPipelineState_);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    Draw3Dlines();
    Draw2Dlines();
}

void LineDrawer::Draw3Dlines()
{
    if (indexFor3d_ == 0u) return;
    assert(cameraFor3dptr_ != nullptr);

    auto commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->IASetVertexBuffers(0, 1, &vertexBufferViewFor3D_);
    commandList->SetGraphicsRootConstantBufferView(0, resourcesForMat3D_->GetGPUVirtualAddress());
    commandList->DrawInstanced(indexFor3d_, indexFor3d_ / 2, 0, 0);

    indexFor3d_ = 0u;
}

void LineDrawer::Draw2Dlines()
{
    if (indexFor2d_ == 0u) return;
    assert(cameraFor2dptr_ != nullptr);

    auto commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->IASetVertexBuffers(0, 1, &vertexBufferViewFor2D_);
    commandList->SetGraphicsRootConstantBufferView(0, resourceForMat2D_->GetGPUVirtualAddress());
    commandList->DrawInstanced(indexFor2d_, indexFor2d_ / 2, 0, 0);

    indexFor2d_ = 0u;
}

void LineDrawer::DrawOBB(const Matrix4x4& _affineMat)
{
    DrawOBB(_affineMat, color_);
}

void LineDrawer::DrawOBB(const Matrix4x4& _affineMat, const Vector4& _color)
{
    for (uint32_t index = 1; index < obbIndices_.size(); index += 2)
    {
        uint32_t sIndex = obbIndices_[index - 1];
        uint32_t eIndex = obbIndices_[index];
        Vector3 spos = Transform(obbVertices_[sIndex], _affineMat);
        Vector3 epos = Transform(obbVertices_[eIndex], _affineMat);
        RegisterPoint(spos, epos, _color);
    }
}

void LineDrawer::DrawOBB(const std::array<Vector3, 8>& _vertices)
{
    DrawOBB(_vertices, color_);
}

void LineDrawer::DrawOBB(const std::array<Vector3, 8>& _vertices, const Vector4& _color)
{
    for (uint32_t index = 1; index < obbIndices_.size(); index += 2)
    {
        uint32_t sIndex = obbIndices_[index - 1];
        uint32_t eIndex = obbIndices_[index];

        Vector3 spos = _vertices[sIndex];
        Vector3 epos = _vertices[eIndex];

        RegisterPoint(spos, epos, _color);
    }
}

void LineDrawer::DrawSphere(const Matrix4x4& _affineMat)
{
    DrawSphere(_affineMat, color_);
}

void LineDrawer::DrawSphere(const Matrix4x4& _affineMat, const Vector4& _color)
{
    for (uint32_t index = 1; index < sphereIndices_.size(); index += 2)
    {
        uint32_t sIndex = sphereIndices_[index - 1];
        uint32_t eIndex = sphereIndices_[index];

        Vector3 spos = Transform(sphereVertices_[sIndex], _affineMat);
        Vector3 epos = Transform(sphereVertices_[eIndex], _affineMat);

        RegisterPoint(spos, epos, _color);
    }
}

void LineDrawer::DrawCircle(const Vector3& _center, float _radius, const float _segmentCount, const Vector3& _normal)
{
    DrawCircle(_center, _radius, _segmentCount, _normal, color_);
}

void LineDrawer::DrawCircle(const Vector3& _center, float _radius, const float _segmentCount, const Vector3& _normal, const Vector4& _color)
{
    // 法線を正規化
    Vector3 normal = _normal.Normalize();

    // 法線ベクトルに垂直な2つのベクトルを見つける
    Vector3 tangent1, tangent2;

    // 法線ベクトルが上向きベクトル(0,1,0)と近い場合は別の基準ベクトルを使用
    if (std::abs(normal.Dot(Vector3(0, 1, 0))) > 0.99f)
    {
        // X軸を基準にする
        tangent1 = Vector3(0, 0, 1);
    }
    else
    {
        // 上向きベクトルと法線の外積で最初の接ベクトルを得る
        tangent1 = normal.Cross(Vector3(0, 1, 0)).Normalize();
    }

    // 法線と最初の接ベクトルの外積で2つ目の接ベクトルを得る
    tangent2 = normal.Cross(tangent1).Normalize();

    // この2つの接ベクトルを使って円周上の点を計算
    const float kEvery = std::numbers::pi_v<float> *2.0f / _segmentCount;

    for (uint32_t index = 0; index < static_cast<uint32_t>(_segmentCount); ++index)
    {
        float rad = index * kEvery;
        float nextRad = (index + 1) * kEvery;

        // 現在の円周上の点
        Vector3 spos = _center +
            tangent1 * std::cos(rad) * _radius +
            tangent2 * std::sin(rad) * _radius;

        // 次の円周上の点
        Vector3 epos = _center +
            tangent1 * std::cos(nextRad) * _radius +
            tangent2 * std::sin(nextRad) * _radius;

        // 線分を登録
        RegisterPoint(spos, epos);
    }
}


void LineDrawer::TransferData()
{
    matFor3dConstMap_->vp = cameraFor3dptr_->GetViewProjection();
    matFor2dConstMap_->vp = cameraFor3dptr_->GetViewProjection();
}

void LineDrawer::SetVerties()
{
    obbVertices_[0] = { 0.5f, 0.5f , 0.5f };
    obbVertices_[1] = { 0.5f, 0.5f ,-0.5f };
    obbVertices_[2] = { 0.5f,-0.5f , 0.5f };
    obbVertices_[3] = { 0.5f,-0.5f ,-0.5f };
    obbVertices_[4] = { -0.5f, 0.5f , 0.5f };
    obbVertices_[5] = { -0.5f, 0.5f ,-0.5f };
    obbVertices_[6] = { -0.5f,-0.5f , 0.5f };
    obbVertices_[7] = { -0.5f,-0.5f ,-0.5f };

    obbIndices_ = { 0,1,0,2,0,4,1,3,1,5,2,6,2,3,3,7,4,5,4,6,5,7,6,7 };


    //sphere頂点の計算
    const float kLatEvery = std::numbers::pi_v<float> / kDivision;  // 緯度方向の間隔 上下
    const float kLonEvery = std::numbers::pi_v<float> *2.0f / kDivision; // 経度方向の間隔 左右

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
