#pragma once

#include "Vector3.h"
#include "Vector4.h"

#include <cstdint>
#include <wrl.h>
#include <d3d12.h>
class PointLight
{
public:

    PointLight() = default;
    ~PointLight() = default;

    void Initialize();
    void Update();
    void Draw();

    void TransferData();

    ID3D12Resource* GetResource() { return resource_.Get(); }

    void SetColor(const Vector4& _color) { color_ = _color; }
    void SetPosition(const Vector3& _position) { position_ = _position; }
    void SetIntensity(float _intensity) { intendity_ = _intensity; }
    void SetRadius(float _radius) { radius_ = _radius; }
    void SetDecay(float _decay) { decay_ = _decay; }

    void EnableHalfLambert() { useHalfLambert_ = true; }
    void DisEnableHalfLambert() { useHalfLambert_ = false; }
private:

    Vector4 color_;
    Vector3 position_;
    float intendity_;
    float radius_;
    float decay_;
    bool useHalfLambert_;


    struct ConstantBufferData
    {
        Vector4 color;		//ライトの色
        Vector3 position_;	//ライトの向き
        float intensity;	//輝度
        float radius;	// ライトの影響半径
        float decay;	// 減衰率
        uint32_t isHalf;
        float pad;
    };

    Microsoft::WRL::ComPtr<ID3D12Resource> resource_ = nullptr;
    ConstantBufferData* constMap_ = nullptr;

};