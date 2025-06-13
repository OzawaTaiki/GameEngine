
#include <Externals/stb/stb_truetype.h>

#include "TextRenderer.h"

#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>

TextRenderer* TextRenderer::GetInstance()
{
    static TextRenderer instance;
    return &instance;
}

void TextRenderer::Initialize(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList, const Config& _config, const Vector2& _windowSize)
{
    if (!_device || !_cmdList)
        return;

    device_ = _device;
    cmdList_ = _cmdList;

    atlasSize_.x = _config.atlasSize.x;
    atlasSize_.y = _config.atlasSize.y;

    fontSize_ = _config.fontSize;

    windowSize_ = _windowSize;


    maxVertices_ = 10000;


    atlasData_.resize(static_cast<size_t>(atlasSize_.x * atlasSize_.y) ,0);
    currentX_ = 0;
    currentY_ = 0;
    maxY_ = 0;
    atlasNeedsUpdate_ = false;

    auto psoMana = PSOManager::GetInstance();
    // パイプラインステートオブジェクトの取得
    pso_ = psoMana->GetPipeLineStateObject(PSOFlags::ForText()).value_or(nullptr);
    if (!pso_)
    {
        assert(false && "TextRenderer: PSO not found. Please initialize PSOManager.");
        return;
    }

    // ルートシグネチャの取得
    rootSignature_ = psoMana->GetRootSignature(PSOFlags::ForText()).value_or(nullptr);
    if (!rootSignature_)
    {
        assert(false && "TextRenderer: RootSignature not found. Please initialize PSOManager.");
        return;
    }


    // フォント読み込み
    LoadFont(_config.fontFilePath, _config.fontSize);

    // フォントテクスチャの作成
    CreateFontTexture();

    // 頂点バッファの作成
    CreateVertexBuffer();

    // よく使う文字を事前生成
    PreloadCommonCharacters();

    if (atlasNeedsUpdate_)
    {
        UpdateFontTexture();
        atlasNeedsUpdate_ = false;
    }

}

void TextRenderer::Finalize()
{
    // テクスチャリソース解放
    if (fontTexture_)
    {
        fontTexture_.Reset();
    }

    // 頂点バッファ解放
    if (vertexBuffer_)
    {
        vertexBuffer_.Reset();
    }

    // メンバ変数をリセット
    glyphs_.clear();
    vertices_.clear();
    fontBuffer_.clear();
    atlasData_.clear();

    device_ = nullptr;
    cmdList_ = nullptr;
    textureIndex_ = UINT32_MAX;
}

void TextRenderer::BeginFrame()
{
    // 頂点配列をクリア
    vertices_.clear();

    // フォントアトラスが更新されている場合はテクスチャを更新
    if (atlasNeedsUpdate_)
    {
        UpdateFontTexture();
        atlasNeedsUpdate_ = false;
    }
}

void TextRenderer::EndFrame()
{
    if (vertices_.empty())
    {
        return;  // 描画するものがない
    }

    // 頂点データをGPUに転送
    UploadVertexData();

    // 描画コマンド実行
    RenderText();
}

void TextRenderer::DrawText(const std::wstring& _text, const Vector2& _pos, const Vector4& _color)
{
    if (_text.empty()) return;

    float currentX = _pos.x;
    float currentY = _pos.y;

    // スクリーン座標を正規化座標に変換するための係数
    float screenToNDCX = 2.0f / windowSize_.x;
    float screenToNDCY = -2.0f / windowSize_.y;  // Yは反転

    for (size_t i = 0; i < _text.length(); ++i)
    {
        wchar_t character = _text[i];

        // 改行処理
        if (character == L'\n')
        {
            currentX = _pos.x;
            currentY += fontSize_;
            continue;
        }

        // スペース処理
        if (character == L' ')
        {
            currentX += fontSize_ * 0.3f;  // スペース幅
            continue;
        }

        // グリフ情報取得
        GlyphInfo glyph = GetGlyph(character);
        if (!glyph.isValid)
        {
            continue;
        }

        // 文字の描画位置計算
        float glyphX = currentX + glyph.bearingX;
        float baseline = currentY + fontAscent_;
        float glyphY = baseline + glyph.bearingY;
        float glyphW = glyph.width;
        float glyphH = glyph.height;

        // 正規化座標に変換
        float x1 = (glyphX * screenToNDCX) - 1.0f;
        float y1 = (glyphY * screenToNDCY) + 1.0f;
        float x2 = ((glyphX + glyphW) * screenToNDCX) - 1.0f;
        float y2 = ((glyphY + glyphH) * screenToNDCY) + 1.0f;

        // 四角形を2つの三角形で構成（6頂点）
        TextVertex quad[6] =
        {
            // 1つ目の三角形 (左上, 右上, 左下)
            {{x1, y1, 0.0f, 1.0f}, {glyph.u0, glyph.v0}, _color},
            {{x2, y1, 0.0f, 1.0f}, {glyph.u1, glyph.v0}, _color},
            {{x1, y2, 0.0f, 1.0f}, {glyph.u0, glyph.v1}, _color},

            // 2つ目の三角形 (左下, 右上, 右下)
            {{x1, y2, 0.0f, 1.0f}, {glyph.u0, glyph.v1}, _color},
            {{x2, y1, 0.0f, 1.0f}, {glyph.u1, glyph.v0}, _color},
            {{x2, y2, 0.0f, 1.0f}, {glyph.u1, glyph.v1}, _color}
        };

        // 頂点配列に追加
        for (int j = 0; j < 6; ++j)
        {
            if (vertices_.size() < maxVertices_)
            {
                vertices_.push_back(quad[j]);
            }
        }

        // 次の文字位置に移動
        currentX += glyph.advance;
    }
}

