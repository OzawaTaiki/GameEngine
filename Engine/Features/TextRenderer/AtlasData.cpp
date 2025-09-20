#include "AtlasData.h"

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>

#include <fstream>
#include <cassert>

void AtlasData::Initialize(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, const Vector2& _windowSize, const std::string& _fontFilePath, float _fontSize,const Vector2& _atlasSize)
{
    if (!_device || !_cmdList)
        return;

    device_ = _device;
    cmdList_ = _cmdList;

    atlasSize_ = _atlasSize;

    srvManager_ = SRVManager::GetInstance();
    dxCommon_ = DXCommon::GetInstance();

    fontSize_ = _fontSize;

    atlasData_.resize(static_cast<size_t>(atlasSize_.x * atlasSize_.y), 0);
    currentX_ = 0;
    currentY_ = 0;
    maxY_ = 0;
    atlasNeedsUpdate_ = false;

    // フォント読み込み
    LoadFont(_fontFilePath, _fontSize);

    // フォントテクスチャの作成
    CreateFontTexture();

    // 文字の事前読み込み
    PreloadCommonCharacters();

    if (atlasNeedsUpdate_)
    {
        UpdateFontTexture();
        atlasNeedsUpdate_ = false;
    }

}

void AtlasData::LoadFont(const std::string& _fontFilePath, float _fontSize)
{
    std::ifstream file(_fontFilePath, std::ios::binary);
    if (!file.is_open())
    {
        assert("Failed to open font file");
        return;
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    fontBuffer_.resize(fileSize);
    file.read(reinterpret_cast<char*>(fontBuffer_.data()), fileSize);
    file.close();

    if (!stbtt_InitFont(&fontInfo_, fontBuffer_.data(), 0))
    {
        assert("Failed to initialize font");
        return;
    }

    scale_ = stbtt_ScaleForPixelHeight(&fontInfo_, _fontSize);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&fontInfo_, &ascent, &descent, &lineGap);

    // スケールを掛けてfloat型に変換
    fontAscent_ = ascent * scale_;
    fontDescent_ = descent * scale_;
    fontLineGap_ = lineGap * scale_;
}

void AtlasData::CreateFontTexture()
{
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width = static_cast<UINT64>(atlasSize_.x);
    texDesc.Height = static_cast<UINT>(atlasSize_.y);
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;


    HRESULT hr = device_->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr,
        IID_PPV_ARGS(&fontTexture_));

    if (FAILED(hr))
    {
        assert("Failed to create font texture resource");
        return;
    }

    textureIndex_= srvManager_->Allocate();
    srvManager_->CreateSRVForTexture2D(textureIndex_, fontTexture_.Get(), DXGI_FORMAT_R8_UNORM, 1);

    size_t uploadBufferSize = static_cast<size_t>(atlasSize_.x * atlasSize_.y);

    uploadBuffer_ = dxCommon_->CreateBufferResource(static_cast<uint32_t>(uploadBufferSize));
}

void AtlasData::UpdateFontTexture()
{// アップロード用の中間バッファ作成
    size_t uploadBufferSize = static_cast<size_t>(atlasSize_.x * atlasSize_.y);

    // データをアップロードバッファにコピー
    void* mappedData;
    HRESULT hr = uploadBuffer_->Map(0, nullptr, &mappedData);
    if (SUCCEEDED(hr))
    {
        memcpy(mappedData, atlasData_.data(), uploadBufferSize);
        uploadBuffer_->Unmap(0, nullptr);
    }

    // テクスチャコピー情報設定
    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource = uploadBuffer_.Get();
    srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    srcLocation.PlacedFootprint.Offset = 0;
    srcLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8_UNORM;
    srcLocation.PlacedFootprint.Footprint.Width = static_cast<UINT>(atlasSize_.x);
    srcLocation.PlacedFootprint.Footprint.Height = static_cast<UINT>(atlasSize_.y);
    srcLocation.PlacedFootprint.Footprint.Depth = 1;
    srcLocation.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(atlasSize_.x);

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource = fontTexture_.Get();
    dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = 0;

    // リソース状態遷移
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = fontTexture_.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmdList_->ResourceBarrier(1, &barrier);

    // テクスチャコピー実行
    cmdList_->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);

    // リソース状態を戻す
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    cmdList_->ResourceBarrier(1, &barrier);
}

