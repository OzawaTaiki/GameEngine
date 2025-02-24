#include <Features/Model/Primitive/Cylinder.h>

#include <Features/LineDrawer/LineDrawer.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Core/DXCommon/DXCommon.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Features/Light/System/LightingSystem.h>


Cylinder::Cylinder(float _topRadius, float _bottomRadius, float _height, int32_t _divide, bool _top, bool _bottom):
    topRadius_(_topRadius),
    bottomRadius_(_bottomRadius),
    height_(_height),
    divide_(_divide),
    top_(_top),
    bottom_(_bottom),
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

void Cylinder::Generate()
{
    vertices_.clear();
    indices_.clear();

    if (divide_ < 3)
        divide_ = 3;


    // 頂点ごとの角度
    float kDivideAngle = std::numbers::pi_v<float> *2.0f / divide_;
    // 一頂点ごとのUV座標の割合
    // xは均等 yは3分割
    Vector2 kDivideUV = { 1.0f / (divide_), 1.0f / 3.0f };



    // 上面と下面の頂点計算
    for (int32_t index = 0; index <= divide_; ++index)
    {
        // 上面
        VertexData topVertex;

        // 下面
        VertexData bottomVertex;

        float angle = kDivideAngle * static_cast<float>(index);

        // 上面
        topVertex.position = Vector4(std::cosf(angle) * topRadius_, height_ / 2,std::sinf(angle) * topRadius_, 1.0f);
        topVertex.normal = topVertex.position.xyz().Normalize();
        topVertex.texcoord = { kDivideUV.x * static_cast<float>(index),kDivideUV.y};  // 円の中心が上

        // 下面
        bottomVertex.position = Vector4(std::cosf(angle) * bottomRadius_, -height_ / 2, std::sinf(angle) * bottomRadius_, 1.0f);
        bottomVertex.normal = bottomVertex.position.xyz().Normalize();
        bottomVertex.texcoord = { kDivideUV.x * static_cast<float>(index),kDivideUV.y * 2.0f };  // 円の中心が下

        vertices_.push_back(topVertex);
        vertices_.push_back(bottomVertex);

    }

    // 側面を三角に分割
    for (int32_t index = 0; index < divide_; ++index)
    {
        //  021 123 243 345 465...
        indices_.push_back((index * 2 + 2) );
        indices_.push_back((index * 2 + 1) );
        indices_.push_back((index * 2 + 0) );


        indices_.push_back((index * 2 + 3) );
        indices_.push_back((index * 2 + 1) );
        indices_.push_back((index * 2 + 2) );
    }

    // IBVの設定
    if (top_)
    {
        // 上面の中心
        VertexData topCenter;
        topCenter.position = Vector4(0, height_ / 2, 0, 1.0f);
        topCenter.normal = Vector3(0.0f, 1.0f, 0.0f);
        topCenter.texcoord = { 0.5f, 0.0f };
        vertices_.push_back(topCenter);

        for (int32_t index = 0; index < divide_; index++)
        {
            indices_.push_back(static_cast<uint32_t>(vertices_.size() - 1));
            indices_.push_back((index * 2 + 2));
            indices_.push_back(index * 2);
        }
    }
    if (bottom_)
    {
        // 下面の中心
        VertexData bottomCenter;
        bottomCenter.position = Vector4(0, -height_ / 2, 0, 1.0f);
        bottomCenter.normal = Vector3(0.0f, -1.0f, 0.0f);
        bottomCenter.texcoord = { 0.5f, 1.0f };
        vertices_.push_back(bottomCenter);

        for (int32_t index = 0; index < divide_; index++)
        {
            indices_.push_back(static_cast<uint32_t>(vertices_.size() - 1));
            indices_.push_back(index * 2 + 1);
            indices_.push_back((index * 2 + 3));
        }
    }

    mesh_.Initialize(vertices_, indices_);

}

void Cylinder::Update()
{
#ifdef _DEBUG
    ImGui::Begin("Cylinder");
    if (ImGui::DragFloat("topRadius", &topRadius_, 0.01f))
        Generate();
    if (ImGui::DragFloat("bottomRadius", &bottomRadius_, 0.01f))
        Generate();
    if (ImGui::DragFloat("height", &height_, 0.01f))
        Generate();
    if(ImGui::Checkbox("top", &top_))
        Generate();
    if (ImGui::Checkbox("bottom", &bottom_))
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

void Cylinder::Draw()
{

    for (uint32_t index = 0; index < indices_.size(); index += 3)
    {
        uint32_t index0 = indices_[index + 0];
        uint32_t index1 = indices_[index + 1];
        uint32_t index2 = indices_[index + 2];

        LineDrawer::GetInstance()->RegisterPoint(vertices_[index0].position.xyz(), vertices_[index1].position.xyz());
        LineDrawer::GetInstance()->RegisterPoint(vertices_[index1].position.xyz(), vertices_[index2].position.xyz());
        LineDrawer::GetInstance()->RegisterPoint(vertices_[index2].position.xyz(), vertices_[index0].position.xyz());
    }
}

void Cylinder::Draw(const Camera& _camera, const Vector4& _color)
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
