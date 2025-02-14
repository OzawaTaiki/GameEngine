#include "RenderTexture.h"

RenderTexture::RenderTexture():
    descriptorHeap_(nullptr), 
    renderTextureResource_(nullptr),
    clearValue_{ 1.0f,0.0f,0.0f,1.0f },
    srvIndex_(0),
    rtvIndex_(0)
{
}

void RenderTexture::Initialize(uint32_t _width, uint32_t _height, DXGI_FORMAT _format, const Vector4& _clearColor)
{
    descriptorHeap_ = DXCommon::GetInstance()->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, true);
    renderTextureResource_ = DXCommon::GetInstance()->CreateRenderTextureResource(_width, _height, _format, _clearColor);
    SRVManager::GetInstance()->CreateSRVForTextrue2D(srvIndex_, renderTextureResource_.Get(), _format, 1);
    srvIndex_++;
}

