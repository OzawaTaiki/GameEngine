#pragma once
#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>
#include <Features/Model/Color/ObjectColor.h>

#include <string>
#include <memory>
#include <d3d12.h>
#include <wrl.h>
class Sprite
{
public:

    Sprite(const std::string& _name );
    ~Sprite() ;

    void Initialize();
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

    void SetTextureHandle(uint32_t _textureHandle) { textureHandle_ = _textureHandle; }
    void SetAnchor(const Vector2& _anchor) { anchor_ = _anchor; CalculateVertex(); }

    void SetSize(const Vector2& _size);
    void SetColor(const Vector4& _color);

    void SetUVSize(const Vector2& _size);
    void SetLeftTop(const Vector2& _leftTop);

private:

    static uint32_t winWidth_;
    static uint32_t winHeight_;

    void TransferData(ID3D12GraphicsCommandList *_commandList );

    void CalculateVertex();
    void CalculateMatrix();

    uint32_t textureHandle_ = 0;

    Vector2 defaultTextureSize_ = { 100.0f,100.0f };
    Vector2 anchor_ = { 0.5f, 0.5f };
    Vector4 color_ = {};
    Matrix4x4 worldMat_ = {};
    Matrix4x4 uvTransMat_ = {};
    Matrix4x4 orthoMat_= {};

    struct ConstantBufferData
    {
        Matrix4x4 worldMat;
        Matrix4x4 uvTransMat;
    };

    std::unique_ptr<ObjectColor> colorObj_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> matResource_ = nullptr;
    ConstantBufferData* constMap_ = nullptr;


    struct VertexData
    {
        Vector4 position;
        Vector2 texcoord;
    };

    VertexData* vConstMap_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>      vertexResource_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW                    vertexBufferView_ = {};

    std::string name_ = "";

    Vector2 lefttop_ = { 0.0f,0.0f };
    Vector2 size_ = { 100.0f,100.0f };
    void ImGui();


};
