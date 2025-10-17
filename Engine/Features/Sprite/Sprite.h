#pragma once
#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>

#include <Framework/Batch2DRenderer.h>

#include <string>
#include <memory>
#include <d3d12.h>
#include <wrl.h>
class Sprite
{
public:

    Sprite(const std::string& _name);
    ~Sprite() ;

    void Initialize();
    void Initialize(uint32_t _textureHandle);
    void Update();
    void Draw();
    void Draw(const Vector4& _color);

    // worldMat
    Vector2 translate_ = {0.0f,0.0f };
    Vector2 scale_={1.0f,1.0f };
    float rotate_ = 0.0f;


    //uvTrans
    Vector2 uvTranslate_ = { 0.0f,0.0f };
    Vector2 uvScale_ = { 1.0f,1.0f };
    float uvRotate_ = 0.0f;

    static std::unique_ptr<Sprite> Create(const std::string& _name, uint32_t _textureHandle, const Vector2& _anchor = { 0.5f, 0.5f });
    static void StaticInitialize(uint32_t _windowWidth, uint32_t _windowWHeight);
    static void PreDraw();

    void SetTextureHandle(uint32_t _textureHandle);
    void SetAnchor(const Vector2& _anchor);

    void SetSize(const Vector2& _size);
    void SetColor(const Vector4& _color);

    void SetUVSize(const Vector2& _size);
    void SetLeftTop(const Vector2& _leftTop);

    void ImGui();
private:

    static uint32_t winWidth_;
    static uint32_t winHeight_;

    void UpdateVertexData();
    void UpdateInstanceData();

    uint32_t textureHandle_ = UINT32_MAX;

    Vector2 defaultTextureSize_ = { 100.0f,100.0f };
    Vector2 anchor_ = { 0.5f, 0.5f };
    Vector4 color_ = {};

    std::string name_ = "";

    Vector2 lefttop_ = { 0.0f,0.0f };
    Vector2 size_ = { 100.0f,100.0f };


    std::vector<Batch2DRenderer::VertexData> vertexData_= {};
    Batch2DRenderer::InstanceData instanceData_ = {};

    // 頂点データを計算する必要があるか
    bool isVertexDirty_ = true;
    // instanceDataを計算する必要があるか
    bool isDirty_ = true;

};
