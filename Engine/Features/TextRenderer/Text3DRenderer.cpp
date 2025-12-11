#include "Text3DRenderer.h"

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Core/DXCommon/PSOManager/PSOBuilder.h>
#include <Core/DXCommon/ShaderCompiler/ShaderCompiler.h>
#include <Math/Matrix/MatrixFunction.h>

namespace
{
constexpr float kPixelToWorldScale = 0.01f; // ピクセルからワールド単位への変換スケール
}

Text3DRenderer* Text3DRenderer::GetInstance()
{
    static Text3DRenderer instance;
    return &instance;
}

void Text3DRenderer::Initialize(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList)
{
    if (!_device || !_cmdList)
        return;

    device_ = _device;
    cmdList_ = _cmdList;

    CreateCameraBuffer();
    CreateRootSignature();
    CreatePipelineState();
    CreateImmediateResources();
}

void Text3DRenderer::Finalize()
{
    // 逐次描画用のPersistent Mappingを解除
    if (immediateVertexBuffer_ && immediateVertexMap_)
    {
        immediateVertexBuffer_->Unmap(0, nullptr);
        immediateVertexMap_ = nullptr;
    }

    if (immediateMatrixBuffer_ && immediateMatrixMap_)
    {
        immediateMatrixBuffer_->Unmap(0, nullptr);
        immediateMatrixMap_ = nullptr;
    }

    device_ = nullptr;
    cmdList_ = nullptr;
}

void Text3DRenderer::BeginFrame()
{
    for (auto& [textureIndex, res] : resourceDataGroups_)
    {
        res->vertices_.clear();
        res->worldMatrices_.clear();
    }
}

void Text3DRenderer::EndFrame()
{
    for (auto& [textureIndex, res] : resourceDataGroups_)
    {
        if (res->vertices_.empty())
            continue;

        UploadVertexData(res.get());
        UploadMatrixData(res.get());
        RenderText(res.get());
    }
}

void Text3DRenderer::DrawText3D(
    const std::wstring& _text,
    AtlasData* _atlas,
    const Camera* _camera,
    const Vector3& _position,
    const Vector3& _rotation,
    const Vector2& _scale,
    const Vector4& _color,
    const Vector2& _pivot)
{
    auto res = EnsureAtlasResources(_atlas);
    DrawTextInternal(_text, _position, _rotation, _scale, _pivot, _color, _color, res, _camera);
}

void Text3DRenderer::DrawText3D(
    const std::wstring& _text,
    AtlasData* _atlas,
    const Camera* _camera,
    const Vector3& _position,
    const Vector3& _rotation,
    const Vector2& _scale,
    const Vector4& _topColor,
    const Vector4& _bottomColor,
    const Vector2& _pivot)
{
    auto res = EnsureAtlasResources(_atlas);
    DrawTextInternal(_text, _position, _rotation, _scale, _pivot, _topColor, _bottomColor, res, _camera);
}

void Text3DRenderer::DrawText3DImmediate(
    const std::wstring& _text,
    AtlasData* _atlas,
    const Camera* _camera,
    const Vector3& _position,
    const Vector3& _rotation,
    const Vector2& _scale,
    const Vector4& _color,
    const Vector2& _pivot)
{
    DrawTextImmediate_Internal(_text, _atlas, _camera, _position, _rotation, _scale, _color, _color, _pivot);
}

void Text3DRenderer::DrawText3DImmediate(
    const std::wstring& _text,
    AtlasData* _atlas,
    const Camera* _camera,
    const Vector3& _position,
    const Vector3& _rotation,
    const Vector2& _scale,
    const Vector4& _topColor,
    const Vector4& _bottomColor,
    const Vector2& _pivot)
{
    DrawTextImmediate_Internal(_text, _atlas, _camera, _position, _rotation, _scale, _topColor, _bottomColor, _pivot);
}

