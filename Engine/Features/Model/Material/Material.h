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

namespace Engine {

class Material
{
public:
    Material(const std::string& _name = "") : name_(_name) {}
    ~Material() = default;

    // コピーコンストラクタ（ディープコピー）
    Material(const Material& _other);

    void Initialize(const std::string& _texturepath);

    uint32_t GetTexturehandle() const { return textureHandle_; }
    ID3D12Resource* GetResource() { return resorces_.Get(); }

    UVTransform& GetUVTransform() { return uvTransform_; }

    void SetColor(const Vector4& _color) { deffuseColor_ = _color; }

    void SetShininess(float _shininess) { shiness_ = _shininess; }
    void SetShadingStrength(float _shadingStrength) { shadingStrength_ = _shadingStrength; }
    void SetShadeColor(const Vector3& _shadeColor) { shadeColor_ = _shadeColor; }
    void SetSpecularStrength(float _specularStrength) { specularStrength_ = _specularStrength; }
    void SetEnableLighting(bool _enable) { enableLighting_ = _enable; }
    void SetEnableEnvironment(bool _enable) { enableEnvironment_ = _enable; }
    void SetEnvScale(float _scale) { envScale_ = _scale; }

    void SetName(const std::string& _name) { name_ = _name; }
    const std::string& GetName() const { return name_; }

    void TransferData();
    void MaterialQueueCommand(ID3D12GraphicsCommandList* _commandList, UINT _index);
    void TextureQueueCommand(ID3D12GraphicsCommandList* _commandList, UINT _index) const;
    void TextureQueueCommand(ID3D12GraphicsCommandList* _commandList, UINT _index, uint32_t _textureHandle) const;

    void AnalyzeMaterial(const aiMaterial* _material);

    /// <summary>
    /// マテリアル設定をJSONファイルへ保存する。
    /// 拡張子が省略された場合は .json を付与する。
    /// </summary>
    bool SaveToFile(const std::string& _filePath) const;

    /// <summary>
    /// JSONファイルからマテリアル設定を読み込む。
    /// </summary>
    bool LoadFromFile(const std::string& _filePath);

    void SetMaterialFilePath(const std::string& _filePath) { materialFilePath_ = _filePath; }
    const std::string& GetMaterialFilePath() const { return materialFilePath_; }

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
    float shadingStrength_ = 1.0f;
    Vector3 shadeColor_ = { 0.0f, 0.0f, 0.0f };
    float specularStrength_ = 0.3f;

    std::string     name_                           = {};
    std::string     texturePath_ = {};
    std::string     materialFilePath_ = {};
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
        float           specularStrength; // 鏡面反射度
        float           shadingStrength; // ライティングによる陰影の強さ
        float           padding;

        Vector3         shadeColor; // ライティングによる陰影の色
        float           shadeColorPadding;
    };

    Microsoft::WRL::ComPtr<ID3D12Resource>          resorces_ = nullptr;
    DataForGPU* constMap_ = nullptr;;

    void LoadTexture();

};

} // namespace Engine
