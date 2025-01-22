#include <Features/Model/Primitive/Ellipse.h>

#include <Features/LineDrawer/LineDrawer.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Core/DXCommon/DXCommon.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Features/Light/System/LightingSystem.h>

EllipseModel::EllipseModel(const Vector3& _radius, uint32_t _divide) :
    radius_(_radius),
    divide_(_divide),
    scale_({ 1,1,1 }),
    rotation_({ 0,0,0 }),
    quternion_({ 0,0,0,1 }),
    translate_({ 0,0,0 }),
    textureHandle_(2)
{
    objectColor_.Initialize();
    material_.Initialize("");
    worldTransform_.Initialize();
}

EllipseModel::EllipseModel(float _radius, uint32_t _divide) :
    radius_({ _radius,_radius,_radius }),
    divide_(_divide),
    scale_({ 1,1,1 }),
    rotation_({ 0,0,0 }),
    quternion_({ 0,0,0,1 }),
    translate_({ 0,0,0 }),
    textureHandle_(2)
{
    objectColor_.Initialize();
    material_.Initialize("");
    worldTransform_.Initialize();
}

void EllipseModel::Generate()
{
    vertices_.clear();
    indices_.clear();

    if (divide_ < 3)
        divide_ = 3;

    // 頂点ごとの角度（緯度と経度の分割）
    float kLatEvery = std::numbers::pi_v<float> / divide_;
    float kLonEvery = std::numbers::pi_v<float> *2.0f / divide_;
    // UV座標の割合
    float kDivideUV = 1.0f / divide_;

    // 頂点の計算
    for (int32_t lat = 0; lat <= divide_; ++lat)
    {
        float theta = kLatEvery * static_cast<float>(lat); // 緯度の角度
        float sinTheta = std::sinf(theta);
        float cosTheta = std::cosf(theta);

        for (int32_t lon = 0; lon <= divide_; ++lon)
        {
            float phi = kLonEvery * static_cast<float>(lon); // 経度の角度
            float sinPhi = std::sinf(phi);
            float cosPhi = std::cosf(phi);

            Mesh::VertexData vertex;

            // 楕円体の頂点位置を計算
            vertex.position = Vector4(
                radius_.x * sinTheta * cosPhi,  // x軸半径
                radius_.y * sinTheta * sinPhi,  // y軸半径
                radius_.z * cosTheta,           // z軸半径
                1.0f
            );

            // 法線の計算（楕円体スケールを考慮して正規化）
            Vector3 unnormalizedNormal(
                sinTheta * cosPhi / radius_.x,  // x方向のスケールを考慮
                sinTheta * sinPhi / radius_.y,  // y方向のスケールを考慮
                cosTheta / radius_.z            // z方向のスケールを考慮
            );
            vertex.normal = unnormalizedNormal.Normalize(); // 正規化

            // UV座標の設定
            vertex.texcoord = { kDivideUV * static_cast<float>(lon), kDivideUV * static_cast<float>(lat) };

            vertices_.push_back(vertex);
        }
    }

    // インデックスの計算
    for (int32_t lat = 0; lat < divide_; ++lat)
    {
        for (int32_t lon = 0; lon < divide_; ++lon)
        {
            int32_t current = lat * (divide_ + 1) + lon;
            int32_t next = current + divide_ + 1;

            // 三角形1
            indices_.push_back(current);
            indices_.push_back(next);
            indices_.push_back(current + 1);

            // 三角形2
            indices_.push_back(current + 1);
            indices_.push_back(next);
            indices_.push_back(next + 1);
        }
    }

    // メッシュの初期化
    mesh_.Initialize(vertices_, indices_);
}


void EllipseModel::Update()
{
#ifdef _DEBUG

    ImGui::Begin("Ellipse");
    if (ImGui::DragFloat3("radius", &radius_.x, 0.01f))
        Generate();
    if (ImGui::DragInt("divide", reinterpret_cast<int*>(&divide_)))
        Generate();
    ImGui::DragFloat3("scale", &scale_.x, 0.01f);
    ImGui::DragFloat3("rotation", &rotation_.x, 0.01f);
    ImGui::DragFloat3("translate", &translate_.x, 0.01f);
    ImGui::Checkbox("useQuaternion", &useQuaternion_);
    ImGui::End();

#endif

    worldTransform_.transform_ = translate_;
    worldTransform_.scale_ = scale_;
    if (useQuaternion_)
        worldTransform_.quaternion_ = quternion_;
    else
        worldTransform_.rotate_ = rotation_;
    worldTransform_.UpdateData(useQuaternion_);

}

void EllipseModel::Draw()
{
    for (int32_t index = 0; index < indices_.size(); index += 3)
    {
        uint32_t index0 = indices_[index + 0];
        uint32_t index1 = indices_[index + 1];
        uint32_t index2 = indices_[index + 2];
        Vector3 v0 = vertices_[index0].position.xyz();
        Vector3 v1 = vertices_[index1].position.xyz();
        Vector3 v2 = vertices_[index2].position.xyz();
        LineDrawer::GetInstance()->RegisterPoint(v0, v1);
        LineDrawer::GetInstance()->RegisterPoint(v1, v2);
        LineDrawer::GetInstance()->RegisterPoint(v2, v0);
    }
}

void EllipseModel::Draw(const Camera& _camera, const Vector4& _color)
{
    auto commandList = DXCommon::GetInstance()->GetCommandList();
    ModelManager::GetInstance()->PreDrawForObjectModel();

    mesh_.QueueCommand(commandList);
    _camera.QueueCommand(commandList, 0);
    worldTransform_.QueueCommand(commandList, 1);
    material_.MaterialQueueCommand(commandList, 2);
    objectColor_.SetColor(_color);
    objectColor_.QueueCommand(commandList, 3);
    commandList->SetGraphicsRootDescriptorTable(4, TextureManager::GetInstance()->GetGPUHandle(textureHandle_));
    LightingSystem::GetInstance()->QueueCommand(commandList, 5);

    commandList->DrawIndexedInstanced(mesh_.GetIndexNum(), 1, 0, 0, 0);
}

void EllipseModel::SetTexture(const std::string& _path)
{
    textureHandle_ = TextureManager::GetInstance()->Load(_path);
}
