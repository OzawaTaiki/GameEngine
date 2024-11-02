#pragma once

#include "Vector3.h"
#include "Vector4.h"

#include <cstdint>
#include <wrl.h>
#include <d3d12.h>

class SpotLight
{
public:

    SpotLight() = default;
    ~SpotLight() = default;

    void Initialize();
    void Update();
    void Draw();

    void TransferData();

    ID3D12Resource* GetResource() { return resource_.Get(); }

    void SetColor(const Vector4& _color) { color_ = _color; }
    void SetPosition(const Vector3& _position) { position_ = _position; }
    void SetDirection(const Vector3& _direction) { position_ = _direction; }
    void SetIntensity(float _intensity) { intensity_ = _intensity; }
    void SetDistance(float _distance) { distance_ = _distance; }
    void SetDecay(float _decay) { decay_ = _decay; }
    void SetCosAngle(float _cos) { cosAngle_ = _cos; }
    void SetFalloutStartAngle(float _falloutStartAngle) { falloutStartAngle_ = _falloutStartAngle; }

    void EnableHalfLambert() { useHalfLambert_ = true; }
    void DisEnableHalfLambert() { useHalfLambert_ = false; }
private:

    Vector4 color_;		
    Vector3 position_;
    Vector3 direction_;
    float   intensity_;	
    float   distance_;	
    float   decay_;	
    float   cosAngle_;
    float   falloutStartAngle_;
    uint32_t useHalfLambert_;


    struct ConstantBufferData
    {
        Vector4 color;		//ライトの色
        Vector3 position;
        float intensity;	//輝度
        Vector3 direction;
        float distance;
        float decay;	// 減衰率
        float cosAngle;
        float falloutStartAngle;
        uint32_t isHalf;
    };

    Microsoft::WRL::ComPtr<ID3D12Resource> resource_ = nullptr;
    ConstantBufferData* constMap_ = nullptr;

};