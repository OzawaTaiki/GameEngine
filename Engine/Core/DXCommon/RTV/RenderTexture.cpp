#include "RenderTexture.h"


RenderTarget::RenderTarget() :
    renderTextureResource_(nullptr),
    srvIndex_(0),
    rtvHandle_{},
    viewport_{},
    scissorRect_{}
{
    srvIndex_ = SRVManager::GetInstance()->Allocate();

}

void RenderTarget::Initialize(Microsoft::WRL::ComPtr<ID3D12Resource> _resource, D3D12_CPU_DESCRIPTOR_HANDLE _rtvHandle, DXGI_FORMAT _format, uint32_t _width, uint32_t _height)
{
    renderTextureResource_ = _resource;
    rtvHandle_ = _rtvHandle;

    width_ = _width;
    height_ = _height;

    auto srvManager = SRVManager::GetInstance();
    srvManager->CreateSRVForRenderTexture(srvIndex_, renderTextureResource_.Get(), _format);
}


void RenderTarget::SetRenderTexture() const
{
    auto DXCommon = DXCommon::GetInstance();
    auto commandList = DXCommon->GetCommandList();

    commandList->OMSetRenderTargets(1, &rtvHandle_, false, &dsvHandle_);
    commandList->ClearDepthStencilView(dsvHandle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    commandList->ClearRenderTargetView(rtvHandle_, clearValue_, 0, nullptr);

    D3D12_VIEWPORT viewport{};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<FLOAT>(width_);
    viewport.Height = static_cast<FLOAT>(height_);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D12_RECT scissorRect{};
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = width_;
    scissorRect.bottom = height_;

    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

}

void RenderTarget::SetDepthStencil() const
{
    auto DXCommon = DXCommon::GetInstance();
    auto commandList = DXCommon->GetCommandList();
    commandList->OMSetRenderTargets(1, &rtvHandle_, false, &dsvHandle_);

    commandList->ClearDepthStencilView(dsvHandle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    commandList->ClearRenderTargetView(rtvHandle_, clearValue_, 0, nullptr);

    D3D12_VIEWPORT viewport{};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<FLOAT>(width_);
    viewport.Height = static_cast<FLOAT>(height_);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D12_RECT scissorRect{};
    scissorRect.left = 0;
    scissorRect.top = 0;
    scissorRect.right = width_;
    scissorRect.bottom = height_;

    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);
}

void RenderTarget::Draw() const
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
