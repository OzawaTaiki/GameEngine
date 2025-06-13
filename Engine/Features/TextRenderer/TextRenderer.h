#pragma once

#include <Externals/stb/stb_truetype.h>

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector4.h>

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

    struct Config
    {
        Vector2 atlasSize = { 4096,4096 }; // アトラスの幅
        float fontSize = 32.0f; // フォントサイズ
        std::string fontFilePath = "Resources/Fonts/NotoSansJP-Regular.ttf"; // フォントファイルのパス
    };

private:

    struct GlyphInfo
    {
        float u0, v0, u1, v1;      // UV座標
        float width, height;        // サイズ
        float bearingX, bearingY;   // オフセット
        float advance;              // 次の文字までの距離
        bool isValid = false;
    };
public:


    void Initialize(ID3D12Device* _device,ID3D12GraphicsCommandList* _cmdList, const Config& _config, const Vector2& _windowSize = { 1280,720 });
    void Finalize();

    void BeginFrame();
    void EndFrame();


    void DrawText(const std::wstring& _text, const Vector2& _pos, const Vector4& _color = { 1,1,1,1 });


private:

    void LoadFont(const std::string& _fontFilePath,float _fontSize);
    void CreateFontTexture();
    void CreateVertexBuffer();

    void UpdateFontTexture();
    void UploadVertexData();
    void RenderText();


    void PreloadCommonCharacters();


    GlyphInfo GetGlyph(wchar_t _character);
    GlyphInfo GenerateGlyph(wchar_t _character);

private:


    struct TextVertex
    {
        Vector4 position = {0,0,0,1}; // 頂点の位置
        Vector2 texCoord = { 0,0 }; // テクスチャ座標
        Vector4 color = { 1,1,1,1 }; // 頂点の色
    };

    ID3D12PipelineState* pso_; // パイプラインステートオブジェクト
    ID3D12RootSignature* rootSignature_; // ルートシグネチャ

    D3D12_VERTEX_BUFFER_VIEW vbv_; // 頂点バッファビュー

    ID3D12Device* device_; // D3D12デバイス
    ID3D12GraphicsCommandList* cmdList_; // コマンドリスト
    Microsoft::WRL::ComPtr<ID3D12Resource> fontTexture_; // フォントテクスチャ

    uint32_t textureIndex_; // テクスチャのインデックス
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_; // 頂点バッファ

    Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer_;

    stbtt_fontinfo fontInfo_; // フォント情報
    std::vector<uint8_t> fontBuffer_; // フォントデータを格納するバッファ
    float scale_;
    float fontSize_;

    // テクスチャアトラスの情報
    std::vector<uint8_t> atlasData_; // テクスチャアトラスのバッファ
    Vector2 atlasSize_; // アトラスのサイズ
    int32_t currentX_; // 現在のX位置
    int32_t currentY_; // 現在のY位置
    int32_t maxY_; // アトラスの最大Y位置
    std::unordered_map<wchar_t, GlyphInfo> glyphs_; // グリフ情報を格納するマップ
    bool atlasNeedsUpdate_; // アトラスの更新が必要かどうか

    float fontAscent_;
    float fontDescent_;
    float fontLineGap_;

    // 描画用
    std::vector<TextVertex> vertices_;
    size_t maxVertices_;


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