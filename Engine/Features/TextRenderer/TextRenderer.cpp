
#include <Externals/stb/stb_truetype.h>

#include "TextRenderer.h"

#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Math/Matrix/MatrixFunction.h>

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

    maxCharacters_ = 1500;
    maxVertices_ = 6 * maxCharacters_ ;// 1500文字分の頂点を確保


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

    // アフィン変換行列のバッファを作成
    CreateMatrixBuffer();

    // 投影行列のバッファを作成
    CreateProjectionMatrixBuffer();

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
    affineMatrices_.clear();

    device_ = nullptr;
    cmdList_ = nullptr;
    textureIndex_ = UINT32_MAX;
}

void TextRenderer::BeginFrame()
{
    // 頂点配列をクリア
    vertices_.clear();
    affineMatrices_.clear();

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
    // アフィン変換行列をGPUに転送
    UploadMatrixData();
    // 描画コマンド実行
    RenderText();
}

void TextRenderer::DrawText(const std::wstring& _text, const Vector2& _pos, const Vector4& _color)
{
    DrawText(_text, _pos, { 1.0f, 1.0f }, 0.0f, { 0.5f, 0.5f }, _color, _color);
}

void TextRenderer::DrawText(const std::wstring& _text, const TextParam& _param)
{
    if (_param.useOutline)
    {
        if (_param.useGradient)
            DrawTextWithOutline(_text, _param.position, _param.scale, _param.rotate, _param.pivot, _param.topColor, _param.bottomColor, _param.outlineColor, _param.outlineScale);
        else
            DrawTextWithOutline(_text, _param.position, _param.scale, _param.rotate, _param.pivot, _param.topColor, _param.topColor, _param.outlineColor, _param.outlineScale);
    }
    else
    {
        if (_param.useGradient)
            DrawText(_text, _param.position, _param.scale, _param.rotate, _param.pivot, _param.topColor, _param.bottomColor);
        else
            DrawText(_text, _param.position, _param.scale, _param.rotate, _param.pivot, _param.topColor, _param.topColor);
    }
}