void Text3DRenderer::DrawTextInternal(
    const std::wstring& _text,
    const Vector3& _position,
    const Vector3& _rotation,
    const Vector2& _scale,
    const Vector2& _pivot,
    const Vector4& _topColor,
    const Vector4& _bottomColor,
    ResourceDataGroup* _res,
    const Camera* _camera)
{
    if (_text.empty() || !_res || !_camera)
        return;

    AtlasData* atlas = _res->atlasData_;
    float fontSize = atlas->GetFontSize();
    float fontAscent = atlas->GetFontAscent();

    // ピクセルからワールドへの変換係数
    float worldScale = kPixelToWorldScale;

    // 文字列全体のサイズを計算（ピクセル単位）
    Vector2 stringAreaPixels = atlas->GetStringAreaSize(_text, Vector2::one);

    // ワールド単位に変換してピボット計算
    Vector2 pivot = {
        stringAreaPixels.x * worldScale * _pivot.x,
        stringAreaPixels.y * worldScale * _pivot.y
    };

    // ワールド変換行列を作成
    Matrix4x4 worldMatrix = MakeAffineMatrix(
        Vector3(_scale.x, -_scale.y, 1.0f),  // スケールはそのまま
        _rotation,
        _position
    );

    float currentX = 0.0f;
    float currentY = 0.0f;

    for (size_t i = 0; i < _text.length(); ++i)
    {
        wchar_t character = _text[i];

        // 改行処理
        if (character == L'\n')
        {
            currentX = 0.0f;
            currentY += fontSize;
            continue;
        }

        // スペース処理
        if (character == L' ')
        {
            currentX += fontSize * 0.3f;
            continue;
        }

        // グリフ情報取得
        GlyphInfo glyph = atlas->GetGlyph(character);
        if (!glyph.isValid)
            continue;

        // ★ ローカル座標計算（ピクセル単位 → ワールド単位に変換）
        float glyphX = (currentX + glyph.bearingX) * worldScale * _scale.x - pivot.x;
        float baseline = currentY + fontAscent;
        float glyphY = (baseline + glyph.bearingY) * worldScale * _scale.y - pivot.y;
        float glyphW = glyph.width * worldScale * _scale.x;
        float glyphH = glyph.height * worldScale * _scale.y;

        // 6頂点で四角形を構成
        std::vector<TextVertex> quad = {
            {{glyphX,          glyphY,          0.0f, 1.0f}, {glyph.u0, glyph.v0}, _topColor},
            {{glyphX + glyphW, glyphY,          0.0f, 1.0f}, {glyph.u1, glyph.v0}, _topColor},
            {{glyphX,          glyphY + glyphH, 0.0f, 1.0f}, {glyph.u0, glyph.v1}, _bottomColor},

            {{glyphX,          glyphY + glyphH, 0.0f, 1.0f}, {glyph.u0, glyph.v1}, _bottomColor},
            {{glyphX + glyphW, glyphY,          0.0f, 1.0f}, {glyph.u1, glyph.v0}, _topColor},
            {{glyphX + glyphW, glyphY + glyphH, 0.0f, 1.0f}, {glyph.u1, glyph.v1}, _bottomColor}
        };

        // 頂点データを追加
        _res->vertices_.insert(_res->vertices_.end(), quad.begin(), quad.end());

        // ワールド行列を追加
        _res->worldMatrices_.push_back(worldMatrix);

        // 次の文字位置に移動
        currentX += glyph.advance;
    }

    // カメラバッファを更新
    UpdateCameraBuffer(_camera);
}

Text3DRenderer::ResourceDataGroup* Text3DRenderer::EnsureAtlasResources(AtlasData* _atlas)
{
    if (!_atlas)
        return nullptr;

    int32_t atlasTextureIndex = _atlas->GetFontTextureIndex();
    auto it = resourceDataGroups_.find(atlasTextureIndex);
    if (it != resourceDataGroups_.end())
    {
        return it->second.get();
    }

    // 新しいリソースグループを作成
    auto resourceDataGroup = std::make_unique<ResourceDataGroup>();

    CreateVertexBuffer(resourceDataGroup.get());
    CreateMatrixBuffer(resourceDataGroup.get());

    resourceDataGroup->atlasData_ = _atlas;
    resourceDataGroup->textureIndex_ = atlasTextureIndex;

    resourceDataGroups_.emplace(atlasTextureIndex, std::move(resourceDataGroup));

    return resourceDataGroups_[atlasTextureIndex].get();
}

