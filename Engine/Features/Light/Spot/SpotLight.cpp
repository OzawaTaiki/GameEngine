#include <Features/Light/Spot/SpotLight.h>
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/RTV/RTVManager.h>
#include <Math/Vector/VectorFunction.h>
#include <Math/Matrix/MatrixFunction.h>

SpotLightComponent::SpotLightComponent()
{
    data_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    data_.position = { 1.0f, 1.0f, 0.0f };
    data_.direction = { -1.0f, -1.0f, 0.0f };
    data_.intensity = 4.0f;
    data_.distance = 7.0f;
    data_.decay = 2.0f;
    data_.cosAngle = std::cosf(std::numbers::pi_v<float> / 3.0f);  // 60度
    data_.falloutStartAngle = std::cosf(std::numbers::pi_v<float> / 3.0f);
    data_.isHalf = 1;
    data_.castShadow = 1;
}

void SpotLightComponent::Update()
{
    if(parent_)
    {
        data_.position = *parent_;
    }

    data_.direction = data_.direction.Normalize();

    if (data_.castShadow && shadowMapHandle_ != 0)
    {
        UpdateViewProjection();
    }
}

void SpotLightComponent::SetCosAngle(float angleDegree)
{
    float angleRad = angleDegree * std::numbers::pi_v<float> / 180.0f;
    data_.cosAngle = std::cosf(angleRad);
}

void SpotLightComponent::SetFalloutStartAngle(float angleDegree)
{
    float angleRad = angleDegree * std::numbers::pi_v<float> / 180.0f;
    data_.falloutStartAngle = std::cosf(angleRad);
}

void SpotLightComponent::UpdateViewProjection()
{
    if (!data_.castShadow) 
        return;

    Vector3 up = { 0.0f, 1.0f, 0.0f };
    if (std::abs(up.Dot(data_.direction)) > 0.99f)
    {
        up = { 1.0f, 0.0f, 0.0f };
    }

    Vector3 target = data_.position + data_.direction;
    Matrix4x4 viewMat = LookAt(data_.position, target, up);

    float fovY = 2.0f * std::acosf(data_.cosAngle);

    // シャドウマップは正方形
    float aspectRatio = 1.0f;

    // 射影行列の計算
    float nearClip = 0.1f;
    float farClip = data_.distance;
    Matrix4x4 projMat = MakePerspectiveFovMatrix(fovY, aspectRatio, nearClip, farClip);

    // ビュー射影行列を設定
    data_.viewProjection = viewMat * projMat;

}

void SpotLightComponent::CreateShadowMap(uint32_t shadowMapSize)
{
    if (!data_.castShadow) return;

    // 既存のシャドウマップを解放
    ReleaseShadowMap();

    // シャドウマップを作成
    std::string mapName = name_ + "_ShadowMap";

    shadowMapHandle_ = RTVManager::GetInstance()->CreateRenderTarget(
        mapName,
        shadowMapSize,
        shadowMapSize,
        DXGI_FORMAT_R32_FLOAT,
        { 1.0f, 1.0f, 1.0f, 1.0f },
        true // 深度バッファも作成
    );

    // ビュー射影行列を更新
    UpdateViewProjection();
}

void SpotLightComponent::ReleaseShadowMap()
{
    shadowMapHandle_ = 0;
}

Matrix4x4 SpotLightComponent::LookAt(const Vector3& eye, const Vector3& at, const Vector3& up)
{
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
