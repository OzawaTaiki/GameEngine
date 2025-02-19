#include "RTVManager.h"
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>

const uint32_t RTVManager::kMaxRTVIndex_ = 8;
const uint32_t RTVManager::kMaxDSVIndex_ = 8;
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
    rtvDescriptorHeap_ = dxcommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, kMaxRTVIndex_, false);
    dsvDescriptorHeap_ = dxcommon_->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, kMaxDSVIndex_, false);
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
        uint32_t rtvIndex = AllocateRTVIndex();

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetCPURTVDescriptorHandle(rtvIndex);
        dxcommon_->GetDevice()->CreateRenderTargetView(sr, &rtvDesc, rtvHandle);
    }


    uint32_t dsvIndex = AllocateDSVIndex();
    CreateDepthStencilTextureResource(_winWidth, _winHeight, dsvIndex);

}

void RTVManager::DrawRenderTexture(uint32_t _index)
{

    auto it = renderTargets_.find(_index);
    if (it == renderTargets_.end())
        assert(false && "not found RenderTarget");

    auto psoManager = PSOManager::GetInstance();

    psoManager->SetPipeLineStateObject(PSOFlags::Type_OffScreen);
    psoManager->SetRootSignature(PSOFlags::Type_OffScreen);


    it->second->Draw();

}

void RTVManager::DrawRenderTexture(const std::string& _name)
{
    auto it = textureMap_.find(_name);
    if (it == textureMap_.end())
        assert(false && "not found RenderTarget");

    DrawRenderTexture(it->second);
}

void RTVManager::SetRenderTexture(uint32_t _index)
{
    auto it = renderTargets_.find(_index);
    if (it == renderTargets_.end())
        assert(false && "not found RenderTarget");

    it->second->SetRenderTexture();
}

void RTVManager::SetRenderTexture(std::string _name)
{
    auto it = textureMap_.find(_name);
    if (it == textureMap_.end())
        return;

    SetRenderTexture(it->second);
}

void RTVManager::SetDepthStencil(uint32_t _index)
{
    auto it = renderTargets_.find(_index);
    if (it == renderTargets_.end())
        assert(false && "not found RenderTarget");
    it->second->SetDepthStencil();
}

void RTVManager::SetDepthStencil(std::string _name)
{
    auto it = textureMap_.find(_name);
    if (it == textureMap_.end())
        return;
    SetDepthStencil(it->second);
}

void RTVManager::SetSwapChainRenderTexture(IDXGISwapChain4* _swapChain)
{
    UINT backBufferIndex = _swapChain->GetCurrentBackBufferIndex();

    auto DXCommon = DXCommon::GetInstance();
    auto commandList = DXCommon->GetCommandList();

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetCPURTVDescriptorHandle(backBufferIndex);

    auto dsvHandle = GetCPUDSVDescriptorHandle(0);

    commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

    commandList->ClearRenderTargetView(rtvHandle, clearColor_, 0, nullptr);

    commandList->RSSetViewports(1, &viewport_);
    commandList->RSSetScissorRects(1, &scissorRect_);
}