void Text3DRenderer::CreateVertexBuffer(ResourceDataGroup* _res)
{
    _res->vertexBuffer_ = DXCommon::GetInstance()->CreateBufferResource(
        static_cast<uint32_t>(sizeof(TextVertex) * maxVertices_)
    );

    if (!_res->vertexBuffer_)
    {
        assert(false && "Failed to create vertex buffer resource");
        return;
    }

    _res->vbv_.BufferLocation = _res->vertexBuffer_->GetGPUVirtualAddress();
    _res->vbv_.StrideInBytes = sizeof(TextVertex);
}

void Text3DRenderer::CreateMatrixBuffer(ResourceDataGroup* _res)
{
    _res->matrixBuffer_ = DXCommon::GetInstance()->CreateBufferResource(
        static_cast<uint32_t>(sizeof(Matrix4x4) * maxCharacters_)
    );

    if (!_res->matrixBuffer_)
    {
        assert(false && "Failed to create matrix buffer resource");
        return;
    }

    _res->matrixSRVIndex_ = SRVManager::GetInstance()->Allocate();
    SRVManager::GetInstance()->CreateSRVForStructureBuffer(
        _res->matrixSRVIndex_,
        _res->matrixBuffer_.Get(),
        static_cast<UINT>(maxCharacters_),
        sizeof(Matrix4x4)
    );
}

void Text3DRenderer::CreateCameraBuffer()
{
    cameraBuffer_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(CameraBuffer));

    if (!cameraBuffer_)
    {
        assert(false && "Failed to create camera buffer resource");
        return;
    }

    cameraBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cameraBufferMap_));
}

void Text3DRenderer::CreatePipelineState()
{
    // InputLayout定義
    static D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // PSOBuilderを使ってPSOを作成
    auto builder = PSOBuilder::Create();

    // inputLayoutを手動で設定



    pso_ = builder
        .SetShaders("Text3DRenderer.VS", "Text3DRenderer.PS")
        .SetBlendMode(PSOFlags::BlendMode::Normal)
        .SetCullMode(PSOFlags::CullMode::None)
        .SetDepthMode(PSOFlags::DepthMode::Comb_mZero_fLessEqual)
        .SetRootSignature(rootSignature_.Get())
        .SetInputLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>(
            std::begin(inputElementDescs),
            std::end(inputElementDescs)))
        .Build();

}

void Text3DRenderer::CreateRootSignature()
{
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // ルートパラメータ設定
    D3D12_ROOT_PARAMETER rootParameters[3] = {};

    // b0: カメラバッファ
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    // t0: ワールド行列バッファ（StructuredBuffer）
    D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRange[0];
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

    // t1: フォントテクスチャ
    descriptorRange[1].BaseShaderRegister = 1;
    descriptorRange[1].NumDescriptors = 1;
    descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange[1];
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

    rootSignatureDesc.pParameters = rootParameters;
    rootSignatureDesc.NumParameters = _countof(rootParameters);

    // サンプラー設定
    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 0;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.ShaderRegister = 0;
    samplerDesc.RegisterSpace = 0;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    rootSignatureDesc.pStaticSamplers = &samplerDesc;
    rootSignatureDesc.NumStaticSamplers = 1;

    // シリアライズ
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signatureBlob,
        &errorBlob
    );

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
        }
        assert(false && "Failed to serialize root signature");
        return;
    }

    // RootSignature作成
    hr = device_->CreateRootSignature(
        0,
        signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature_)
    );

    assert(SUCCEEDED(hr) && "Failed to create root signature for Text3DRenderer");
}

void Text3DRenderer::UploadVertexData(ResourceDataGroup* _res)
{
    if (_res->vertices_.empty())
        return;

    void* mappedData;
    HRESULT hr = _res->vertexBuffer_->Map(0, nullptr, &mappedData);
    if (SUCCEEDED(hr))
    {
        size_t dataSize = sizeof(TextVertex) * _res->vertices_.size();
        memcpy(mappedData, _res->vertices_.data(), dataSize);
        _res->vertexBuffer_->Unmap(0, nullptr);
    }
}

