#include "RTVManager.h"
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>

const uint32_t RTVManager::kMaxRTVIndex_ = 64;
const uint32_t RTVManager::kMaxDSVIndex_ = 64;
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
    rtvDescriptorSize_ = dxcommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    dsvDescriptorSize_ = dxcommon_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

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

    it->second->Draw();

}

void RTVManager::ClearAllRenderTarget()
{
    auto DXCommon = DXCommon::GetInstance();
    auto commandList = DXCommon->GetCommandList();
    for (auto& rt : renderTargets_)
    {
        rt.second->Clear(commandList);
    }
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

    //commandList->ClearRenderTargetView(rtvHandle, clearColor_, 0, nullptr);

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
        renderTargets_[rtvIndex]->SetDepthStencilResource(dsvResources_[dsvIndex].Get());
    }

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetCPUDSVDescriptorHandle(dsvIndex);
    renderTargets_[rtvIndex]->SetDepthStencilResource(dsvHandle);


    return rtvIndex;
}


uint32_t RTVManager::CreateCubemapRenderTarget(std::string _name, uint32_t _width, uint32_t _height, DXGI_FORMAT _colorFormat, const Vector4& _clearColor, bool _createDSV)
{
    // 既存のキューブマップがあるか確認
    auto it = textureMap_.find(_name);
    if (it != textureMap_.end())
        return it->second;

    // レンダーターゲットインデックスを確保
    uint32_t rtvIndex = AllocateRTVIndex();

    // キューブマップリソースを作成
    auto cubemapResource = CreateCubemapResource(_width, _height, rtvIndex, _colorFormat, _clearColor);
    if (!cubemapResource) {
        return 0; // リソース作成失敗
    }

    // キューブマップ全体に対するRTVを作成
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = _colorFormat;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
    rtvDesc.Texture2DArray.MipSlice = 0;
    rtvDesc.Texture2DArray.FirstArraySlice = 0;  // 最初の面から
    rtvDesc.Texture2DArray.ArraySize = 6;        // 全6面

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetCPURTVDescriptorHandle(rtvIndex);
    dxcommon_->GetDevice()->CreateRenderTargetView(
        cubemapResource.Get(),
        &rtvDesc,
        rtvHandle);

    // キューブマップ用のSRVを作成
    uint32_t srvIndex = SRVManager::GetInstance()->Allocate();
    SRVManager::GetInstance()->CreateSRVForCubemap(srvIndex, cubemapResource.Get(), _colorFormat);

    // キューブマップデータを保存
    CubemapData cubemapData;
    cubemapData.resource = cubemapResource;
    cubemapData.srvIndex = srvIndex;
    cubemaps_[rtvIndex] = cubemapData;

    // 名前とハンドルのマッピング
    textureMap_[_name] = rtvIndex;

    // RenderTargetオブジェクトの作成と初期化
    renderTargets_[rtvIndex] = std::make_unique<RenderTarget>();
    renderTargets_[rtvIndex]->Initialize(
        cubemapResource,
        rtvHandle,
        _colorFormat,
        _width,
        _height);
    renderTargets_[rtvIndex]->SetViewport(viewport_);
    renderTargets_[rtvIndex]->SetScissorRect(scissorRect_);
    renderTargets_[rtvIndex]->SetClearColor(_clearColor);

    // 深度バッファを作成（必要な場合）
    uint32_t dsvIndex = 0;
    if (_createDSV) {
        dsvIndex = AllocateDSVIndex();

        // キューブマップ用の深度バッファを作成
        D3D12_RESOURCE_DESC depthDesc{};
        depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        depthDesc.Width = _width;
        depthDesc.Height = _height;
        depthDesc.DepthOrArraySize = 6;  // 6面用
        depthDesc.MipLevels = 1;
        depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.SampleDesc.Quality = 0;
        depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        // ヒーププロパティ
        D3D12_HEAP_PROPERTIES heapProps{};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

        // 深度クリア値
        D3D12_CLEAR_VALUE depthClearValue{};
        depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        depthClearValue.DepthStencil.Depth = 1.0f;

        // 深度リソース作成
        Microsoft::WRL::ComPtr<ID3D12Resource> depthResource;
        HRESULT hr = dxcommon_->GetDevice()->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &depthDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depthClearValue,
            IID_PPV_ARGS(depthResource.GetAddressOf()));

        if (SUCCEEDED(hr)) {
            dsvResources_[dsvIndex] = depthResource;

            // 深度ステンシルビューの作成
            D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
            dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            dsvDesc.Texture2DArray.MipSlice = 0;
            dsvDesc.Texture2DArray.FirstArraySlice = 0;
            dsvDesc.Texture2DArray.ArraySize = 6;

            D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetCPUDSVDescriptorHandle(dsvIndex);
            dxcommon_->GetDevice()->CreateDepthStencilView(
                depthResource.Get(),
                &dsvDesc,
                dsvHandle);

            renderTargets_[rtvIndex]->SetDepthStencilResource(depthResource.Get());
            renderTargets_[rtvIndex]->SetDepthStencilResource(dsvHandle);
        }
    }

    return rtvIndex;
}

