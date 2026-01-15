#include "RenderTexture.h"



namespace Engine {

RenderTarget::RenderTarget() :
    renderTextureResource_(nullptr),
    srvIndexofRTV_(0),
    srvIndexofDSV_(0),
    rtvHandle_{},
    viewport_{},
    scissorRect_{}
{
    srvIndexofRTV_ = SRVManager::GetInstance()->Allocate();
}

void RenderTarget::Initialize(Microsoft::WRL::ComPtr<ID3D12Resource> _resource, D3D12_CPU_DESCRIPTOR_HANDLE _rtvHandle, DXGI_FORMAT _format, uint32_t _width, uint32_t _height)
{
    renderTextureResource_ = _resource;
    rtvHandle_ = _rtvHandle;

    width_ = _width;
    height_ = _height;

    auto srvManager = SRVManager::GetInstance();
    srvManager->CreateSRVForRenderTexture(srvIndexofRTV_, renderTextureResource_.Get(), _format);
}

void RenderTarget::SetDepthStencilResource(ID3D12Resource* _dsvResource)
{
    srvIndexofDSV_ = SRVManager::GetInstance()->Allocate();

    dsvResource_ = _dsvResource;

    SRVManager::GetInstance()->CreateSRVForRenderTexture(srvIndexofDSV_, dsvResource_, DXGI_FORMAT_R32_FLOAT);

    //DSVCurrentState_ = D3D12_RESOURCE_STATE_DEPTH_WRITE;

}


void RenderTarget::SetRenderTexture()
{
    auto DXCommon = DXCommon::GetInstance();
    auto commandList = DXCommon->GetCommandList();

    ChangeRTVState(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList->OMSetRenderTargets(1, &rtvHandle_, false, &dsvHandle_);

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

void RenderTarget::SetDepthStencil()
{
    auto DXCommon = DXCommon::GetInstance();
    auto commandList = DXCommon->GetCommandList();
    if (DSVCurrentState_ != D3D12_RESOURCE_STATE_DEPTH_WRITE)
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = dsvResource_;
        barrier.Transition.StateBefore = DSVCurrentState_;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        commandList->ResourceBarrier(1, &barrier);
        DSVCurrentState_ = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    }

    commandList->OMSetRenderTargets(1, &rtvHandle_, false, &dsvHandle_);

    //commandList->ClearDepthStencilView(dsvHandle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    //commandList->ClearRenderTargetView(rtvHandle_, clearValue_, 0, nullptr);

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

void RenderTarget::ChangeRTVState(D3D12_RESOURCE_STATES _after)
{
    auto DXCommon = DXCommon::GetInstance();
    auto commandList = DXCommon->GetCommandList();

    if (RTVCurrentState_ != _after)
    {
        D3D12_RESOURCE_BARRIER barrier_ = {};
        barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier_.Transition.pResource = renderTextureResource_.Get();
        barrier_.Transition.StateBefore = RTVCurrentState_;
        barrier_.Transition.StateAfter = _after;
        barrier_.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        commandList->ResourceBarrier(1, &barrier_);

        RTVCurrentState_ = _after;
    }

}

void RenderTarget::ChangeDSVState( D3D12_RESOURCE_STATES _after)
{
    auto DXCommon = DXCommon::GetInstance();
    auto commandList = DXCommon->GetCommandList();

    if (DSVCurrentState_ != _after)
    {
        D3D12_RESOURCE_BARRIER barrier_ = {};
        barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier_.Transition.pResource = dsvResource_;
        barrier_.Transition.StateBefore = DSVCurrentState_;
        barrier_.Transition.StateAfter = _after;
        barrier_.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;


        commandList->ResourceBarrier(1, &barrier_);
        DSVCurrentState_ = _after;
    }
}

void RenderTarget::ChangeRTVState(ID3D12GraphicsCommandList* _cmdList, D3D12_RESOURCE_STATES _after)
{
    if (RTVCurrentState_ != _after)
    {
        D3D12_RESOURCE_BARRIER barrier_ = {};
        barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier_.Transition.pResource = renderTextureResource_.Get();
        barrier_.Transition.StateBefore = RTVCurrentState_;
        barrier_.Transition.StateAfter = _after;
        barrier_.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        _cmdList->ResourceBarrier(1, &barrier_);
        RTVCurrentState_ = _after;
    }
}

void RenderTarget::ChangeDSVState(ID3D12GraphicsCommandList* _cmdList, D3D12_RESOURCE_STATES _after)
{
    if (DSVCurrentState_ != _after)
    {
        D3D12_RESOURCE_BARRIER barrier_ = {};
        barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier_.Transition.pResource = dsvResource_;
        barrier_.Transition.StateBefore = DSVCurrentState_;
        barrier_.Transition.StateAfter = _after;
        barrier_.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        _cmdList->ResourceBarrier(1, &barrier_);
        DSVCurrentState_ = _after;
    }
}

void RenderTarget::QueueCommandDSVtoSRV(uint32_t _index)
{
    auto DXCommon = DXCommon::GetInstance();
    auto commandList = DXCommon->GetCommandList();
    auto srvManager = SRVManager::GetInstance();

    if (DSVCurrentState_ != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = dsvResource_;
        barrier.Transition.StateBefore = DSVCurrentState_;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        commandList->ResourceBarrier(1, &barrier);

        DSVCurrentState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    }

    auto srvHandle = srvManager->GetGPUSRVDescriptorHandle(srvIndexofDSV_);

    commandList->SetGraphicsRootDescriptorTable(_index, srvHandle);

}

void RenderTarget::QueueCommandRTVtoSRV(uint32_t _index)
{
    auto DXCommon = DXCommon::GetInstance();
    auto commandList = DXCommon->GetCommandList();
    auto srvManager = SRVManager::GetInstance();
    if (RTVCurrentState_!= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = renderTextureResource_.Get();
        barrier.Transition.StateBefore = RTVCurrentState_;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        commandList->ResourceBarrier(1, &barrier);

        RTVCurrentState_ = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    }
    auto srvHandle = srvManager->GetGPUSRVDescriptorHandle(srvIndexofRTV_);
    commandList->SetGraphicsRootDescriptorTable(_index, srvHandle);
}

void RenderTarget::Draw()
{
    auto DXCommon = DXCommon::GetInstance();
    auto commandList = DXCommon->GetCommandList();
    auto srvManager = SRVManager::GetInstance();


    ChangeRTVState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    auto srvHandle = srvManager->GetGPUSRVDescriptorHandle(srvIndexofRTV_);

    commandList->SetGraphicsRootDescriptorTable(0, srvHandle);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList->DrawInstanced(3, 1, 0, 0);

    ChangeRTVState(D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void RenderTarget::Clear(ID3D12GraphicsCommandList* _cmdList)
{
    if (!needClear_)
        return;

    // クリア前に適切な状態に遷移させる
    if (DSVCurrentState_ != D3D12_RESOURCE_STATE_DEPTH_WRITE)
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = dsvResource_;
        barrier.Transition.StateBefore = DSVCurrentState_;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        _cmdList->ResourceBarrier(1, &barrier);
        DSVCurrentState_ = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    }

    if (RTVCurrentState_ != D3D12_RESOURCE_STATE_RENDER_TARGET)
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = renderTextureResource_.Get();
        barrier.Transition.StateBefore = RTVCurrentState_;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        _cmdList->ResourceBarrier(1, &barrier);
        RTVCurrentState_ = D3D12_RESOURCE_STATE_RENDER_TARGET;
    }

    _cmdList->ClearDepthStencilView(dsvHandle_, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    _cmdList->ClearRenderTargetView(rtvHandle_, clearValue_, 0, nullptr);

}

} // namespace Engine