void Text3DRenderer::UploadMatrixData(ResourceDataGroup* _res)
{
    if (_res->worldMatrices_.empty())
        return;

    void* mappedData;
    HRESULT hr = _res->matrixBuffer_->Map(0, nullptr, &mappedData);
    if (SUCCEEDED(hr))
    {
        size_t dataSize = sizeof(Matrix4x4) * _res->worldMatrices_.size();
        memcpy(mappedData, _res->worldMatrices_.data(), dataSize);
        _res->matrixBuffer_->Unmap(0, nullptr);
    }
}

void Text3DRenderer::UpdateCameraBuffer(const Camera* _camera)
{
    if (!_camera || !cameraBufferMap_)
        return;

    cameraBufferMap_->view = _camera->matView_;
    cameraBufferMap_->projection = _camera->matProjection_;
    cameraBufferMap_->position = _camera->translate_;
}

void Text3DRenderer::RenderText(ResourceDataGroup* _res)
{
    if (_res->vertices_.empty())
        return;

    cmdList_->SetPipelineState(pso_.Get());
    cmdList_->SetGraphicsRootSignature(rootSignature_.Get());

    // 頂点バッファビュー設定
    _res->vbv_.SizeInBytes = static_cast<UINT>(sizeof(TextVertex) * _res->vertices_.size());

    cmdList_->IASetVertexBuffers(0, 1, &_res->vbv_);
    cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // ルートパラメータ設定
    cmdList_->SetGraphicsRootConstantBufferView(0, cameraBuffer_->GetGPUVirtualAddress());
    cmdList_->SetGraphicsRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(_res->matrixSRVIndex_));
    cmdList_->SetGraphicsRootDescriptorTable(2, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(_res->textureIndex_));

    // 描画実行
    UINT vertexCount = static_cast<UINT>(_res->vertices_.size());
    cmdList_->DrawInstanced(vertexCount, 1, 0, 0);
}

void Text3DRenderer::CreateImmediateResources()
{
    // 逐次描画専用の頂点バッファ作成
    immediateVertexBuffer_ = DXCommon::GetInstance()->CreateBufferResource(
        static_cast<uint32_t>(sizeof(TextVertex) * immediateMaxVertices_)
    );

    if (!immediateVertexBuffer_)
    {
        assert(false && "Failed to create immediate vertex buffer");
        return;
    }

    // 永続的にMapする
    immediateVertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&immediateVertexMap_));

    // VBV設定
    immediateVBV_.BufferLocation = immediateVertexBuffer_->GetGPUVirtualAddress();
    immediateVBV_.StrideInBytes = sizeof(TextVertex);

    // 逐次描画専用の行列バッファ作成
    immediateMatrixBuffer_ = DXCommon::GetInstance()->CreateBufferResource(
        static_cast<uint32_t>(sizeof(Matrix4x4) * immediateMaxCharacters_)
    );

    if (!immediateMatrixBuffer_)
    {
        assert(false && "Failed to create immediate matrix buffer");
        return;
    }

    // 永続的にMapする
    immediateMatrixBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&immediateMatrixMap_));

    // SRV作成
    immediateSRVIndex_ = SRVManager::GetInstance()->Allocate();
    SRVManager::GetInstance()->CreateSRVForStructureBuffer(
        immediateSRVIndex_,
        immediateMatrixBuffer_.Get(),
        static_cast<UINT>(immediateMaxCharacters_),
        sizeof(Matrix4x4)
    );
}

