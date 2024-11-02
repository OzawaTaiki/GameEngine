#pragma once

#include "Vector3.h"
#include "Vector4.h"

#include <cstdint>
#include <wrl.h>
#include <d3d12.h>

class DirectionalLight
{
public:

    DirectionalLight() = default;
    ~DirectionalLight() = default;

    void Initialize();
    void Update();
    void Draw();

    void TransferData();

    ID3D12Resource* GetResource() { return resource_.Get(); }

    void SetColor(const Vector4& _color) { color_ = _color; }
    void SetDirection(const Vector3& _directoin);
    void SetIntensity(float _intensity) { intendity_ = _intensity; }

    void EnableHalfLambert() { useHalfLambert_ = true; }
    void DisEnableHalfLambert() { useHalfLambert_ = false; }

private:

    Vector4 color_;
    Vector3 direction_;
    float intendity_;
    bool useHalfLambert_;


    struct ConstantBufferData
    {
        Vector4 color;		//ライトの色
        Vector3 direction;	//ライトの向き
        float intensity;	//輝度
        uint32_t isHalf;
        float pad[3];
    };

    Microsoft::WRL::ComPtr<ID3D12Resource> resource_ = nullptr;
    ConstantBufferData* constMap_ = nullptr;

};