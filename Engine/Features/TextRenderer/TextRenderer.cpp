
#include <Externals/stb/stb_truetype.h>

#include "TextRenderer.h"

#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Framework/Batch2DRenderer.h>

TextRenderer* TextRenderer::GetInstance()
{
    static TextRenderer instance;
    return &instance;
}

void TextRenderer::Initialize(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList,  const Vector2& _windowSize)
{
    if (!_device || !_cmdList)
        return;

    device_ = _device;
    cmdList_ = _cmdList;

    windowSize_ = _windowSize;

    maxCharacters_ = 1500;
    maxVertices_ = 6 * maxCharacters_ ;// 1500文字分の頂点を確保

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
    CreateProjectionMatrixBuffer();
}

void TextRenderer::Finalize()
{

    device_ = nullptr;
    cmdList_ = nullptr;
}

void TextRenderer::BeginFrame()
{
    for (auto& [textureindex, res] : resourceDataGroups_)
    {
        // 頂点配列をクリア
        res->vertices_.clear();
        res->affineMatrices_.clear();
    }
}

void TextRenderer::EndFrame()
{
    for (auto& [textureindex, res] : resourceDataGroups_)
    {
        if (res->vertices_.empty())
            continue;

        // 頂点データをGPUに転送
        UploadVertexData(res.get());
        // アフィン変換行列をGPUに転送
        UploadMatrixData(res.get());
        // 描画コマンド実行
        RenderText(res.get());
    }
}

void TextRenderer::DrawText(const std::wstring& _text, AtlasData* _atlas, const Vector2& _pos, const Vector4& _color)
{
    auto res = EnsureAtlasResources(_atlas);
    DrawText(_text, _pos, { 1.0f, 1.0f }, 0.0f, { 0.5f, 0.5f }, _color, _color, res);
}

void TextRenderer::DrawText(const std::wstring& _text, AtlasData* _atlas, const TextParam& _param)
{
    auto res = EnsureAtlasResources(_atlas);

    if (_param.useOutline)
    {
        if (_param.useGradient)
            DrawTextWithOutline(_text, _param.position, _param.scale, _param.rotate, _param.pivot, _param.topColor, _param.bottomColor, _param.outlineColor, _param.outlineScale, res);
        else
            DrawTextWithOutline(_text, _param.position, _param.scale, _param.rotate, _param.pivot, _param.topColor, _param.topColor, _param.outlineColor, _param.outlineScale, res);
    }
    else
    {
        if (_param.useGradient)
            DrawText(_text, _param.position, _param.scale, _param.rotate, _param.pivot, _param.topColor, _param.bottomColor, res);
        else
            DrawText(_text, _param.position, _param.scale, _param.rotate, _param.pivot, _param.topColor, _param.topColor, res);
    }
}

