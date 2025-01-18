#include "Ring.h"
#include <Rendering/LineDrawer/LineDrawer.h>
#include <ResourceManagement/TextureManager/TextureManager.h>
#include <Core/DirectX/DXCommon.h>
#include <Rendering/Model/ModelManager.h>
#include <Rendering/Light/LightingSystem.h>

#include <numbers>

Ring::Ring(float _innerRadius, float _outerRadius, uint32_t _divide, std::array<bool, 3> _billboard) :
    innerRadius_(_innerRadius),
    outerRadius_(_outerRadius),
    divide_(_divide),
    billboard_(_billboard),
    scale_({ 1,1,1 }),
    rotation_({ 0,0,0 }),
    quternion_({ 0,0,0,1 }),
    translate_({ 0,0,0 }),
    textureHandle_(2)
{
    objectColor_.Initialize();
    material_.Initialize("");
    worldTransform.Initialize();
}

void Ring::Generate()
{
    vertices_.clear();
    indices_.clear();

    if (divide_ < 3)
        divide_ = 3;

    // 頂点ごとの角度
    float kDivideAngle = std::numbers::pi_v<float> *2.0f / divide_;
    // 一頂点ごとのUV座標の割合
    float kDivideUV = 1.0f / divide_;

    // 頂点の計算　頂点の数 ＝ 分割数
    for (int32_t index = 0; index <= divide_; ++index)
    {
        // 外側の頂点
        Mesh::VertexData outerVertex;
        // 内側の頂点
        Mesh::VertexData innerVertex;

        float angle = kDivideAngle * static_cast<float>(index);

        outerVertex.position = Vector4(std::cosf(angle) * outerRadius_, std::sinf(angle) * outerRadius_, 0.0f,1.0f);
        outerVertex.normal = Vector3(0.0f, 0.0f, 1.0f);
        outerVertex.texcoord = { kDivideUV * static_cast<float>(index),1.0f };


        innerVertex.position = Vector4(std::cosf(angle) * innerRadius_, std::sinf(angle) * innerRadius_, 0.0f, 1.0f);
        innerVertex.normal = Vector3(0.0f, 0.0f, 1.0f);
        innerVertex.texcoord = { kDivideUV * static_cast<float>(index),0.0f };

        vertices_.push_back(outerVertex);
        vertices_.push_back(innerVertex);
    }

    // インデックスの計算
    for (int32_t index = 0; index < divide_ * 2; index += 2)
    {
        //  021 123 243 345 465...
        indices_.push_back((index + 2));
        indices_.push_back((index + 0));
        indices_.push_back((index + 1));

        indices_.push_back((index + 3));
        indices_.push_back((index + 2));
        indices_.push_back((index + 1));
    }

    mesh_.Initialize(vertices_, indices_);

}

void Ring::Update()
{
#ifdef _DEBUG

    ImGui::Begin("Ring");
    if(ImGui::DragFloat("innerRadius", &innerRadius_,0.01f))
        Generate();
    if(ImGui::DragFloat("outerRadius", &outerRadius_, 0.01f))
        Generate();
    if (ImGui::DragInt("divide", reinterpret_cast<int*>(&divide_)))
        Generate();
    ImGui::Checkbox("billboardX", &billboard_[0]);
    ImGui::Checkbox("billboardY", &billboard_[1]);
    ImGui::Checkbox("billboardZ", &billboard_[2]);
    ImGui::DragFloat3("scale", &scale_.x, 0.01f);
    ImGui::DragFloat3("rotation", &rotation_.x, 0.01f);
    ImGui::DragFloat3("translate", &translate_.x, 0.01f);
    ImGui::Checkbox("useQuaternion", &useQuaternion_);
    ImGui::End();


#endif

    worldTransform.transform_ = translate_;
    worldTransform.scale_ = scale_;
    if (useQuaternion_)
        worldTransform.quaternion_ = quternion_;
    else
        worldTransform.rotate_ = rotation_;

    worldTransform.UpdateData(useQuaternion_);
}

void Ring::Draw()
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

void Ring::Draw(const Camera& _camera, const Vector4& _color)
{
    ID3D12GraphicsCommandList* commandList = DXCommon::GetInstance()->GetCommandList();
    ModelManager::GetInstance()->PreDrawForObjectModel();

    mesh_.QueueCommand(commandList);
    _camera.QueueCommand(commandList, 0);//
    worldTransform.QueueCommand(commandList, 1);//
    material_.MaterialQueueCommand(commandList, 2);//
    objectColor_.SetColor(_color);
    objectColor_.QueueCommand(commandList, 3);//
    commandList->SetGraphicsRootDescriptorTable(4, TextureManager::GetInstance()->GetGPUHandle(textureHandle_));
    LightingSystem::GetInstance()->QueueCommand(commandList,5);//

    commandList->DrawIndexedInstanced(mesh_.GetIndexNum(), 1, 0, 0, 0);

}

void Ring::SetTexture(const std::string& _path)
{
    textureHandle_ = TextureManager::GetInstance()->Load(_path);
}
