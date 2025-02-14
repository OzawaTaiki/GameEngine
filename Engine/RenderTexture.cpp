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

void RenderTexture::PreDraw()
{
    auto DXCommon = DXCommon::GetInstance();
    auto commandList = DXCommon->GetCommandList();
    auto device = DXCommon->GetDevice();


	const uint32_t desriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

//	//描画先とRTVとDSVの設定を行う
//	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = DXCommon->GetCPUDescriptorHandle(dsvDescriptorHeap_.Get(), desriptorSizeDSV, 0);
//    commandList->OMSetRenderTargets(1, &RTVHandles_[2], false, &dsvHandle);
//
//	//float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
//    commandList->ClearRenderTargetView(RTVHandles_[2], rtClearValue_, 0, nullptr);
//
//	//指定した深度で画面をクリアする
//    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
//
//    commandList->RSSetViewports(1, &viewport_);                      // Viewportを設定
//    commandList->RSSetScissorRects(1, &scissorRect_);                      // Scissorを設定
//
//    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//}

