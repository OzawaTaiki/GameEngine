#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <vector>


class RootSignatureBuilder
{
public:
    RootSignatureBuilder();

    // RootParameter追加
    RootSignatureBuilder& AddCBV(UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
    RootSignatureBuilder& AddSRV(UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
    RootSignatureBuilder& AddUAV(UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);

    // DescriptorTable追加
    RootSignatureBuilder& AddDescriptorTable(
        const std::vector<D3D12_DESCRIPTOR_RANGE>& ranges,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL
    );

    // 簡易版DescriptorTable
    RootSignatureBuilder& AddSRVTable(UINT numDescriptors, UINT baseRegister, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_PIXEL);


    // StaticSampler追加
    RootSignatureBuilder& AddDefaultStaticSampler(
        UINT shaderRegister,
        D3D12_FILTER filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE addressMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_PIXEL
    );

    // 深度比較用StaticSampler追加
    RootSignatureBuilder& AddDepthComparisonStaticSampler(
        UINT shaderRegister,
        D3D12_FILTER filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE addressMode = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_PIXEL
    );


    // 生成
    Microsoft::WRL::ComPtr<ID3D12RootSignature> Build();

private:
    std::vector<D3D12_ROOT_PARAMETER> rootParameters_;
    std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers_;
    std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> descriptorRanges_; // Table用
    D3D12_ROOT_SIGNATURE_FLAGS flags_;
};