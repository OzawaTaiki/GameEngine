#pragma once

#include <Features/Light/Light.h>

#include <Math/Vector/Vector3.h>

#include <vector>

struct PointLight
{
    Vector4 color = { 1, 1, 1, 1 };     // ライトの色

    Vector3 position = { 0, 1, 0 };     // ライトの位置
    float intensity = 1.0f;           // 輝度

    float radius = 5.0f;              // ライトの影響半径
    float decay = 0.5f;               // 減衰率
    uint32_t isHalf = 1;              // ハーフランバートを使うか
    uint32_t castShadow = 1;            // シャドウマップを生成するか

    float shadowFactor = 0.2f;          // 影の濃さ 1.0f = 影なし
    float pad[3];

    Matrix4x4 viewProjection[6];      // シャドウマップのビュー射影行列（キューブマップの6面）
};

class PointLightComponent : public Light
{
public:
    PointLightComponent();
    ~PointLightComponent() = default;

    void Update() override;
    bool IsCastShadow() const override { return data_.castShadow == 1; }

    PointLight& GetData() { return data_; }
    const PointLight& GetData() const { return data_; }
    void SetColor(const Vector4& color) { data_.color = color; }

    void SetPosition(const Vector3& position) { data_.position = position; }
    void SetIntensity(float intensity) { data_.intensity = intensity; }
    void SetRadius(float radius) { data_.radius = radius; }
    void SetDecay(float decay) { data_.decay = decay; }
    void SetIsHalf(bool isHalf) { data_.isHalf = isHalf ? 1 : 0; }
    void SetCastShadow(bool castShadow) { data_.castShadow = castShadow ? 1 : 0; }
    void SetShadowFactor(float shadowFactor) { data_.shadowFactor = shadowFactor; }
    void SetOffset(const Vector3& off) { offset = off; }

    void UpdateViewProjections();

    void CreateShadowMaps(uint32_t shadowMapSize);
    void ReleaseShadowMaps();
    const std::vector<uint32_t>& GetShadowMapHandles() const { return shadowMapHandles_; }

private:
    PointLight data_ = {};
    Vector3 offset = { 0.0f, 0.0f, 0.0f };
    std::vector<uint32_t> shadowMapHandles_;

    // キューブマップ各面のビュー行列計算ヘルパー
    void CalculateCubemapViewProjections();
    Matrix4x4 LookAt(const Vector3& eye, const Vector3& at, const Vector3& up);
};