void AtlasData::PreloadCommonCharacters()
{
    for (char c = 32; c < 127; c++)
    {
        GetGlyph(static_cast<wchar_t>(c));
    }
    for (wchar_t c = L'あ'; c <= L'ん'; c++)
    {
        GetGlyph(c);
    }
    for (wchar_t c = L'ア'; c <= L'ン'; c++)
    {
        GetGlyph(c);
    }

}

Vector2 AtlasData::GetStringAreaSize(const std::wstring& _text, const Vector2& _scale)
{
    Vector2 area = {};
    if (_text.empty()) return area;

    float currentX = 0.0f;
    float currentY = 0.0f;
    float maxWidth = 0.0f;
    float lineHeight = fontSize_ * _scale.y;

    for (size_t i = 0; i < _text.length(); ++i)
    {
        wchar_t character = _text[i];
        // 改行処理
        if (character == L'\n')
        {
            currentX = 0.0f;
            currentY += lineHeight;
            maxWidth = (std::max)(maxWidth, currentX);

            continue;
        }
        // スペース処理
        if (character == L' ')
        {
            currentX += fontSize_ * 0.3f * _scale.x;  // スペース幅
            continue;
        }
        // グリフ情報取得
        GlyphInfo glyph = GetGlyph(character);
        if (!glyph.isValid)
        {
            continue;
        }
        // 文字の幅を加算
        currentX += glyph.advance * _scale.x;
    }
    area.x = currentX;
    area.y = currentY + lineHeight;

    if (area.x < 0.0f) area.x = 0.0f;
    if (area.y < 0.0f) area.y = 0.0f;

    return area;

}

GlyphInfo AtlasData::GetGlyph(wchar_t _character)
{
    auto it = glyphs_.find(_character);
    if (it != glyphs_.end())
    {
        return it->second;
    }

    auto glyph = GenerateGlyph(_character);

    // 新しい文字が生成された場合は即座にテクスチャ更新
    if (atlasNeedsUpdate_)
    {
        UpdateFontTexture();
        atlasNeedsUpdate_ = false;
    }

    return glyph;
}

GlyphInfo AtlasData::GenerateGlyph(wchar_t _character)
{
    int width, height, xOffset, yOffset;

    unsigned char* bitmap = stbtt_GetCodepointBitmap(
        &fontInfo_, 0, scale_, _character,
        &width, &height, &xOffset, &yOffset);

    GlyphInfo glyph = {};

    if (!bitmap)
    {
        // 文字が見つからない場合は□を表示
        if (_character != L'□')
        {
            return GetGlyph(L'□');
        }
        glyph.isValid = false;
        return glyph;
    }

    // アトラスに配置
    if (currentX_ + width >= atlasSize_.x)
    {
        currentX_ = 0;
        currentY_ += maxY_;
        maxY_ = 0;
    }

    // ビットマップをアトラスにコピー
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int srcIndex = y * width + x;
            int dstIndex = static_cast<int>(currentY_ + y) * static_cast<int>(atlasSize_.x) + static_cast<int>(currentX_ + x);
            if (dstIndex < static_cast<int>(atlasData_.size())) {
                atlasData_[dstIndex] = bitmap[srcIndex];
            }
        }
    }

    // グリフ情報を設定
    glyph.u0 = static_cast<float>(currentX_) / atlasSize_.x;
    glyph.v0 = static_cast<float>(currentY_) / atlasSize_.y;
    glyph.u1 = static_cast<float>(currentX_ + width) / atlasSize_.x;
    glyph.v1 = static_cast<float>(currentY_ + height) / atlasSize_.y;
    glyph.width = static_cast<float>(width);
    glyph.height = static_cast<float>(height);
    glyph.bearingX = static_cast<float>(xOffset);
    glyph.bearingY = static_cast<float>(yOffset);
    glyph.isValid = true;

    // アドバンス幅を取得
    int advanceWidth, leftSideBearing;
    stbtt_GetCodepointHMetrics(&fontInfo_, _character, &advanceWidth, &leftSideBearing);
    glyph.advance = advanceWidth * scale_;

    // 位置を更新
    currentX_ += width + 1;
    maxY_ = (std::max)(maxY_, height + 1);

    // キャッシュに保存
    glyphs_[_character] = glyph;
    atlasNeedsUpdate_ = true;

    stbtt_FreeBitmap(bitmap, nullptr);
    return glyph;
}
