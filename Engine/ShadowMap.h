#pragma once



#include <d3d12.h>
#include <wrl.h>

class ShadowMap
{
public:
    ShadowMap();
    ~ShadowMap();

    void Initialize();

private:

    Microsoft::WRL::ComPtr<ID3D12Resource> shadowMapResource_;

};