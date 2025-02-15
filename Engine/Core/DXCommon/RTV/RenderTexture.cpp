#include "RenderTexture.h"


RenderTexture::RenderTexture() :
    renderTextureResource_(nullptr),
    rtvIndex_(0),
    srvIndex_(0),
    rtvHandle_{},
    viewport_{},
    scissorRect_{}
{
    srvIndex_ = SRVManager::GetInstance()->Allocate();

}

void RenderTexture::Initialize(ID3D12Resource* _resource, D3D12_CPU_DESCRIPTOR_HANDLE _rtvHandle, DXGI_FORMAT _format, uint32_t _rtvIndex)
{
    renderTextureResource_ = _resource;
    rtvHandle_ = _rtvHandle;
    rtvIndex_ = _rtvIndex;

    auto srvManager = SRVManager::GetInstance();
    srvManager->CreateSRVForRenderTexture(srvIndex_, renderTextureResource_.Get(), _format);
}


void RenderTexture::SetRenderTexture(uint32_t _dsvHandle) const
{
    auto DXCommon = DXCommon::GetInstance();
    auto commandList = DXCommon->GetCommandList();

    auto dsvHandle = DXCommon->GetDSVDescriptorHandle(_dsvHandle);

    commandList->OMSetRenderTargets(1, &rtvHandle_, false, &dsvHandle);
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    commandList->ClearRenderTargetView(rtvHandle_, clearValue_, 0, nullptr);

    commandList->RSSetViewports(1, &viewport_);
    commandList->RSSetScissorRects(1, &scissorRect_);

}

void RenderTexture::Draw() const
{
    auto DXCommon = DXCommon::GetInstance();
    auto commandList = DXCommon->GetCommandList();
    auto srvManager = SRVManager::GetInstance();

    D3D12_RESOURCE_BARRIER barrier_{};
    barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier_.Transition.pResource = renderTextureResource_.Get();
    barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    commandList->ResourceBarrier(1, &barrier_);


    auto srvHandle = srvManager->GetGPUSRVDescriptorHandle(srvIndex_);

    commandList->SetGraphicsRootDescriptorTable(0, srvHandle);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->DrawInstanced(3, 1, 0, 0);

    barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    commandList->ResourceBarrier(1, &barrier_);
}

