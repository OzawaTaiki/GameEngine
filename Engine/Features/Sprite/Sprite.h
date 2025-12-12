#pragma once
#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>

#include <Framework/Batch2DRenderer.h>

#include <string>
#include <memory>
#include <d3d12.h>
#include <wrl.h>
#include <Features/UVTransform/UVTransform.h>
class Sprite
{
public:

    Sprite(const std::string& _name, bool _debugWinddow = false);
    ~Sprite();

    void Initialize();
    void Initialize(uint32_t _textureHandle);
    void Update();
    void Draw() const;
    void Draw(const Vector4& _color);

    // worldMat
    Vector2 translate_ = {0.0f,0.0f };
    Vector2 scale_={1.0f,1.0f };
    float rotate_ = 0.0f;


    ////uvTrans
    //Vector2 uvTranslate_ = { 0.0f,0.0f };
    //Vector2 uvScale_ = { 1.0f,1.0f };
    //float uvRotate_ = 0.0f;

    UVTransform& GetUVTransform() { return uvTransform_; }

    static std::unique_ptr<Sprite> Create(const std::string& _name, uint32_t _textureHandle, const Vector2& _anchor = { 0.5f, 0.5f });
    static void StaticInitialize(uint32_t _windowWidth, uint32_t _windowWHeight);
    static void PreDraw();

    void SetTextureHandle(uint32_t _textureHandle);
    void SetAnchor(const Vector2& _anchor);

    void SetSize(const Vector2& _size);
    Vector2 GetSize() const { return scale_ * defaultTextureSize_; }
    void SetColor(const Vector4& _color);

    void SetUVSize(const Vector2& _size);
    void SetLeftTop(const Vector2& _leftTop);

    // スプライト矩形とマウスポインタとの衝突判定
    bool IsHitTest(const Vector2& _mousePos) const;

    void SetOrder(int16_t _order) { order_ = _order; }
    uint16_t GetOrder() const { return order_; }

    void ImGui();
private:

    static uint32_t winWidth_;
    static uint32_t winHeight_;

    void UpdateVertexData();
    void UpdateInstanceData();

    UVTransform uvTransform_;

    uint32_t textureHandle_ = UINT32_MAX;

    Vector2 defaultTextureSize_ = { 100.0f,100.0f };
    Vector2 anchor_ = { 0.5f, 0.5f };
    Vector4 color_ = {};

    std::string name_ = "";

    Vector2 lefttop_ = { 0.0f,0.0f };
    Vector2 size_ = { 100.0f,100.0f };


    std::vector<Batch2DRenderer::VertexData> vertexData_= {};
    Batch2DRenderer::InstanceData instanceData_ = {};
    int16_t order_ = 0;

    // 頂点データを計算する必要があるか
    bool isVertexDirty_ = true;
    // instanceDataを計算する必要があるか
    bool isDirty_ = true;

};
