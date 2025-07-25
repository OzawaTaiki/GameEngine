#include "ColorMask.h"
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Core/DXCommon/RTV/RTVManager.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Debug/Debug.h>

std::unique_ptr<ColorMask> ColorMask::instance_ = nullptr;


ColorMask* ColorMask::GetInstance()
{
    if (!instance_)
    {
        Initialize();
    }
    return instance_.get();
}

void ColorMask::DisPatch(uint32_t _monoTexHandle, uint32_t _colorTexHandle, uint32_t _outputTexHandle, ColorMaskData _data, ID3D12GraphicsCommandList* _commandList, uint32_t _width, uint32_t _height)
{
    *instance_->constantBufferMap_ = _data;


    auto it = instance_->textureSRVMap_.find(_outputTexHandle);//UAV
    if (it == instance_->textureSRVMap_.end())
        return;

    auto it2 = instance_->outputTextures_.find(it->second);
    if (it2 != instance_->outputTextures_.end())
        // すでに存在する場合は、RTVを設定
        RTVManager::GetInstance()->GetRenderTexture(it2->second)->ChangeRTVState(_commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    _commandList->SetPipelineState(instance_->pipelineState_.Get());
    _commandList->SetComputeRootSignature(instance_->rootSignature_.Get());

    // 定数バッファを設定
    _commandList->SetComputeRootConstantBufferView(0, instance_->constantBuffer_->GetGPUVirtualAddress());
    // 入力テクスチャ mono
    _commandList->SetComputeRootDescriptorTable(1, TextureManager::GetInstance()->GetGPUHandle(_monoTexHandle));
    // 入力テクスチャ color
    _commandList->SetComputeRootDescriptorTable(2, TextureManager::GetInstance()->GetGPUHandle(_colorTexHandle));
    // 出力テクスチャ
    _commandList->SetComputeRootDescriptorTable(3, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(it->second));

    // スレッドグループ数計算 (8x8のスレッドグループ)
    uint32_t dispatchX = (_width + 7) / 8;
    uint32_t dispatchY = (_height + 7) / 8;

    // Dispatch実行
    _commandList->Dispatch(dispatchX, dispatchY, 1);

    if (it2 != instance_->outputTextures_.end())
    {
        // すでに存在する場合は、RTVを設定
        RTVManager::GetInstance()->GetRenderTexture(it2->second)->ChangeRTVState(_commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

}


uint32_t ColorMask::CreateOutputTexture(const std::string& _name, uint32_t _width, uint32_t _height, const Vector4& _clearColor, DXGI_FORMAT _format)
{
    if (!instance_)
        Initialize();

    uint32_t rtvIndex = RTVManager::GetInstance()->CreateComputeOutputTexture(_name, _width, _height, _format, _clearColor);

    auto srvManager = SRVManager::GetInstance();
    uint32_t UAVIndex = srvManager->Allocate();
    auto boxFilterResource = RTVManager::GetInstance()->GetRenderTexture(_name)->GetRTVResource();
    srvManager->CreateUAVForTexture2D(UAVIndex, boxFilterResource, DXGI_FORMAT_R8G8B8A8_UNORM);

    uint32_t SRVIndex = srvManager->Allocate();
    srvManager->CreateSRVForTexture2D(SRVIndex, boxFilterResource, DXGI_FORMAT_R8G8B8A8_UNORM,1);


    instance_->textureMap_[_name] = UAVIndex; // テクスチャ名とUAVハンドルのマッピング
    instance_->outputTextures_[UAVIndex] = rtvIndex;
    instance_->textureSRVMap_[SRVIndex] = UAVIndex;

    return SRVIndex;
}

uint32_t ColorMask::GetOutputTextureHandle(const std::string& _name)
{
    if (!instance_)
        Initialize();

    auto it = instance_->textureMap_.find(_name);
    if (it != instance_->textureMap_.end())
    {
        for (auto& [srv, uav] : instance_->textureSRVMap_)
        {
            if (uav == it->second)
            {
                // UAVとSRVのマッピングを返す
                return srv;
            }
        }
    }
    // 存在しない場合は0を返す
    return 0;
}

void ColorMask::Finalize()
{
    if (instance_)
    {
        instance_->constantBuffer_.Reset();
        instance_->pipelineState_.Reset();
        instance_->rootSignature_.Reset();
        instance_.reset();
    }
}

void ColorMask::Initialize()
{
    if (!instance_)
    {
        instance_ = std::make_unique<ColorMask>();
    }

    // 定数バッファを生成
    instance_->CreateConstantBuffer();
    // ルートシグネチャを生成
    instance_->CreateRootSignature();
    // パイプラインステートを生成
    instance_->CreatePipelineState();
}

void ColorMask::CreatePipelineState()
{
    HRESULT hr = S_FALSE;

#pragma region Shader
    // Compute Shaderをコンパイルする
    Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob =
        PSOManager::GetInstance()->ComplieShader(L"ColorMask.CS.hlsl", L"cs_6_0");
    assert(computeShaderBlob != nullptr);
#pragma endregion

    // Compute PSOを生成する
    D3D12_COMPUTE_PIPELINE_STATE_DESC computePipelineStateDesc{};
    computePipelineStateDesc.pRootSignature = rootSignature_.Get();
    computePipelineStateDesc.CS = { computeShaderBlob->GetBufferPointer(), computeShaderBlob->GetBufferSize() };
    computePipelineStateDesc.NodeMask = 0;
    computePipelineStateDesc.CachedPSO.pCachedBlob = nullptr;
    computePipelineStateDesc.CachedPSO.CachedBlobSizeInBytes = 0;
    computePipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    // PSOを生成
    hr = DXCommon::GetInstance()->GetDevice()->CreateComputePipelineState(&computePipelineStateDesc, IID_PPV_ARGS(&pipelineState_));
    assert(SUCCEEDED(hr));
}

void ColorMask::CreateRootSignature()
{
    HRESULT hr = S_FALSE;

    // RootSignatureを生成する
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE; // Compute用

    // descriptorRange
    D3D12_DESCRIPTOR_RANGE descriptorRange[3] = {};

    // SRV (t0) - 入力テクスチャ mono
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // SRV (t1) - 入力テクスチャ colot
    descriptorRange[1].BaseShaderRegister = 1;
    descriptorRange[1].NumDescriptors = 1;
    descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // UAV (u0) - 出力テクスチャ
    descriptorRange[2].BaseShaderRegister = 0;
    descriptorRange[2].NumDescriptors = 1;
    descriptorRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // RootParameter作成
    D3D12_ROOT_PARAMETER rootParameters[4] = {};

    // CBV (b0) - 定数バッファ
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    // SRV テーブル (t0) mono
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRange[0];
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

    // UAV テーブル (t1) color
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange[1];
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

    // SRV テーブル (u0)
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[3].DescriptorTable.pDescriptorRanges = &descriptorRange[2];
    rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);

    // Compute用なのでサンプラーは不要
    descriptionRootSignature.pStaticSamplers = nullptr;
    descriptionRootSignature.NumStaticSamplers = 0;

    // シリアライズしてバイナリする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));
}

void ColorMask::CreateConstantBuffer()
{
    constantBuffer_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(ColorMaskData));
    constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&constantBufferMap_));

    constantBufferMap_->monoTexUVTransform = Matrix4x4::Identity();
    constantBufferMap_->colorTexUVTransform = Matrix4x4::Identity();
}