void Text3DRenderer::DrawTextImmediate_Internal(
    const std::wstring& _text,
    AtlasData* _atlas,
    const Camera* _camera,
    const Vector3& _position,
    const Vector3& _rotation,
    const Vector2& _scale,
    const Vector4& _topColor,
    const Vector4& _bottomColor,
    const Vector2& _pivot)
{
    if (_text.empty() || !_atlas || !_camera)
        return;

    if (!immediateVertexMap_ || !immediateMatrixMap_)
        return;

    // 一時バッファに頂点データを構築
    std::vector<TextVertex> tempVertices;
    std::vector<Matrix4x4> tempMatrices;

    float fontSize = _atlas->GetFontSize();
    float fontAscent = _atlas->GetFontAscent();
    float worldScale = kPixelToWorldScale;

    // 文字列全体のサイズを計算
    Vector2 stringAreaPixels = _atlas->GetStringAreaSize(_text, Vector2::one);
    Vector2 pivot = {
        stringAreaPixels.x * worldScale * _pivot.x,
        stringAreaPixels.y * worldScale * _pivot.y
    };

    // ワールド変換行列を作成
    Matrix4x4 worldMatrix = MakeAffineMatrix(
        Vector3(_scale.x, -_scale.y, 1.0f),
        _rotation,
        _position
    );

    float currentX = 0.0f;
    float currentY = 0.0f;

    for (size_t i = 0; i < _text.length(); ++i)
    {
        wchar_t character = _text[i];

        if (character == L'\n')
        {
            currentX = 0.0f;
            currentY += fontSize;
            continue;
        }

        if (character == L' ')
        {
            currentX += fontSize * 0.3f;
            continue;
        }

        GlyphInfo glyph = _atlas->GetGlyph(character);
        if (!glyph.isValid)
            continue;

        // ローカル座標計算
        float glyphX = (currentX + glyph.bearingX) * worldScale * _scale.x - pivot.x;
        float baseline = currentY + fontAscent;
        float glyphY = (baseline + glyph.bearingY) * worldScale * _scale.y - pivot.y;
        float glyphW = glyph.width * worldScale * _scale.x;
        float glyphH = glyph.height * worldScale * _scale.y;

        // 6頂点で四角形を構成
        std::vector<TextVertex> quad = {
            {{glyphX,          glyphY,          0.0f, 1.0f}, {glyph.u0, glyph.v0}, _topColor},
            {{glyphX + glyphW, glyphY,          0.0f, 1.0f}, {glyph.u1, glyph.v0}, _topColor},
            {{glyphX,          glyphY + glyphH, 0.0f, 1.0f}, {glyph.u0, glyph.v1}, _bottomColor},

            {{glyphX,          glyphY + glyphH, 0.0f, 1.0f}, {glyph.u0, glyph.v1}, _bottomColor},
            {{glyphX + glyphW, glyphY,          0.0f, 1.0f}, {glyph.u1, glyph.v0}, _topColor},
            {{glyphX + glyphW, glyphY + glyphH, 0.0f, 1.0f}, {glyph.u1, glyph.v1}, _bottomColor}
        };

        tempVertices.insert(tempVertices.end(), quad.begin(), quad.end());
        tempMatrices.push_back(worldMatrix);

        currentX += glyph.advance;

        // バッファサイズ超過チェック
        if (tempVertices.size() >= immediateMaxVertices_)
            break;
    }

    if (tempVertices.empty())
        return;

    // Persistent Mappingされたバッファに直接書き込み（Map/Unmap不要）
    memcpy(immediateVertexMap_, tempVertices.data(), sizeof(TextVertex) * tempVertices.size());
    memcpy(immediateMatrixMap_, tempMatrices.data(), sizeof(Matrix4x4) * tempMatrices.size());

    // カメラバッファ更新
    UpdateCameraBuffer(_camera);

    // 即座に描画
    cmdList_->SetPipelineState(pso_.Get());
    cmdList_->SetGraphicsRootSignature(rootSignature_.Get());

    immediateVBV_.SizeInBytes = static_cast<UINT>(sizeof(TextVertex) * tempVertices.size());
    cmdList_->IASetVertexBuffers(0, 1, &immediateVBV_);
    cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    cmdList_->SetGraphicsRootConstantBufferView(0, cameraBuffer_->GetGPUVirtualAddress());
    cmdList_->SetGraphicsRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(immediateSRVIndex_));
    cmdList_->SetGraphicsRootDescriptorTable(2, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(_atlas->GetFontTextureIndex()));

    UINT vertexCount = static_cast<UINT>(tempVertices.size());
    cmdList_->DrawInstanced(vertexCount, 1, 0, 0);
}
