#pragma once

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Math/Vector/Vector4.h>

#include <d3d12.h>
#include <wrl.h>

#include <cstdint>


class RenderTarget
{
public:

    RenderTarget();
    ~RenderTarget() = default;

    void Initialize(Microsoft::WRL::ComPtr<ID3D12Resource> _resource, D3D12_CPU_DESCRIPTOR_HANDLE _rtvHandle, DXGI_FORMAT _format, uint32_t _width, uint32_t _height);

    void SetViewport(D3D12_VIEWPORT _viewport) { viewport_ = _viewport; }
    void SetScissorRect(D3D12_RECT _scissorRect) { scissorRect_ = _scissorRect; }
    void SetDepthStencilResource(D3D12_CPU_DESCRIPTOR_HANDLE _dsvHandle) { dsvHandle_ = _dsvHandle; }
    void SetDepthStencilResource(ID3D12Resource* _dsvResource);


    uint32_t GetSRVIndex() const { return srvIndexofRTV_; }

    void SetClearColor(float _r, float _g, float _b, float _a)  { clearValue_[0] = _r;          clearValue_[1] = _g;        clearValue_[2] = _b;        clearValue_[3] = _a;        }
    void SetClearColor(float _color[4])                         { clearValue_[0] = _color[0];   clearValue_[1] = _color[1]; clearValue_[2] = _color[2]; clearValue_[3] = _color[3]; }
    void SetClearColor(const Vector4& _color)                   { clearValue_[0] = _color.x ;   clearValue_[1] = _color.y;  clearValue_[2] = _color.z;  clearValue_[3] = _color.w;  }

    void SetRenderTexture();
    void SetDepthStencil();

    void ChangeRTVState( D3D12_RESOURCE_STATES _after);
    void ChangeDSVState( D3D12_RESOURCE_STATES _after);

    void ChangeRTVState(ID3D12GraphicsCommandList* _cmdList, D3D12_RESOURCE_STATES _after);
    void ChangeDSVState(ID3D12GraphicsCommandList* _cmdList, D3D12_RESOURCE_STATES _after);

    void QueueCommandDSVtoSRV(uint32_t _index);
    void QueueCommandRTVtoSRV(uint32_t _index);

    uint32_t GetSRVindexofRTV() const { return srvIndexofRTV_; }
    uint32_t GetSRVindexofDSV() const { return srvIndexofDSV_; }

    D3D12_RESOURCE_STATES GetRTVCurrentState() const { return RTVCurrentState_; }
    D3D12_RESOURCE_STATES GetDSVCurrentState() const { return DSVCurrentState_; }

    void Draw();

    ID3D12Resource* GetRTVResource() const { return renderTextureResource_.Get(); }
    ID3D12Resource* GetDSVResource() const { return dsvResource_; }



    void Clear(ID3D12GraphicsCommandList* _cmdList);

private:



    D3D12_RESOURCE_STATES RTVCurrentState_ = D3D12_RESOURCE_STATE_RENDER_TARGET;
    D3D12_RESOURCE_STATES DSVCurrentState_ = D3D12_RESOURCE_STATE_DEPTH_WRITE;


    Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource_;
    ID3D12Resource* dsvResource_ = nullptr;
    float clearValue_[4] = { 1.0f,0.0f,0.0f,1.0f };

    // DSVの状態がSRVか
    bool isDSVinSRVState = false;
    bool isRTVinSRVState = false;

    uint32_t srvIndexofRTV_ = 0;
    uint32_t srvIndexofDSV_ = 0;

    uint32_t width_ = 0;
    uint32_t height_ = 0;

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_;


    D3D12_VIEWPORT viewport_{};
    D3D12_RECT scissorRect_{};

};
