#pragma once

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Math/Vector/Vector4.h>
#include <d3d12.h>


class RenderTexture
{
public:

    RenderTexture();
    ~RenderTexture() = default;

    void Initialize(uint32_t _width, uint32_t _height, DXGI_FORMAT _format, const Vector4& _clearColor);



private:

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;

    Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource_;
    float clearValue_[4] = { 1.0f,0.0f,0.0f,1.0f };

    uint32_t srvIndex_ = 0;
    uint32_t rtvIndex_ = 0;

};
