#pragma once

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Math/Vector/Vector4.h>

#include <d3d12.h>
#include <wrl.h>

#include <cstdint>


class RenderTexture
{
public:

    RenderTexture();
    ~RenderTexture() = default;

    void Initialize(ID3D12Resource* _resource, D3D12_CPU_DESCRIPTOR_HANDLE _rtvHandle, DXGI_FORMAT _format, uint32_t _rtvIndex);

    void SetViewport(D3D12_VIEWPORT _viewport) { viewport_ = _viewport; }
    void SetScissorRect(D3D12_RECT _scissorRect) { scissorRect_ = _scissorRect; }

    uint32_t GetRTVIndex() const { return rtvIndex_; }
    uint32_t GetSRVIndex() const { return srvIndex_; }

    void SetClearColor(float _r, float _g, float _b, float _a)  { clearValue_[0] = _r;          clearValue_[1] = _g;        clearValue_[2] = _b;        clearValue_[3] = _a;        }
    void SetClearColor(float _color[4])                         { clearValue_[0] = _color[0];   clearValue_[1] = _color[1]; clearValue_[2] = _color[2]; clearValue_[3] = _color[3]; }
    void SetClearColor(const Vector4& _color)                   { clearValue_[0] = _color.x ;   clearValue_[1] = _color.y;  clearValue_[2] = _color.z;  clearValue_[3] = _color.w;  }

    void SetRenderTexture(uint32_t _dsvHandle = 0) const;



private:

    Microsoft::WRL::ComPtr<ID3D12Resource> renderTextureResource_;
    float clearValue_[4] = { 1.0f,0.0f,0.0f,1.0f };

    uint32_t rtvIndex_ = 0;
    uint32_t srvIndex_ = 0;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_;

    D3D12_VIEWPORT viewport_{};
    D3D12_RECT scissorRect_{};

};