void RTVManager::QueuePointLightShadowMapToSRV(const std::string& _name, uint32_t _index)
{
    auto it = textureMap_.find(_name);
    if (it == textureMap_.end())
        return;

    uint32_t handle = it->second;
    QueuePointLightShadowMapToSRV(handle, _index);

}

void RTVManager::QueuePointLightShadowMapToSRV(uint32_t _handle, uint32_t _index)
{
    // キューブマップの存在確認
    auto it = cubemaps_.find(_handle);
    if (it == cubemaps_.end()) {
        return; // シャドウマップが存在しない
    }

    auto& cubemapData = it->second;
    auto renderTarget = renderTargets_[_handle].get();

    if (!renderTarget || !cubemapData.resource) {
        return; // レンダーターゲットまたはリソースが存在しない
    }

    //renderTarget->QueueCommandDSVtoSRV(_index);

    auto DXCommon = DXCommon::GetInstance();
    auto commandList = DXCommon->GetCommandList();
    auto srvManager = SRVManager::GetInstance();

    renderTarget->ChangeRTVState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    //// リソース状態の遷移（必要な場合）
    //if (renderTarget->GetDSVCurrentState() != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
    //{
    //    D3D12_RESOURCE_BARRIER barrier = {};
    //    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    //    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    //    barrier.Transition.pResource = renderTarget->GetDSVResource();
    //    barrier.Transition.StateBefore = renderTarget->GetDSVCurrentState();
    //    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    //    commandList->ResourceBarrier(1, &barrier);

    //    // レンダーターゲットの状態を更新
    //    renderTarget->ChangeDSVState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    //}

    //// SRVハンドルを取得してセット
    auto srvHandle = srvManager->GetGPUSRVDescriptorHandle(cubemapData.srvIndex);
    commandList->SetGraphicsRootDescriptorTable(_index, srvHandle);
}

uint32_t RTVManager::CreateComputeOutputTexture(const std::string& _name, uint32_t _width, uint32_t _height, DXGI_FORMAT _format, const Vector4& _clearColor)
{
    uint32_t rtvIndex = AllocateRTVIndex();

    // 既に存在するかチェック
    auto it = textureMap_.find(_name);
    if (it != textureMap_.end())
        return it->second;

    // UAVフラグも追加したリソース作成
    auto rtvResource = CreateRenderTextureResourceWithUAV(_width, _height, rtvIndex, _format,_clearColor);

    textureMap_[_name] = rtvIndex;

    renderTargets_[rtvIndex] = std::make_unique<RenderTarget>();
    renderTargets_[rtvIndex]->Initialize(rtvResource, GetCPURTVDescriptorHandle(rtvIndex), _format, _width, _height);
    renderTargets_[rtvIndex]->SetViewport(viewport_);
    renderTargets_[rtvIndex]->SetScissorRect(scissorRect_);
    renderTargets_[rtvIndex]->SetClearColor(_clearColor);

    uint32_t dsvIndex = AllocateDSVIndex();
    CreateDepthStencilTextureResource(_width, _height, dsvIndex);
    renderTargets_[rtvIndex]->SetDepthStencilResource(dsvResources_[dsvIndex].Get());

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetCPUDSVDescriptorHandle(dsvIndex);
    renderTargets_[rtvIndex]->SetDepthStencilResource(dsvHandle);

    return rtvIndex;
}