void TextRenderer::DrawText(
    const std::wstring& _text,
    const Vector2& _pos,
    const Vector2& _scale,
    float _rotate,
    const Vector2& _piv,
    const Vector4& _topColor,
    const Vector4& _bottomColor,
    ResourceDataGroup* _res)
{
    if (_text.empty()) return;

    float currentX = 0;
    float currentY = 0;

    AtlasData* atlas = _res->atlasData_;
    float fontSize = atlas->GetFontSize();
    float fontAscent = atlas->GetFontAscent();

    Vector2 anchor = _piv;
    Vector2 stringArea = atlas->GetStringAreaSize(_text, _scale);
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
            currentY += fontSize;
            continue;
        }

        // スペース処理
        if (character == L' ')
        {
            currentX += fontSize * 0.3f;  // スペース幅
            continue;
        }

        // グリフ情報取得
        GlyphInfo glyph = atlas->GetGlyph(character);
        if (!glyph.isValid)
        {
            continue;
        }

        // 文字の描画位置計算
        float glyphX = (currentX + glyph.bearingX) * _scale.x - pivot.x;
        float baseline = currentY + fontAscent;
        float glyphY = (baseline + glyph.bearingY) * _scale.y - pivot.y;
        float glyphW = glyph.width * _scale.x;
        float glyphH = glyph.height * _scale.y;


        // 四角形を2つの三角形で構成（6頂点）
        std::vector<Batch2DRenderer::VertexData> quad=
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
            if (_res->vertices_.size() < maxVertices_)
            {
                //_res->vertices_.push_back(quad[j]);
            }
        }

        // アフィン変換行列を設定
        _res->affineMatrices_.push_back(transformMatrix);
        Batch2DRenderer::InstanceData data;
        data.color = Vector4(1, 1, 1, 1);
        data.textureIndex = _res->textureIndex_;
        data.useTextureAlpha = 1; // テキスト
        data.transform = transformMatrix;
        data.uvTransform = Matrix4x4::Identity();

        Batch2DRenderer::GetInstance()->AddInstace(data, quad);

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
    float _outlineThickness,
    ResourceDataGroup* _res)
{
    if (_text.empty()) return;

    // アウトライン描画（先に描く）
    static const Vector2 offsetTable[8] = {
               {-1,  0}, {1,  0}, {0, -1}, {0, 1},
               {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
    };


    AtlasData* atlas = _res->atlasData_;
    float fontSize = atlas->GetFontSize();
    float fontAscent = atlas->GetFontAscent();

    float offsetPx = fontSize * _outlineThickness;

    for (int i = 0; i < 8; ++i)
    {
        Vector2 offsetPos = {
            _pos.x + offsetTable[i].x * offsetPx,
            _pos.y + offsetTable[i].y * offsetPx
        };

        // アウトライン色で本関数を再帰的に呼び出す（本体色でなく、単色）
        DrawText(_text, offsetPos, _scale, _rotate, _piv, _outlineColor, _outlineColor, _res);
    }

    float currentX = 0;
    float currentY = 0;

    Vector2 anchor = _piv;
    Vector2 stringArea = atlas->GetStringAreaSize(_text, _scale);
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
            currentY += fontSize;
            continue;
        }

        // スペース処理
        if (character == L' ')
        {
            currentX += fontSize * 0.3f;  // スペース幅
            continue;
        }

        // グリフ情報取得
        GlyphInfo glyph = atlas->GetGlyph(character);
        if (!glyph.isValid)
        {
            continue;
        }

        // 文字の描画位置計算
        float glyphX = (currentX + glyph.bearingX) * _scale.x - pivot.x;
        float baseline = currentY + fontAscent;
        float glyphY = (baseline + glyph.bearingY) * _scale.y - pivot.y;
        float glyphW = glyph.width * _scale.x;
        float glyphH = glyph.height * _scale.y;


        // 四角形を2つの三角形で構成（6頂点）
        std::vector<Batch2DRenderer::VertexData> quad =
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
            if (_res->vertices_.size() < maxVertices_)
            {
                //_res->vertices_.push_back(quad[j]);
            }
        }

        // アフィン変換行列を設定
        _res->affineMatrices_.push_back(transformMatrix);

        Batch2DRenderer::InstanceData data;
        data.color = Vector4(1, 1, 1, 1);
        data.textureIndex = _res->textureIndex_;
        data.useTextureAlpha = 1; // テキスト
        data.transform = transformMatrix;
        data.uvTransform = Matrix4x4::Identity();
        Batch2DRenderer::GetInstance()->AddInstace(data, quad);


        // 次の文字位置に移動
        currentX += glyph.advance;
    }
}


TextRenderer::ResourceDataGroup* TextRenderer::EnsureAtlasResources(AtlasData* _atlas)
{
    if (!_atlas)
        return nullptr;

    int32_t atlasTextureIndex = _atlas->GetFontTextureIndex();
    auto it = resourceDataGroups_.find(atlasTextureIndex);
    if (it != resourceDataGroups_.end())
    {
        return it->second.get(); // 既存のリソースグループを返す
    }

    // 新しいアトラスデータグループを作成
    auto resourceDataGroup = std::make_unique<ResourceDataGroup>();

    // 頂点バッファの作成
    CreateVertexBuffer(resourceDataGroup.get());

    // アフィン変換行列のバッファを作成
    CreateMatrixBuffer(resourceDataGroup.get());

    // アトラスデータを設定
    resourceDataGroup->atlasData_ = _atlas;
    resourceDataGroup->textureIndex_ = atlasTextureIndex;

    resourceDataGroups_.emplace(atlasTextureIndex, std::move(resourceDataGroup));


    return resourceDataGroups_[atlasTextureIndex].get();

}

