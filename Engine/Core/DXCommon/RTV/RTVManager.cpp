#include "RTVManager.h"
#include <Core/DXCommon/DXCommon.h>

const uint32_t RTVManager::kMaxIndex_ = 8;
 uint32_t RTVManager::winWidth_ = 0;
 uint32_t RTVManager::winHeight_ = 0;

RTVManager* RTVManager::GetInstance()
{
    static RTVManager instance;
    return &instance;
}

void RTVManager::Initialize(size_t _backBufferCount, uint32_t _winWidth, uint32_t _winHeight)
{
    dxcommon_ = DXCommon::GetInstance();
    descriptorHeap_ = dxcommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, kMaxIndex_, false);
    descriptorSize_ = dxcommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    winWidth_ = _winWidth;
    winHeight_ = _winHeight;

    viewport_.TopLeftX = 0;
    viewport_.TopLeftY = 0;
    viewport_.Width = static_cast<FLOAT>(winWidth_);
    viewport_.Height = static_cast<FLOAT>(winHeight_);
    viewport_.MinDepth = 0.0f;
    viewport_.MaxDepth = 1.0f;

    scissorRect_.left = 0;
    scissorRect_.top = 0;
    scissorRect_.right = winWidth_;
    scissorRect_.bottom = winHeight_;

    backBufferCount_ = static_cast<uint32_t>(_backBufferCount);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    
    for (size_t i = 0; i < backBufferCount_; i++)
    {
        auto sr = dxcommon_->GetSwapChainResource(i);
        uint32_t rtvIndex = Allocate();

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetCPURTVDescriptorHandle(rtvIndex);
        dxcommon_->GetDevice()->CreateRenderTargetView(sr, &rtvDesc, rtvHandle);
    }

}

void RTVManager::SetRenderTexture(uint32_t _index, uint32_t _dsvHandle)
{
    auto it = renderTextures_.find(_index);
    if (it == renderTextures_.end())
        assert(false && "not found RenderTexture");

    it->second->SetRenderTexture(_dsvHandle);
}

void RTVManager::SetRenderTexture(std::string _name, uint32_t _dsvHandle)
{
    auto it = textureMap_.find(_name);
    if (it == textureMap_.end())
        return;

    SetRenderTexture(it->second, _dsvHandle);
}

void RTVManager::SetSwapChainRenderTexture(IDXGISwapChain4* _swapChain, uint32_t _dsvHandle)
{
    UINT backBufferIndex = _swapChain->GetCurrentBackBufferIndex();

    auto DXCommon = DXCommon::GetInstance();
    auto commandList = DXCommon->GetCommandList();

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetCPURTVDescriptorHandle(backBufferIndex);

    auto dsvHandle = DXCommon->GetDSVDescriptorHandle(_dsvHandle);

    commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

    commandList->ClearRenderTargetView(rtvHandle, clearColor_, 0, nullptr);

    commandList->RSSetViewports(1, &viewport_);
    commandList->RSSetScissorRects(1, &scissorRect_);


}

uint32_t RTVManager::CreateRenderTexture(std::string _name, uint32_t _width, uint32_t _height, DXGI_FORMAT _format, const Vector4& _clearColor)
{
    uint32_t rtvIndex = Allocate();
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetCPURTVDescriptorHandle(rtvIndex);

    auto it = textureMap_.find(_name);
    if (it != textureMap_.end())
        return it->second;

    auto rtvResource = CreateRenderTextureResource(_width, _height, _format, _clearColor);
    dxcommon_->GetDevice()->CreateRenderTargetView(rtvResource.Get(), nullptr, rtvHandle);
    textureMap_[_name] = rtvIndex;

    renderTextures_[rtvIndex] = std::make_unique<RenderTexture>();
    renderTextures_[rtvIndex]->Initialize(rtvResource.Get(), rtvHandle, _format, rtvIndex);
    renderTextures_[rtvIndex]->SetViewport(viewport_);
    renderTextures_[rtvIndex]->SetScissorRect(scissorRect_);


    return rtvIndex;
}

Microsoft::WRL::ComPtr<ID3D12Resource> RTVManager::CreateRenderTextureResource( uint32_t _width, uint32_t _height, DXGI_FORMAT _format, const Vector4& _clearColor)
{

    Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource = nullptr;

    // テクスチャの設定
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
    resourceDesc.Width = _width; // Textureの幅
    resourceDesc.Height = _height; // Textureの高さ
    resourceDesc.DepthOrArraySize = 1; // 奥行き or 配列Textureの配列数
    resourceDesc.MipLevels = 1; // mipmapの数
    resourceDesc.Format = _format; // フォーマット
    resourceDesc.SampleDesc.Count = 1; // サンプリングカウント、通常は1
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // テクスチャのレイアウト
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // レンダーターゲットとして使う指定

    // ヒープの設定
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = _format; // フォーマット
    clearValue.Color[0] = _clearColor.x; // 赤
    clearValue.Color[1] = _clearColor.y; // 緑
    clearValue.Color[2] = _clearColor.z; // 青
    clearValue.Color[3] = _clearColor.w; // アルファ


    auto device = dxcommon_->GetDevice();

    HRESULT hr= device->CreateCommittedResource(
        &heapProperties,					// Heapの設定
        D3D12_HEAP_FLAG_NONE,				// Heapの特別な設定は特になし。
        &resourceDesc,						// Resourceの設定
		D3D12_RESOURCE_STATE_RENDER_TARGET,
        &clearValue,						// クリア値
        IID_PPV_ARGS(renderTextureResource.GetAddressOf())); // 作成するResourceポインタへのポインタ

	assert(SUCCEEDED(hr));


    return renderTextureResource;
}


uint32_t RTVManager::Allocate()
{
    uint32_t index = useIndex_++;
    if (useIndex_ >= kMaxIndex_)
        throw std::runtime_error("over MaxRTVindex");
    return index;
}

D3D12_CPU_DESCRIPTOR_HANDLE RTVManager::GetCPURTVDescriptorHandle(uint32_t _index)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (descriptorSize_ * _index);
    return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE RTVManager::GetGPURTVDescriptorHandle(uint32_t _index)
{
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (descriptorSize_ * _index);
    return handleGPU;
}
