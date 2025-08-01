#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>

#include <Features/UVTransform/UVTransform.h>

#include <cstdint>

#include <d3d12.h>
#include <wrl.h>
#include <string>

struct aiMaterial;
class Material
{
public:


    void Initialize(const std::string& _texturepath);

    uint32_t GetTexturehandle() const { return textureHandle_; }
    ID3D12Resource* GetResource() { return resorces_.Get(); }

    UVTransform& GetUVTransform() { return uvTransform_; }

    void SetColor(const Vector4& _color) { deffuseColor_ = _color; }

    void SetShininess(float _shininess) { shiness_ = _shininess; }
    void SetEnableLighting(bool _enable) { enableLighting_ = _enable; }
    void SetEnableEnvironment(bool _enable) { enableEnvironment_ = _enable; }
    void SetEnvScale(float _scale) { envScale_ = _scale; }

    void TransferData();
    void MaterialQueueCommand(ID3D12GraphicsCommandList* _commandList, UINT _index);
    void TextureQueueCommand(ID3D12GraphicsCommandList* _commandList, UINT _index) const;
    void TextureQueueCommand(ID3D12GraphicsCommandList* _commandList, UINT _index, uint32_t _textureHandle) const;

    void AnalyzeMaterial(const aiMaterial* _material);

    void Imgui();
private:


private:

    UVTransform     uvTransform_ = {};
    Vector4 deffuseColor_ = { 1.0f, 1.0f, 1.0f , 1.0f }; // ディフューズカラー
    bool hasTexture_ = true;

    float shiness_ = 40.0f;                //
    bool enableLighting_ = true;                 // ライティングの有無
    float envScale_ = 0.0f;                 // 環境マッピングのスケール
    bool enableEnvironment_ = false; // 環境マッピングの有無

    std::string     name_                           = {};
    std::string     texturePath_ = {};
    uint32_t        textureHandle_ = 0;

    struct DataForGPU
    {
        Matrix4x4       uvTransform;

        Vector4         deffuseColor;

        float           shininess;
        int32_t         enabledLighthig;
        int32_t         hasTexture;
        float           envScale;

        int32_t         enableEnvironment = false; // 環境マッピングの有無
        float           padding[3];
    };

    Microsoft::WRL::ComPtr<ID3D12Resource>          resorces_ = nullptr;
    DataForGPU* constMap_ = nullptr;;

    void LoadTexture();

};