void TextRenderer::CreateVertexBuffer(ResourceDataGroup* _res)
{
    _res->vertexBuffer_ = DXCommon::GetInstance()->CreateBufferResource(static_cast<uint32_t>(sizeof(TextVertex) * maxVertices_));

    if (!_res->vertexBuffer_)
    {
        assert("Failed to create vertex buffer resource");
        return;
    }
    // 頂点バッファビューの設定
    _res->vbv_.BufferLocation = _res->vertexBuffer_->GetGPUVirtualAddress();
    _res->vbv_.StrideInBytes = sizeof(TextVertex);

}

void TextRenderer::CreateMatrixBuffer(ResourceDataGroup* _res)
{
    // マトリックスバッファの作成
    _res->matrixBuffer_ = DXCommon::GetInstance()->CreateBufferResource(static_cast<uint32_t>(sizeof(Matrix4x4) * maxCharacters_));
    if (!_res->matrixBuffer_)
    {
        assert("Failed to create matrix buffer resource");
        return;
    }

    _res->affrinMatSRVIndex_ = SRVManager::GetInstance()->Allocate();

    SRVManager::GetInstance()->CreateSRVForStructureBuffer(_res->affrinMatSRVIndex_, _res->matrixBuffer_.Get(), static_cast<UINT>(maxCharacters_), sizeof(Matrix4x4));
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

void TextRenderer::UploadVertexData(ResourceDataGroup* _res)
{
    if (_res->vertices_.empty()) return;

    // 頂点バッファにデータをマップしてコピー
    void* mappedData;
    HRESULT hr = _res->vertexBuffer_->Map(0, nullptr, &mappedData);
    if (SUCCEEDED(hr))
    {
        size_t dataSize = sizeof(TextVertex) * _res->vertices_.size();
        memcpy(mappedData, _res->vertices_.data(), dataSize);
        _res->vertexBuffer_->Unmap(0, nullptr);
    }
}

void TextRenderer::UploadMatrixData(ResourceDataGroup* _res)
{
    if (_res->affineMatrices_.empty()) return;
    // アフィン変換行列をマップしてコピー
    void* mappedData;
    HRESULT hr = _res->matrixBuffer_->Map(0, nullptr, &mappedData);
    if (SUCCEEDED(hr))
    {
        size_t dataSize = sizeof(Matrix4x4) * _res->affineMatrices_.size();
        memcpy(mappedData, _res->affineMatrices_.data(), dataSize);
        _res->matrixBuffer_->Unmap(0, nullptr);
    }
}

void TextRenderer::RenderText(ResourceDataGroup* _res)
{
    if (_res->vertices_.empty() || true) return;

    cmdList_->SetPipelineState(pso_);
    cmdList_->SetGraphicsRootSignature(rootSignature_);

    // 頂点バッファビュー設定
    _res->vbv_.SizeInBytes = static_cast<UINT>(sizeof(TextVertex) * _res->vertices_.size());

    // 描画コマンド
    cmdList_->IASetVertexBuffers(0, 1, &_res->vbv_);
    cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    cmdList_->SetGraphicsRootConstantBufferView(0, projectionMatrixBuffer_->GetGPUVirtualAddress());
    cmdList_->SetGraphicsRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(_res->textureIndex_));
    cmdList_->SetGraphicsRootDescriptorTable(2, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(_res->affrinMatSRVIndex_));


    // 描画実行
    UINT vertexCount = static_cast<UINT>(_res->vertices_.size());
    cmdList_->DrawInstanced(vertexCount, 1, 0, 0);
}
