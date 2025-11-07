#pragma once

#include <Features/Light/Light.h>

struct DirectionalLight
{
    Matrix4x4 viewProjection = Matrix4x4::Identity();

    Vector4 color = { 1,1,1,1 };		// ライトの色

    Vector3 direction = { 0,-1,0 };	// ライトの向き
    float intensity = 1.0f;	        // 輝度

    uint32_t isHalf = 1;            // ハーフランバートを使うか
    uint32_t castShadow = 1;        // シャドウマップを生成するか
    float shadowFactor = 0.2f;        // 影の濃さ 1.0f = 影なし
    float pad;
};

class DirectionalLightComponent : public Light
{
public:
    DirectionalLightComponent();
    ~DirectionalLightComponent() = default;

    void Update() override;

    bool IsCastShadow() const override { return data_.castShadow == 1; }

    DirectionalLight& GetData() { return data_; }
    const DirectionalLight& GetData() const { return data_; }


    void SetColor(const Vector4& color) { data_.color = color; }
    void SetDirection(const Vector3& direction) { data_.direction = direction.Normalize(); UpdateViewProjection(); }
    void SetIntensity(float intensity) { data_.intensity = intensity; }
    void SetCastShadow(bool castShadow) { data_.castShadow = castShadow ? 1 : 0; }
    void SetIsHalf(bool isHalf) { data_.isHalf = isHalf ? 1 : 0; }
    void SetShadowFactor(float shadowFactor) { data_.shadowFactor = shadowFactor; }


    void UpdateViewProjection();

    uint32_t GetShadowMapHandle() const { return shadowMapHandle_; }
    void SetShadowMapHandle(uint32_t handle) { shadowMapHandle_ = handle; }

private:
    DirectionalLight data_ = {};
    uint32_t shadowMapHandle_ = 0;

    Matrix4x4 LookAt(const Vector3& eye, const Vector3& at, const Vector3& up);
};
