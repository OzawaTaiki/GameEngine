#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <vector>

class RootSignatureBuilder {
public:
    RootSignatureBuilder();

    // RootParameter追加
    RootSignatureBuilder& AddCBV(UINT _shaderRegister, UINT _registerSpace = 0, D3D12_SHADER_VISIBILITY _visibility = D3D12_SHADER_VISIBILITY_ALL);
    RootSignatureBuilder& AddSRV(UINT _shaderRegister, UINT _registerSpace = 0, D3D12_SHADER_VISIBILITY _visibility = D3D12_SHADER_VISIBILITY_ALL);
    RootSignatureBuilder& AddUAV(UINT _shaderRegister, UINT _registerSpace = 0, D3D12_SHADER_VISIBILITY _visibility = D3D12_SHADER_VISIBILITY_ALL);

    // DescriptorTable追加
    RootSignatureBuilder& AddDescriptorTable(
        const std::vector<D3D12_DESCRIPTOR_RANGE>& _ranges,
        D3D12_SHADER_VISIBILITY _visibility = D3D12_SHADER_VISIBILITY_ALL
    );

    // 簡易版DescriptorTable
    RootSignatureBuilder& AddSRVTable(UINT _numDescriptors, UINT _baseRegister, D3D12_SHADER_VISIBILITY _visibility = D3D12_SHADER_VISIBILITY_PIXEL);

    // StaticSampler追加
    RootSignatureBuilder& AddStaticSampler(
        UINT _shaderRegister,
        D3D12_FILTER _filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE _addressMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        D3D12_SHADER_VISIBILITY _visibility = D3D12_SHADER_VISIBILITY_PIXEL
    );

    // Flags設定
    RootSignatureBuilder& SetFlags(D3D12_ROOT_SIGNATURE_FLAGS _flags);
    RootSignatureBuilder& AllowInputLayout();
    RootSignatureBuilder& DenyVS();
    RootSignatureBuilder& DenyPS();

    // 生成
    Microsoft::WRL::ComPtr<ID3D12RootSignature> Build();

private:
    std::vector<D3D12_ROOT_PARAMETER> rootParameters_;
    std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers_;
    std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> descriptorRanges_; // Table用
    D3D12_ROOT_SIGNATURE_FLAGS flags_;
};
