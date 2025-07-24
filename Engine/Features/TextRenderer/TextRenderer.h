#pragma once

#include <Externals/stb/stb_truetype.h>

#include <Features/TextRenderer/AtlasData.h>
#include <Features/TextRenderer/TextParam.h>

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>

#include <d3d12.h>
#include <wrl.h>

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <fstream>
#include <cstdint>

#ifdef DrawText
#undef DrawText
#endif // DrawText

class TextRenderer
{
public:
    static TextRenderer* GetInstance();

public:


    void Initialize(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, const Vector2& _windowSize = { 1280,720 });
    void Finalize();

    void BeginFrame();

    void EndFrame();

    void DrawText(const std::wstring& _text, AtlasData* _atlas, const Vector2& _pos, const Vector4& _color = { 1,1,1,1 });

    void DrawText(const std::wstring& _text, AtlasData* _atlas, const TextParam& _param);



private:

    struct TextVertex
    {
        Vector4 position = { 0,0,0,1 }; // 頂点の位置
        Vector2 texCoord = { 0,0 }; // テクスチャ座標
        Vector4 color = { 1,1,1,1 }; // 頂点の色
    };

    struct ResourceDataGroup
    {
        // 描画用
        std::vector<TextVertex> vertices_;

        D3D12_VERTEX_BUFFER_VIEW vbv_; // 頂点バッファビュー
        Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_; // 頂点バッファ

        std::vector<Matrix4x4> affineMatrices_; // アフィン変換行列

        Microsoft::WRL::ComPtr<ID3D12Resource> matrixBuffer_; // アフィン変換行列のバッファ
        uint32_t affrinMatSRVIndex_ = UINT32_MAX; // アフィン変換行列のSRVインデックス

        uint32_t textureIndex_ = UINT32_MAX; // テクスチャのSRVインデックス

        AtlasData* atlasData_ = nullptr; // アトラスデータ
    };

private:
    void DrawText(const std::wstring& _text, const Vector2& _pos, const Vector2& _scale, float _rotate, const Vector2& _piv, const Vector4& _topColor, const Vector4& _bottomColor, ResourceDataGroup* _res);
    void DrawTextWithOutline(
        const std::wstring& _text,
        const Vector2& _pos,
        const Vector2& _scale,
        float _rotate,
        const Vector2& _piv,
        const Vector4& _topColor,
        const Vector4& _bottomColor,
        const Vector4& _outlineColor,
        float _outlineThickness,
        ResourceDataGroup* _res);

    // アトラスデータを設定
    ResourceDataGroup* EnsureAtlasResources(AtlasData* _atlas);

    void CreateVertexBuffer(ResourceDataGroup* _res);
    void CreateMatrixBuffer(ResourceDataGroup* _res);
    void CreateProjectionMatrixBuffer();


    void UploadVertexData(ResourceDataGroup* _res);
    void UploadMatrixData(ResourceDataGroup* _res);
    void RenderText(ResourceDataGroup* _res);

private:



    ID3D12PipelineState* pso_; // パイプラインステートオブジェクト
    ID3D12RootSignature* rootSignature_; // ルートシグネチャ

    ID3D12Device* device_; // D3D12デバイス
    ID3D12GraphicsCommandList* cmdList_; // コマンドリスト

    size_t maxVertices_;
    size_t maxCharacters_ = 1500; // 最大文字数

    Matrix4x4* projectionMatrix_; // 投影行列
    Microsoft::WRL::ComPtr<ID3D12Resource> projectionMatrixBuffer_; // 投影行列のバッファ

    std::map<uint32_t, std::unique_ptr<ResourceDataGroup>> resourceDataGroups_; // リソースデータグループ

    Vector2 windowSize_;

private:

    // singleton
    TextRenderer() = default;
    ~TextRenderer() = default;

    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;
    TextRenderer(TextRenderer&&) = delete;
    TextRenderer& operator=(TextRenderer&&) = delete;

};