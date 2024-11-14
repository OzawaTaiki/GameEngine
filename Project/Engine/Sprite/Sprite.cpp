#include "Sprite.h"

#include "TextureManager.h"
#include "SpriteManager.h"
#include "DXCommon.h"
#include "WinApp.h"
#include "MatrixFunction.h"

uint32_t Sprite::winWidth_ = 1280;
uint32_t Sprite::winHeight_ = 720;

void Sprite::Initialize()
{
    color_ = { 1,1,1,1 };
    colorObj_ = std::make_unique<ObjectColor>();
    colorObj_->Initialize();
    colorObj_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

    matResource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(ConstantBufferData));
    matResource_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));

    constMap_->worldMat = MakeIdentity4x4();
    constMap_->uvTransMat = MakeIdentity4x4();


    vertexResource_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(VertexData)*6);
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vConstMap_));

    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(VertexData) * 6;
    vertexBufferView_.StrideInBytes = sizeof(VertexData);


    vConstMap_[0].texcoord = {0.0f,1.0f };
    vConstMap_[1].texcoord = {0.0f,0.0f };
    vConstMap_[2].texcoord = {1.0f,1.0f };
    vConstMap_[3].texcoord = vConstMap_[1].texcoord;
    vConstMap_[4].texcoord = {1.0f,0.0f };
    vConstMap_[5].texcoord = vConstMap_[2].texcoord;

    textureSize_ = TextureManager::GetInstance()->GetTextureSize(textureHandle_);
    anchor_ = { 0,0 };
    CalculateVertex();
}

void Sprite::Update()
{

#ifdef _DEBUG
    ImGui();
#endif // _DEBUG
}

void Sprite::Draw()
{
    auto commandList = DXCommon::GetInstance()->GetCommandList();
    TransferData(commandList);
    colorObj_->SetColor(color_);

    commandList->DrawInstanced(6, 1, 0, 0);

}

void Sprite::Draw(const Vector4& _color)
{
    auto commandList = DXCommon::GetInstance()->GetCommandList();
    TransferData(commandList);
    colorObj_->SetColor(_color);

    commandList->DrawInstanced(6, 1, 0, 0);
}

Sprite* Sprite::Create(uint32_t _textureHandle, const Vector2& _anchor)
{
    Sprite* sprite = SpriteManager::GetInstance()->Create();
    sprite->textureHandle_ = _textureHandle;
    sprite->anchor_ = _anchor;

    return sprite;
}

void Sprite::StaticInitialize(uint32_t _windowWidth, uint32_t _windowWHeight)
{
    SpriteManager::GetInstance()->Initialize();
    winWidth_ = winWidth_;
    winHeight_ = _windowWHeight;
}

void Sprite::PreDraw()
{
    SpriteManager::GetInstance()->PreDraw();
}

void Sprite::SetSize(const Vector2& _size)
{
    uvScale_ = _size / textureSize_;
}

void Sprite::SetLeftTop(const Vector2& _leftTop)
{
    uvTranslate_ = _leftTop / textureSize_;
}

void Sprite::TransferData(ID3D12GraphicsCommandList* _commandList)
{
    CalculateMatrix();
    _commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);

    _commandList->SetGraphicsRootConstantBufferView(0, matResource_->GetGPUVirtualAddress());
    colorObj_->QueueCommand(_commandList,1);
    _commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetGPUHandle(textureHandle_));
}

void Sprite::CalculateVertex()
{
    Vector2 size = textureSize_;

    vConstMap_[0].position = {
        -anchor_.x * size.x ,
        (1.0f - anchor_.y) * size.y ,0.0f,1.0f }; // 左下

    vConstMap_[1].position = {
        -anchor_.x * size.x,
        -anchor_.y * size.y,0.0f,1.0f }; // 左上

    vConstMap_[2].position = {
        (1.0f - anchor_.x) * size.x,
        (1.0f - anchor_.y) * size.y,0.0f,1.0f }; // 右下

    vConstMap_[4].position = {
        (1.0f - anchor_.x) * size.x ,
        -anchor_.y * size.y ,0.0f,1.0f }; // 右上


    vConstMap_[3].position = vConstMap_[1].position;
    vConstMap_[5].position = vConstMap_[2].position;

}

void Sprite::CalculateMatrix()
{
    Vector3 s = { scale_,1.0f };
    Vector3 r = { 0.0f,0.0f ,rotate_ };
    Vector3 t = { translate_,0.0f };
    constMap_->worldMat = MakeAffineMatrix(s, r, t);

    Matrix4x4 vp = Inverse(MakeIdentity4x4()) * MakeOrthographicMatrix(0, 0, static_cast<float> (winWidth_), static_cast<float>(winHeight_), -1.0f, 1.0);
    constMap_->worldMat *= vp;


    s = { uvScale_,1.0f };
    r = { 0.0f,0.0f ,uvRotate_ };
    t = { uvTranslate_,0.0f };
    constMap_->uvTransMat = MakeAffineMatrix(s, r, t);
}
#ifdef _DEBUG
#include <imgui.h>
void Sprite::ImGui()
{
    ImGui::Begin("Sprite");
    ImGui::DragFloat2("Translate", &translate_.x);
    ImGui::DragFloat2("Scale", &scale_.x, 0.01f);
    ImGui::DragFloat("Rotate", &rotate_, 0.01f);
    ImGui::DragFloat2("UVTranslate", &uvTranslate_.x, 0.01f);
    ImGui::DragFloat2("UVScale", &uvScale_.x, 0.01f);
    ImGui::DragFloat("UVRotate", &uvRotate_, 0.01f);
    ImGui::ColorEdit4("Color", &color_.x);
    if(ImGui::DragFloat2("Anchor", &anchor_.x, 0.01f))
    {
        CalculateVertex();
    }

    ImGui::DragFloat2("Size", &size_.x, 0.01f);
    ImGui::DragFloat2("LeftTop", &lefttop_.x, 0.01f);
    if (ImGui::Button("Set"))
    {
        SetSize(size_);
        SetLeftTop(lefttop_);
    }

    ImGui::End();
}
#endif // _DEBUG
