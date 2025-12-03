#include "RootSignatureBuilder.h"
#include <Debug/Debug.h>
#include <cassert>
#include <Core/DXCommon/DXCommon.h>

RootSignatureBuilder::RootSignatureBuilder()
{
    flags_ = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
}

RootSignatureBuilder& RootSignatureBuilder::AddCBV(UINT shaderRegister,D3D12_SHADER_VISIBILITY visibility)
{
    D3D12_ROOT_PARAMETER param{};

    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    param.Descriptor.ShaderRegister = shaderRegister;
    param.ShaderVisibility = visibility;

    rootParameters_.push_back(param);

    return *this;
}

RootSignatureBuilder& RootSignatureBuilder::AddSRV(UINT shaderRegister,D3D12_SHADER_VISIBILITY visibility)
{
    D3D12_ROOT_PARAMETER param{};

    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    param.Descriptor.ShaderRegister = shaderRegister;
    param.ShaderVisibility = visibility;

    rootParameters_.push_back(param);

    return *this;
}

RootSignatureBuilder& RootSignatureBuilder::AddUAV(UINT shaderRegister,D3D12_SHADER_VISIBILITY visibility)
{
    D3D12_ROOT_PARAMETER param{};

    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
    param.Descriptor.ShaderRegister = shaderRegister;
    param.ShaderVisibility = visibility;
    rootParameters_.push_back(param);

    return *this;
}

RootSignatureBuilder& RootSignatureBuilder::AddDescriptorTable(const std::vector<D3D12_DESCRIPTOR_RANGE>& ranges, D3D12_SHADER_VISIBILITY visibility)
{
    D3D12_ROOT_PARAMETER param{};

    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(ranges.size());
    param.DescriptorTable.pDescriptorRanges = nullptr; // 後で設定
    param.ShaderVisibility = visibility;

    descriptorRanges_.push_back(ranges);
    rootParameters_.push_back(param);
    return *this;
}

RootSignatureBuilder& RootSignatureBuilder::AddSRVTable(UINT numDescriptors, UINT baseRegister, D3D12_SHADER_VISIBILITY visibility)
{
    std::vector<D3D12_DESCRIPTOR_RANGE> ranges;

    D3D12_DESCRIPTOR_RANGE range{};

    range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range.NumDescriptors = numDescriptors;
    range.BaseShaderRegister = baseRegister;
    range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    ranges.push_back(range);
    return AddDescriptorTable(ranges, visibility);
}

RootSignatureBuilder& RootSignatureBuilder::AddDefaultStaticSampler(UINT shaderRegister, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addressMode, D3D12_SHADER_VISIBILITY visibility)
{
    D3D12_STATIC_SAMPLER_DESC sampler{};

    sampler.Filter = filter;
    sampler.AddressU = addressMode;
    sampler.AddressV = addressMode;
    sampler.AddressW = addressMode;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = shaderRegister;
    sampler.ShaderVisibility = visibility;

    staticSamplers_.push_back(sampler);

    return *this;
}

RootSignatureBuilder& RootSignatureBuilder::AddDepthComparisonStaticSampler(UINT shaderRegister, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addressMode, D3D12_SHADER_VISIBILITY visibility)
{
    D3D12_STATIC_SAMPLER_DESC sampler{};

    sampler.Filter = filter;
    sampler.AddressU = addressMode;
    sampler.AddressV = addressMode;
    sampler.AddressW = addressMode;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE; // 影の外は光が当たる
    sampler.ShaderRegister = shaderRegister;
    sampler.ShaderVisibility = visibility;

    staticSamplers_.push_back(sampler);

    return *this;
}


Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignatureBuilder::Build()
{
    DXCommon* dxcommon = DXCommon::GetInstance();
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;

    D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
    rootSigDesc.Flags = flags_;

    // DescriptorTableの範囲を設定
    size_t tableIndex = 0;
    for (size_t i = 0; i < rootParameters_.size(); ++i)
    {
        if (rootParameters_[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
        {
            rootParameters_[i].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(descriptorRanges_[tableIndex].size());
            rootParameters_[i].DescriptorTable.pDescriptorRanges = descriptorRanges_[tableIndex].data();
            tableIndex++;
        }
    }
    rootSigDesc.pParameters = rootParameters_.data();
    rootSigDesc.NumParameters = static_cast<UINT>(rootParameters_.size());
    rootSigDesc.pStaticSamplers = staticSamplers_.data();
    rootSigDesc.NumStaticSamplers = static_cast<UINT>(staticSamplers_.size());

    //シリアライズしてバイナリする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);

    if (FAILED(hr))
    {
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }

    hr = dxcommon->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
    assert(SUCCEEDED(hr));

    return std::move(rootSignature);
}