uint32_t RTVManager::CreateRenderTarget(std::string _name, uint32_t _width, uint32_t _height, DXGI_FORMAT _colorFormat, const Vector4& _clearColor, bool _createDSV)
{
    uint32_t rtvIndex = AllocateRTVIndex();

    auto it = textureMap_.find(_name);
    if (it != textureMap_.end())
        return it->second;

    auto rtvResource = CreateRenderTextureResource(_width, _height, rtvIndex, _colorFormat, _clearColor);


    textureMap_[_name] = rtvIndex;

    renderTargets_[rtvIndex] = std::make_unique<RenderTarget>();
    renderTargets_[rtvIndex]->Initialize(rtvResource, GetCPURTVDescriptorHandle(rtvIndex), _colorFormat, _width, _height);
    renderTargets_[rtvIndex]->SetViewport(viewport_);
    renderTargets_[rtvIndex]->SetScissorRect(scissorRect_);
    renderTargets_[rtvIndex]->SetClearColor(_clearColor);

    uint32_t dsvIndex = 0;

    if (_createDSV)
    {
        dsvIndex = AllocateDSVIndex();
        CreateDepthStencilTextureResource(_width, _height, dsvIndex);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetCPUDSVDescriptorHandle(dsvIndex);
    renderTargets_[rtvIndex]->SetDepthStencilResource(dsvHandle);


    return rtvIndex;
}


Microsoft::WRL::ComPtr<ID3D12Resource> RTVManager::CreateRenderTextureResource( uint32_t _width, uint32_t _height, uint32_t _rtvIndex, DXGI_FORMAT _format, const Vector4& _clearColor)
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


    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;         //出力結果をSRGBに変換して書き込む
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;    //2dテクスチャとして書き込む

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetCPURTVDescriptorHandle(_rtvIndex);
    dxcommon_->GetDevice()->CreateRenderTargetView(renderTextureResource.Get(), &rtvDesc, rtvHandle);

    return renderTextureResource;
}

void RTVManager::CreateDepthStencilTextureResource(uint32_t _width, uint32_t _height, uint32_t _dsvIndex)
{
    auto device = dxcommon_->GetDevice();

    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = nullptr;


    // 生成するResourceの設定
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width = _width; // Textureの幅
    resourceDesc.Height = _height; // Textureの高さ
    resourceDesc.MipLevels = 1; // mipmapの数
    resourceDesc.DepthOrArraySize = 1; // 奥行き or 配列Textureの配列数
    resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能なフォーマット
    resourceDesc.SampleDesc.Count = 1; // サンプリングカウント、通常は1
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う指定
    // 利用するHeapの設定
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

    //震度のクリア設定
    D3D12_CLEAR_VALUE depthClearValue{};
    depthClearValue.DepthStencil.Depth = 1.0f;//1.0f(最大値)でクリア
    depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//フォーマット，Resourceと合わせる

    // Resourceの生成
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties, // Heapの設定
        D3D12_HEAP_FLAG_NONE, // Heapの特別な設定は特になし。
        &resourceDesc, // Resourceの設定
        D3D12_RESOURCE_STATE_DEPTH_WRITE, // 深度値を書き込み状態にしておく
        &depthClearValue, // Clear値の値
        IID_PPV_ARGS(depthStencilResource.GetAddressOf())); // 作成するResourceポインタへのポインタ

    assert(SUCCEEDED(hr));

    auto dsvHandle = GetCPUDSVDescriptorHandle(_dsvIndex);

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//基本的にはresourceに合わせる
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2dTexture
    //DSVHeapの先頭にDSVを作る
    device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvHandle);

    dsvResources_.emplace_back(depthStencilResource);


}


uint32_t RTVManager::AllocateRTVIndex()
{
    uint32_t index = useIndexForRTV_++;
    if (useIndexForRTV_ >= kMaxRTVIndex_)
        throw std::runtime_error("over MaxRTVindex");
    return index;
}

uint32_t RTVManager::AllocateDSVIndex()
{
    uint32_t index = useIndexForDSV_++;
    if (useIndexForDSV_ >= kMaxDSVIndex_)
        throw std::runtime_error("over MaxDSVindex");
    return index;
}

D3D12_CPU_DESCRIPTOR_HANDLE RTVManager::GetCPUDSVDescriptorHandle(uint32_t _index)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (descriptorSize_ * _index);
    return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE RTVManager::GetGPUDSVDescriptorHandle(uint32_t _index)
{
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = dsvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (descriptorSize_ * _index);
    return handleGPU;
}

D3D12_CPU_DESCRIPTOR_HANDLE RTVManager::GetCPURTVDescriptorHandle(uint32_t _index)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (descriptorSize_ * _index);
    return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE RTVManager::GetGPURTVDescriptorHandle(uint32_t _index)
{
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = rtvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (descriptorSize_ * _index);
    return handleGPU;
}