void RTVManager::TransitionForCompute(uint32_t _rtvIndex)
{
    auto it = renderTargets_.find(_rtvIndex);
    if (it == renderTargets_.end())
    {
        assert(false && "RenderTarget not found");
        return;
    }

    it->second->ChangeRTVState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

void RTVManager::TransitionForRender(uint32_t _rtvIndex)
{
    auto it = renderTargets_.find(_rtvIndex);
    if (it == renderTargets_.end())
    {
        assert(false && "RenderTarget not found");
        return;
    }

    it->second->ChangeRTVState(D3D12_RESOURCE_STATE_RENDER_TARGET);
}



void RTVManager::SetCubemapRenderTexture(uint32_t _handle)
{
    auto it = cubemaps_.find(_handle);
    if (it == cubemaps_.end())
    {
        // Not a cubemap handle, treat as regular render texture
        SetRenderTexture(_handle);
        return;
    }

    // Set render target to the cubemap (all 6 faces)
    auto renderTarget = renderTargets_[_handle].get();
    if (renderTarget)
    {
        renderTarget->SetRenderTexture();
    }
}

ID3D12Resource* RTVManager::GetCubemapResource(uint32_t _handle) const
{
    auto it = cubemaps_.find(_handle);
    if (it != cubemaps_.end())
    {
        return it->second.resource.Get();
    }
    return nullptr;
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
    rtvDesc.Format = _format;         //出力結果をSRGBに変換して書き込む
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
    resourceDesc.Format = DXGI_FORMAT_R32_TYPELESS; // DepthStencilとして利用可能なフォーマット
    resourceDesc.SampleDesc.Count = 1; // サンプリングカウント、通常は1
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う指定
    // 利用するHeapの設定
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

    //震度のクリア設定
    D3D12_CLEAR_VALUE depthClearValue{};
    depthClearValue.DepthStencil.Depth = 1.0f;//1.0f(最大値)でクリア
    depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;//フォーマット，Resourceと合わせる

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
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;//基本的にはresourceに合わせる
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2dTexture
    //DSVHeapの先頭にDSVを作る
    device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvHandle);
    dsvResources_[_dsvIndex] = depthStencilResource;


}

Microsoft::WRL::ComPtr<ID3D12Resource> RTVManager::CreateRenderTextureResourceWithUAV(uint32_t _width, uint32_t _height, uint32_t _rtvIndex, DXGI_FORMAT _format, const Vector4& _clearColor)
{
    Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource = nullptr;

    // テクスチャの設定（既存とほぼ同じだが、UAVフラグを追加）
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Width = _width;
    resourceDesc.Height = _height;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = _format;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    // RTV + UAV の両方のフラグを設定
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    // ヒープの設定
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

    // クリア値（デフォルト）
    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = _format;
    clearValue.Color[0] = _clearColor.x;
    clearValue.Color[1] = _clearColor.y;
    clearValue.Color[2] = _clearColor.z;
    clearValue.Color[3] = _clearColor.w;

    auto device = dxcommon_->GetDevice();

    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        &clearValue,
        IID_PPV_ARGS(renderTextureResource.GetAddressOf()));

    assert(SUCCEEDED(hr) && "Failed to create compute output texture");

    // RTV作成
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = _format;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetCPURTVDescriptorHandle(_rtvIndex);
    dxcommon_->GetDevice()->CreateRenderTargetView(renderTextureResource.Get(), &rtvDesc, rtvHandle);

    return renderTextureResource;

}

Microsoft::WRL::ComPtr<ID3D12Resource> RTVManager::CreateCubemapResource(uint32_t _width, uint32_t _height, uint32_t _rtvIndex, DXGI_FORMAT _format, const Vector4& _clearColor)
{
    Microsoft::WRL::ComPtr<ID3D12Resource> cubemapResource = nullptr;

    // Texture description for a cubemap (2D texture array with 6 slices)
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Width = _width;
    resourceDesc.Height = _height;
    resourceDesc.DepthOrArraySize = 6;  // 6 faces for a cubemap
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = _format;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    // Heap properties
    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

    // Clear value
    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = _format;
    clearValue.Color[0] = _clearColor.x;
    clearValue.Color[1] = _clearColor.y;
    clearValue.Color[2] = _clearColor.z;
    clearValue.Color[3] = _clearColor.w;

    // Create the resource
    auto device = dxcommon_->GetDevice();
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        &clearValue,
        IID_PPV_ARGS(cubemapResource.GetAddressOf()));

    assert(SUCCEEDED(hr));

    return cubemapResource;
}


uint32_t RTVManager::AllocateRTVIndex()
{
    assert(useIndexForRTV_ < kMaxRTVIndex_ && "RTV index overflow");
    uint32_t index = useIndexForRTV_++;
    return index;
}

uint32_t RTVManager::AllocateDSVIndex()
{
    assert(useIndexForDSV_ < kMaxDSVIndex_ && "DSV index overflow");
    uint32_t index = useIndexForDSV_++;
    return index;
}

D3D12_CPU_DESCRIPTOR_HANDLE RTVManager::GetCPUDSVDescriptorHandle(uint32_t _index)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (dsvDescriptorSize_  * _index);
    return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE RTVManager::GetGPUDSVDescriptorHandle(uint32_t _index)
{
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = dsvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (dsvDescriptorSize_ * _index);
    return handleGPU;
}

D3D12_CPU_DESCRIPTOR_HANDLE RTVManager::GetCPURTVDescriptorHandle(uint32_t _index)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += (rtvDescriptorSize_ * _index);
    return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE RTVManager::GetGPURTVDescriptorHandle(uint32_t _index)
{
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = rtvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += (rtvDescriptorSize_ * _index);
    return handleGPU;
}
