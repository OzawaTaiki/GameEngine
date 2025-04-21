#include <Features/Light/Point/PointLight.h>
#include <Core/DXCommon/DXCommon.h>
#include <Math/Matrix/MatrixFunction.h>

#include <Core/DXCommon/RTV/RTVManager.h>

PointLightComponent::PointLightComponent()
{
    data_.color = { 1,1,1,1 };
    data_.position = { 0,1,0 };
    data_.intensity = 1.0f;
    data_.radius = 5.0f;
    data_.decay = 0.5f;
    data_.isHalf = 1;
    data_.castShadow = 1;
}

void PointLightComponent::Update()
{
    if (parent_) {
        data_.position = *parent_;
    }
}

void PointLightComponent::UpdateViewProjections(uint32_t shadowMapSize)
{
    if (!data_.castShadow) return;
    CalculateCubemapViewProjections(shadowMapSize);
}

void PointLightComponent::CreateShadowMaps(uint32_t shadowMapSize)
{
    if (!data_.castShadow) return;

    // 既存のシャドウマップを解放
    ReleaseShadowMaps();

    // 6面分のシャドウマップを作成
    std::string mapName = name_ + "_ShadowMap";

    uint32_t handle =
        RTVManager::GetInstance()->CreateCubemapRenderTarget(
            mapName,
            shadowMapSize,
            shadowMapSize,
            DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
            { 1.0f, 1.0f, 1.0f, 1.0f },
            true
        );

    shadowMapHandles_.push_back(handle);

    // ビュー射影行列を更新
    UpdateViewProjections(shadowMapSize);
}

void PointLightComponent::ReleaseShadowMaps()
{
    shadowMapHandles_.clear();
}

void PointLightComponent::CalculateCubemapViewProjections(uint32_t shadowMapSize)
{// キューブマップの6面の視点方向
    // +X, -X, +Y, -Y, +Z, -Z
    const Vector3 directions[6] = {
        {1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, -1.0f}
    };

    // キューブマップの6面の上方向
    // 注意: DirectXのキューブマップの向きに合わせる
    const Vector3 ups[6] = {
        {0.0f, 1.0f, 0.0f},  // +X: 上は+Y
        {0.0f, 1.0f, 0.0f},  // -X: 上は+Y
        {0.0f, 0.0f, -1.0f}, // +Y: 上は-Z
        {0.0f, 0.0f, 1.0f},  // -Y: 上は+Z
        {0.0f, 1.0f, 0.0f},  // +Z: 上は+Y
        {0.0f, 1.0f, 0.0f}   // -Z: 上は+Y
    };

    // 90度のFOVで平行投影ではなく透視投影を使用
    float fovY = 3.14159f / 2.0f; // 90度 = π/2
    float aspectRatio = 1.0f;     // 正方形のテクスチャ
    float nearClip = 0.1f;
    float farClip = data_.radius * 2.0f; // ライトの影響半径の2倍

    // 各面のビュー射影行列を計算
    for (int i = 0; i < 6; i++) {
        // ライトの位置からの視点
        Vector3 target = data_.position + directions[i];

        // ビュー行列を計算
        Matrix4x4 viewMat = LookAt(data_.position, target, ups[i]);

        // 射影行列を計算（透視投影、90度FOV）
        Matrix4x4 projMat = MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);

        // ビュー射影行列を設定
        data_.viewProjection[i] = viewMat * projMat;
    }
}

Matrix4x4 PointLightComponent::LookAt(const Vector3& eye, const Vector3& at, const Vector3& up) {
    Vector3 zaxis = (at - eye).Normalize();
    Vector3 xaxis = up.Cross(zaxis).Normalize();
    if (xaxis.Length() == 0) {
        xaxis = { 1.0f, 0.0f, 0.0f };
    }
    Vector3 yaxis = zaxis.Cross(xaxis);
    if (yaxis.Length() == 0) {
        yaxis = { 0.0f, 1.0f, 0.0f };
    }

    Matrix4x4 result =
    {
        {
            {xaxis.x, yaxis.x, zaxis.x, 0.0f},
            {xaxis.y, yaxis.y, zaxis.y, 0.0f},
            {xaxis.z, yaxis.z, zaxis.z, 0.0f},
            {-xaxis.Dot(eye), -yaxis.Dot(eye), -zaxis.Dot(eye), 1.0f}
        }
    };

    return result;
}