void TextRenderer::LoadFont(const std::string& _fontFilePath, float _fontSize)
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

void TextRenderer::CreateFontTexture()
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

    textureIndex_ = SRVManager::GetInstance()->Allocate();
    SRVManager::GetInstance()->CreateSRVForTextrue2D(textureIndex_, fontTexture_.Get(), DXGI_FORMAT_R8_UNORM, 1);

    size_t uploadBufferSize = atlasSize_.x * atlasSize_.y;

    uploadBuffer_ = DXCommon::GetInstance()->CreateBufferResource(static_cast<uint32_t>(uploadBufferSize));
}

void TextRenderer::CreateVertexBuffer()
{
    vertexBuffer_ = DXCommon::GetInstance()->CreateBufferResource(static_cast<uint32_t>(sizeof(TextVertex) * maxVertices_));

    if (!vertexBuffer_)
    {
        assert("Failed to create vertex buffer resource");
        return;
    }
    // 頂点バッファビューの設定
    vbv_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
    vbv_.StrideInBytes = sizeof(TextVertex);

}

void TextRenderer::UpdateFontTexture()
{
    // アップロード用の中間バッファ作成
    size_t uploadBufferSize = atlasSize_.x * atlasSize_.y;


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

void TextRenderer::UploadVertexData()
{
    if (vertices_.empty()) return;

    // 頂点バッファにデータをマップしてコピー
    void* mappedData;
    HRESULT hr = vertexBuffer_->Map(0, nullptr, &mappedData);
    if (SUCCEEDED(hr))
    {
        size_t dataSize = sizeof(TextVertex) * vertices_.size();
        memcpy(mappedData, vertices_.data(), dataSize);
        vertexBuffer_->Unmap(0, nullptr);
    }
}

void TextRenderer::RenderText()
{
    if (vertices_.empty()) return;

    cmdList_->SetPipelineState(pso_);
    cmdList_->SetGraphicsRootSignature(rootSignature_);

    // 頂点バッファビュー設定
    vbv_.SizeInBytes = static_cast<UINT>(sizeof(TextVertex) * vertices_.size());

    // 描画コマンド
    cmdList_->IASetVertexBuffers(0, 1, &vbv_);
    cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    cmdList_->SetGraphicsRootDescriptorTable(0, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(textureIndex_));

    // 描画実行
    UINT vertexCount = static_cast<UINT>(vertices_.size());
    cmdList_->DrawInstanced(vertexCount, 1, 0, 0);
}

void TextRenderer::PreloadCommonCharacters()
{
    for (char c = 32; c < 127; c++)
    {
        GetGlyph(static_cast<wchar_t>(c));
    }

}

TextRenderer::GlyphInfo TextRenderer::GetGlyph(wchar_t _character)
{
    auto it = glyphs_.find(_character);
    if (it != glyphs_.end())
    {
        return it->second;
    }
    return GenerateGlyph(_character);
}

TextRenderer::GlyphInfo TextRenderer::GenerateGlyph(wchar_t _character)
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
            int dstIndex = static_cast<int>(currentY_ + y) * static_cast<int>(atlasSize_.x)+ static_cast<int>(currentX_ + x);
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
