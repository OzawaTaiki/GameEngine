#pragma once

#include <Externals/stb/stb_truetype.h>

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

struct TextParam
{
    Vector2 scale = { 1.0f, 1.0f }; // スケール
    float rotate = 0.0f; // 回転角度
    Vector2 position; // 描画位置

    bool useGradient = false; // グラデーションを使用するかどうか
    Vector4 topColor = { 1, 1, 1, 1 }; // 上の色(グラデーション用)
    Vector4 bottomColor = { 0, 0, 0, 1 }; // 下の色(グラデーション用)

    Vector2 pivot = { 0.5f, 0.5f }; // ピボット位置

    bool  useOutline = false; // アウトラインを使用するかどうか
    Vector4 outlineColor = { 0, 0, 0, 1 }; // アウトラインの色
    float outlineScale = 0.03f; // アウトラインの太さ

    TextParam& SetScale(const Vector2& _scale) { scale = _scale; return *this; }
    TextParam& SetRotate(float _rotate) { rotate = _rotate; return *this; }
    TextParam& SetPosition(const Vector2& _pos) { position = _pos; return *this; }
    TextParam& SetColor(const Vector4& _color) { topColor = _color; bottomColor = _color; useGradient = false; return *this; }
    TextParam& SetGradientColor(const Vector4& _topColor, const Vector4& _bottomColor) { topColor = _topColor; bottomColor = _bottomColor; useGradient = true; return *this; }
    TextParam& SetPivot(const Vector2& _piv) { pivot = _piv; return *this; }
    TextParam& SetOutline(const Vector4& _outlineColor = { 0, 0, 0, 1 }, float _outlineScale = 0.03f)
    {
        useOutline = true;
        outlineColor = _outlineColor;
        outlineScale = _outlineScale;
        return *this;
    }
};

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

    void DrawText(const std::wstring& _text, const TextParam& _param);



private:
    void DrawText(const std::wstring& _text, const Vector2& _pos, const Vector2& _scale, float _rotate, const Vector2& _piv, const Vector4& _topColor, const Vector4& _bottomColor);
    void DrawTextWithOutline(
        const std::wstring& _text,
        const Vector2& _pos,
        const Vector2& _scale,
        float _rotate,
        const Vector2& _piv,
        const Vector4& _topColor,
        const Vector4& _bottomColor,
        const Vector4& _outlineColor,
        float _outlineThickness);

    // 中心座標を求める ヘルパー関数
    Vector2 GetCenterPosition(const std::wstring& _text, const Vector2& _pos, const Vector2& _scale, const Vector2& _piv);


    void LoadFont(const std::string& _fontFilePath,float _fontSize);
    void CreateFontTexture();
    void CreateVertexBuffer();
    void CreateMatrixBuffer();
    void CreateProjectionMatrixBuffer();


    void UpdateFontTexture();
    void UploadVertexData();
    void UploadMatrixData();
    void RenderText();

    Vector2 GetStringAreaSize(const std::wstring& _text, const Vector2& _scale);

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

    std::vector<Matrix4x4> affineMatrices_; // アフィン変換行列
    size_t maxCharacters_ = 1500; // 最大文字数
    Microsoft::WRL::ComPtr<ID3D12Resource> matrixBuffer_; // アフィン変換行列のバッファ
    uint32_t affrinMatSRVIndex_ = UINT32_MAX; // アフィン変換行列のSRVインデックス

    Matrix4x4* projectionMatrix_; // 投影行列
    Microsoft::WRL::ComPtr<ID3D12Resource> projectionMatrixBuffer_; // 投影行列のバッファ

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