void TextRenderer::DrawText(
    const std::wstring& _text,
    const Vector2& _pos,
    const Vector2& _scale,
    float _rotate,
    const Vector2& _piv,
    const Vector4& _topColor,
    const Vector4& _bottomColor)
{
    if (_text.empty()) return;

    float currentX = 0;
    float currentY = 0;

    Vector2 anchor = _piv;
    Vector2 stringArea = GetStringAreaSize(_text, _scale);
    Vector2 pivot = { stringArea.x * anchor.x, stringArea.y * anchor.y };


    Vector3 scale3D = { _scale.x, _scale.y, 1.0f };
    Vector3 rotate3D = { 0.0f, 0.0f, _rotate };
    Vector3 translate3D = { _pos.x, _pos.y, 0.0f };
    Matrix4x4 transformMatrix = MakeAffineMatrix(scale3D, rotate3D, translate3D);

    for (size_t i = 0; i < _text.length(); ++i)
    {
        wchar_t character = _text[i];

        // 改行処理
        if (character == L'\n')
        {
            currentX = 0;
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
        float glyphX = (currentX + glyph.bearingX) * _scale.x - pivot.x;
        float baseline = currentY + fontAscent_;
        float glyphY = (baseline + glyph.bearingY) * _scale.y - pivot.y;
        float glyphW = glyph.width * _scale.x;
        float glyphH = glyph.height * _scale.y;


        // 四角形を2つの三角形で構成（6頂点）
        TextVertex quad[6] =
        {
            // 1つ目の三角形 (左上, 右上, 左下)
            {{glyphX,          glyphY,          0.0f, 1.0f}, {glyph.u0, glyph.v0}, _topColor},
            {{glyphX + glyphW, glyphY,          0.0f, 1.0f}, {glyph.u1, glyph.v0}, _topColor},
            {{glyphX,          glyphY + glyphH, 0.0f, 1.0f}, {glyph.u0, glyph.v1}, _bottomColor},

            {{glyphX,          glyphY + glyphH, 0.0f, 1.0f}, {glyph.u0, glyph.v1}, _bottomColor},
            {{glyphX + glyphW, glyphY,          0.0f, 1.0f}, {glyph.u1, glyph.v0}, _topColor},
            {{glyphX + glyphW, glyphY + glyphH, 0.0f, 1.0f}, {glyph.u1, glyph.v1}, _bottomColor}
        };

        // 頂点配列に追加
        for (int j = 0; j < 6; ++j)
        {
            if (vertices_.size() < maxVertices_)
            {
                vertices_.push_back(quad[j]);
            }
        }

        // アフィン変換行列を設定
        affineMatrices_.push_back(transformMatrix);

        // 次の文字位置に移動
        currentX += glyph.advance;
    }
}

void TextRenderer::DrawTextWithOutline(const std::wstring& _text,
    const Vector2& _pos,
    const Vector2& _scale,
    float _rotate,
    const Vector2& _piv,
    const Vector4& _topColor,
    const Vector4& _bottomColor,
    const Vector4& _outlineColor,
    float _outlineThickness)
{
    if (_text.empty()) return;

    // アウトライン描画（先に描く）
    static const Vector2 offsetTable[8] = {
               {-1,  0}, {1,  0}, {0, -1}, {0, 1},
               {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
    };

    float offsetPx = fontSize_ * _outlineThickness;

    for (int i = 0; i < 8; ++i)
    {
        Vector2 offsetPos = {
            _pos.x + offsetTable[i].x * offsetPx,
            _pos.y + offsetTable[i].y * offsetPx
        };

        // アウトライン色で本関数を再帰的に呼び出す（本体色でなく、単色）
        DrawText(_text, offsetPos, _scale, _rotate, _piv, _outlineColor, _outlineColor);
    }

    float currentX = 0;
    float currentY = 0;

    Vector2 anchor = _piv;
    Vector2 stringArea = GetStringAreaSize(_text, _scale);
    Vector2 pivot = { stringArea.x * anchor.x, stringArea.y * anchor.y };


    Vector3 scale3D = { _scale.x, _scale.y, 1.0f };
    Vector3 rotate3D = { 0.0f, 0.0f, _rotate };
    Vector3 translate3D = { _pos.x, _pos.y, 0.0f };
    Matrix4x4 transformMatrix = MakeAffineMatrix(scale3D, rotate3D, translate3D);

    for (size_t i = 0; i < _text.length(); ++i)
    {
        wchar_t character = _text[i];

        // 改行処理
        if (character == L'\n')
        {
            currentX = 0;
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
        float glyphX = (currentX + glyph.bearingX) * _scale.x - pivot.x;
        float baseline = currentY + fontAscent_;
        float glyphY = (baseline + glyph.bearingY) * _scale.y - pivot.y;
        float glyphW = glyph.width * _scale.x;
        float glyphH = glyph.height * _scale.y;


        // 四角形を2つの三角形で構成（6頂点）
        TextVertex quad[6] =
        {
            // 1つ目の三角形 (左上, 右上, 左下)
            {{glyphX,          glyphY,          0.0f, 1.0f}, {glyph.u0, glyph.v0}, _topColor},
            {{glyphX + glyphW, glyphY,          0.0f, 1.0f}, {glyph.u1, glyph.v0}, _topColor},
            {{glyphX,          glyphY + glyphH, 0.0f, 1.0f}, {glyph.u0, glyph.v1}, _bottomColor},

            {{glyphX,          glyphY + glyphH, 0.0f, 1.0f}, {glyph.u0, glyph.v1}, _bottomColor},
            {{glyphX + glyphW, glyphY,          0.0f, 1.0f}, {glyph.u1, glyph.v0}, _topColor},
            {{glyphX + glyphW, glyphY + glyphH, 0.0f, 1.0f}, {glyph.u1, glyph.v1}, _bottomColor}
        };

        // 頂点配列に追加
        for (int j = 0; j < 6; ++j)
        {
            if (vertices_.size() < maxVertices_)
            {
                vertices_.push_back(quad[j]);
            }
        }

        // アフィン変換行列を設定
        affineMatrices_.push_back(transformMatrix);

        // 次の文字位置に移動
        currentX += glyph.advance;
    }
}

Vector2 TextRenderer::GetCenterPosition(const std::wstring& _text, const Vector2& _pos, const Vector2& _scale, const Vector2& _piv)
{
    Vector2 stringArea = GetStringAreaSize(_text, _scale);
    Vector2 pivot = { stringArea.x * _piv.x, stringArea.y * _piv.y };
    return _pos + pivot;
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

    size_t uploadBufferSize = static_cast<size_t>(atlasSize_.x * atlasSize_.y);

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

void TextRenderer::CreateMatrixBuffer()
{
    // マトリックスバッファの作成
    matrixBuffer_ = DXCommon::GetInstance()->CreateBufferResource(static_cast<uint32_t>(sizeof(Matrix4x4) * maxCharacters_));
    if (!matrixBuffer_)
    {
        assert("Failed to create matrix buffer resource");
        return;
    }

    affrinMatSRVIndex_ = SRVManager::GetInstance()->Allocate();

    SRVManager::GetInstance()->CreateSRVForStructureBuffer(affrinMatSRVIndex_, matrixBuffer_.Get(), static_cast<UINT>(maxCharacters_), sizeof(Matrix4x4));
}

void TextRenderer::CreateProjectionMatrixBuffer()
{
    // プロジェクションマトリックスバッファの作成
    projectionMatrixBuffer_ = DXCommon::GetInstance()->CreateBufferResource(static_cast<uint32_t>(sizeof(Matrix4x4)));
    if (!projectionMatrixBuffer_)
    {
        assert("Failed to create projection matrix buffer resource");
        return;
    }
    projectionMatrixBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&projectionMatrix_));

    *projectionMatrix_ = MakeOrthographicMatrix(0, 0, windowSize_.x, windowSize_.y, -1.0f, 1.0f);
}

void TextRenderer::UpdateFontTexture()
{
    // アップロード用の中間バッファ作成
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

void TextRenderer::UploadMatrixData()
{
    if (affineMatrices_.empty()) return;
    // アフィン変換行列をマップしてコピー
    void* mappedData;
    HRESULT hr = matrixBuffer_->Map(0, nullptr, &mappedData);
    if (SUCCEEDED(hr))
    {
        size_t dataSize = sizeof(Matrix4x4) * affineMatrices_.size();
        memcpy(mappedData, affineMatrices_.data(), dataSize);
        matrixBuffer_->Unmap(0, nullptr);
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

    cmdList_->SetGraphicsRootConstantBufferView(0, projectionMatrixBuffer_->GetGPUVirtualAddress());
    cmdList_->SetGraphicsRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(textureIndex_));
    cmdList_->SetGraphicsRootDescriptorTable(2, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(affrinMatSRVIndex_));


    // 描画実行
    UINT vertexCount = static_cast<UINT>(vertices_.size());
    cmdList_->DrawInstanced(vertexCount, 1, 0, 0);
}

Vector2 TextRenderer::GetStringAreaSize(const std::wstring& _text, const Vector2& _scale)
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
