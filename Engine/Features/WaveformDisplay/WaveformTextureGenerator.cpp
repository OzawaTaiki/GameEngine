#include "WaveformTextureGenerator.h"
#include <dxcapi.h>
#include <Core/DXCommon/DXCommon.h>
#include <Debug/Debug.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>

const uint32_t WaveformTexturesGenerator::textureHeight_ = 256u * 6u; // 1536 6セグメント分
const uint32_t WaveformTexturesGenerator::textureWidth_ = 2048u;

void WaveformTexturesGenerator::Initialize()
{
    // コンスタントバッファーを作成
    // ストラクチャーバッファーを作成
    // CSを作成

    CreateConstantBuffer();

    CreateStructuredBuffer();

    CreateComputePipeline();

}

void WaveformTexturesGenerator::GenerateWaveformTexture(const std::vector<float>& _waveformData, uint32_t _textureHandle)
{
    if (_waveformData.size() != waveformDataCount_)
    {
        Debug::Log("Waveform data size is incorrect");
        return;
    }

    // 波形データをストラクチャバッファにコピー
    memcpy(mappedWaveformData_, _waveformData.data(), sizeof(float) * waveformDataCount_);
    if (computePipeline_ == nullptr)
    {
        CreateComputePipeline();
    }

    auto cmdList = DXCommon::GetInstance()->GetCommandList();

    cmdList->SetPipelineState(computePipeline_.Get());
    cmdList->SetComputeRootSignature(rootSignature_.Get());
    // 定数バッファを設定
    cmdList->SetComputeRootConstantBufferView(0, constantBuffer_->GetGPUVirtualAddress());
    // 波形データのSRVを設定
    cmdList->SetComputeRootDescriptorTable(1, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(srvIndexWaveformData_));
    // テクスチャのUAVを設定
    cmdList->SetComputeRootDescriptorTable(2, SRVManager::GetInstance()->GetGPUSRVDescriptorHandle(_textureHandle));

    // スレッドグループ数計算 (16x16のスレッドグループ)
    uint32_t dispatchX = (textureWidth_ + 15) / 16;
    uint32_t dispatchY = (textureHeight_ + 15) / 16;
    // Dispatch実行
    cmdList->Dispatch(dispatchX, dispatchY, 1);
}

uint32_t WaveformTexturesGenerator::CreateWaveformTexture()
{
    Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource = nullptr;

    // テクスチャの設定（既存とほぼ同じだが、UAVフラグを追加）
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Width = textureWidth_;
    resourceDesc.Height = textureHeight_;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_R8_UNORM;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    // RTV + UAV の両方のフラグを設定
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    // ヒープの設定
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

    auto device = DXCommon::GetInstance()->GetDevice();

    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, // UAVとして使用するため、初期状態をUNORDERED_ACCESSに設定
        nullptr,
        IID_PPV_ARGS(&renderTextureResource));

    assert(SUCCEEDED(hr) && "Failed to create compute output texture");

    uint32_t uavIndex = SRVManager::GetInstance()->Allocate();
    SRVManager::GetInstance()->CreateUAVForTexture2D(uavIndex, renderTextureResource.Get(), DXGI_FORMAT_R8_UNORM);

    textureSRVMap_[uavIndex] = renderTextureResource.Get();

    return uavIndex;
}

void WaveformTexturesGenerator::CreateConstantBuffer()
{
    // 定数バッファの生成
    constantBuffer_ = DXCommon::GetInstance()->CreateBufferResource(sizeof(WaveformParams));

    constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&params_));

    params_->textureWidth = textureWidth_;
    params_->textureHeight = textureHeight_;
    params_->segmentHeght = 256;
}

void WaveformTexturesGenerator::CreateStructuredBuffer()
{
    structuredBufferWaveform_ = DXCommon::GetInstance()->CreateBufferResource(static_cast<uint32_t>(sizeof(float) * waveformDataCount_));
    structuredBufferWaveform_->Map(0, nullptr, reinterpret_cast<void**>(&mappedWaveformData_));

    srvIndexWaveformData_ = SRVManager::GetInstance()->Allocate();

    SRVManager::GetInstance()->CreateSRVForStructureBuffer(
        srvIndexWaveformData_,
        structuredBufferWaveform_.Get(),
        waveformDataCount_,
        sizeof(float));
}

void WaveformTexturesGenerator::CreateComputePipeline()
{
    HRESULT hr = S_FALSE;


    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_DESCRIPTOR_RANGE descriptorRanges[2] = {};
    // 波形データ
    descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[0].NumDescriptors = 1;
    descriptorRanges[0].BaseShaderRegister = 0;
    descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // テクスチャ
    descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRanges[1].NumDescriptors = 1;
    descriptorRanges[1].BaseShaderRegister = 0;
    descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


    D3D12_ROOT_PARAMETER rootParameters[3] = {};

    // コンスタントバッファ
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].Descriptor.RegisterSpace = 0;

    // 波形データ
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

    // 波形テクスチャ
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRanges[1];
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;



    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);


    //シリアライズしてバイナリする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));


    // ComputeShaderの設定
    Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob = nullptr;
    // ComputeShaderのコンパイル
    computeShaderBlob = PSOManager::GetInstance()->ComplieShader(L"Skinning.CS.hlsl", L"cs_6_0");
    assert(computeShaderBlob != nullptr);

    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = rootSignature_.Get();
    computePsoDesc.CS = { computeShaderBlob->GetBufferPointer(), computeShaderBlob->GetBufferSize() };

    hr = DXCommon::GetInstance()->GetDevice()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&computePipeline_));
    assert(SUCCEEDED(hr));
}
