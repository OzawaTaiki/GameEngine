#pragma once

#include <Features/Light/Light.h>

#include <Math/Vector/Vector3.h>

#include <numbers>
#include <cmath>


struct SpotLight
{
    Vector4 color = { 1,1,1,1 };		// ライトの色

    Vector3 position = { 0,0,0 };	// ライトの位置
    float intensity = 1.0f;	        // 輝度

    Vector3 direction = { 0,-1,0 };	// ライトの向き
    float distance = 4.0f;	        // ライトの有効距離

    float decay = 1.0f;	            // 距離による減衰率
    float cosAngle                        // 角度
        = std::cosf(std::numbers::pi_v<float> / 3.0f);
    float falloutStartAngle               // 開始角度
        = std::cosf(std::numbers::pi_v<float> / 3.0f);
    uint32_t isHalf = 1;            // ハーフランバートを使うか

    uint32_t castShadow = 1;        // シャドウマップを生成するか
    float pad[3] = {};

    Matrix4x4 viewProjection = Matrix4x4::Identity();
};


class SpotLightComponent : public Light
{
public:
    SpotLightComponent();
    ~SpotLightComponent() = default;

    void Update() override;
    bool IsCastShadow() const override { return data_.castShadow == 1; }

    SpotLight& GetData() { return data_; }
    const SpotLight& GetData() const { return data_; }

    void SetColor(const Vector4& color) { data_.color = color; }
    void SetPosition(const Vector3& position) { data_.position = position; }
    void SetIntensity(float intensity) { data_.intensity = intensity; }
    void SetDirection(const Vector3& direction) { data_.direction = direction.Normalize(); }
    void SetDistance(float distance) { data_.distance = distance; }
    void SetDecay(float decay) { data_.decay = decay; }
    void SetCosAngle(float angleDegree);
    void SetFalloutStartAngle(float angleDegree);
    void SetIsHalf(bool isHalf) { data_.isHalf = isHalf ? 1 : 0; }
    void SetCastShadow(bool castShadow) { data_.castShadow = castShadow ? 1 : 0; }

    void UpdateViewProjection();

    void CreateShadowMap(uint32_t shadowMapSize);
    void ReleaseShadowMap();
    uint32_t GetShadowMapHandle() const { return shadowMapHandle_; }

private:
    SpotLight data_ = {};  
    uint32_t shadowMapHandle_ = 0;

    Matrix4x4 LookAt(const Vector3& eye, const Vector3& at, const Vector3& up);
};


