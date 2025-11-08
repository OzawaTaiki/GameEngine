#pragma once

#include <Externals/stb/stb_truetype.h>

#include <Math/Vector/Vector2.h>

#include <d3d12.h>
#include <wrl.h>

#include <string>
#include <unordered_map>
#include <map>

struct GlyphInfo
{
    float u0, v0, u1, v1;      // UV座標
    float width, height;        // サイズ
    float bearingX, bearingY;   // オフセット
    float advance;              // 次の文字までの距離
    bool isValid = false;
};


struct FontConfig
{
    Vector2 atlasSize = { 1024,1024 }; // アトラスの幅
    float fontSize = 32.0f; // フォントサイズ
    std::string fontFilePath = "Resources/Fonts/NotoSansJP-Regular.ttf"; // フォントファイルのパス
};

class DXCommon;
class SRVManager;
class  AtlasData
{
public:

    void Initialize(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, const std::string& _fontFilePath, float _fontSize, const Vector2& _atlasSize);

public: /// アクセッサ

    // フォントテクスチャを取得
    ID3D12Resource* GetFontTexture() const { return fontTexture_.Get(); }

    // フォントテクスチャのSRVインデックスを取得
    uint32_t GetFontTextureIndex() const { return textureIndex_; }

    // フォントサイズを取得
    float GetFontSize() const { return fontSize_; }

    // フォントのアセントを取得
    float GetFontAscent() const { return fontAscent_; }

    // グリフ情報を取得
    GlyphInfo GetGlyph(wchar_t _character);

    // テキストエリアを取得する
    Vector2 GetStringAreaSize(const std::wstring& _text, const Vector2& _scale);

private:

    // フォントの読み込み
    void LoadFont(const std::string& _fontFilePath, float _fontSize);

    // フォントテクスチャの作成
    void CreateFontTexture();

    // フォントテクスチャの更新
    void UpdateFontTexture();

    // 文字の事前読み込み
    void PreloadCommonCharacters();

    // グリフ情報を生成
    GlyphInfo GenerateGlyph(wchar_t _character);

private:

    SRVManager* srvManager_; // SRVマネージャー
    DXCommon* dxCommon_; // DXCommon
    ID3D12Device* device_; // D3D12デバイス
    ID3D12GraphicsCommandList* cmdList_; // コマンドリスト


    // GPU リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> fontTexture_;
    Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer_;
    uint32_t textureIndex_;

    // フォント情報
    stbtt_fontinfo fontInfo_;
    std::vector<uint8_t> fontBuffer_;
    float scale_;
    float fontSize_;

    // アトラス管理
    std::vector<uint8_t> atlasData_;
    Vector2 atlasSize_;
    int32_t currentX_;
    int32_t currentY_;
    int32_t maxY_;
    std::unordered_map<wchar_t, GlyphInfo> glyphs_;
    bool atlasNeedsUpdate_;

    // フォントメトリクス
    float fontAscent_; // ベースラインから上への距離
    float fontDescent_; // ベースラインから下への距離（負の値）
    float fontLineGap_; // 行間

};