#include "DirectionalLight.h"

#include <Math/Matrix/MatrixFunction.h>

DirectionalLightComponent::DirectionalLightComponent()
{
    data_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    data_.direction = { 0.25f, -1.0f, 0.0f };
    data_.direction = data_.direction.Normalize();
    data_.intensity = 1.0f;
    data_.isHalf = 1;
    data_.castShadow = 1;
    data_.shadowFactor = 0.2f;
}

void DirectionalLightComponent::Update()
{
    data_.direction = data_.direction.Normalize();
}

void DirectionalLightComponent::UpdateViewProjection()
{
    if (!data_.castShadow) return;

    // シャドウマップサイズから計算値を設定
    float sceneSize = 30.0f;
    float halfSize = sceneSize / 2.0f;

    // 上方向ベクトルの設定
    Vector3 up = { 0.0f, 1.0f, 0.0f };
    if (std::abs(up.Dot(data_.direction)) == 1.0f) {
        up = { 1.0f, 0.0f, 0.0f };
    }

    // ライトの視点位置からの距離
    const float distance = 100.0f;

    // ビュー行列を計算
    Matrix4x4 viewMat = LookAt(-data_.direction * distance, Vector3{ 0.0f, 0.0f, 0.0f }, up);

    // 射影行列を計算（平行投影）
    float nearClip = 0.1f;
    float farClip = 200.0f;
    Matrix4x4 projMat = MakeOrthographicMatrix(-halfSize, -halfSize, halfSize, halfSize, nearClip, farClip);

    // ビュー射影行列を設定
    data_.viewProjection = viewMat * projMat;
}

Matrix4x4 DirectionalLightComponent::LookAt(const Vector3& _eye, const Vector3& _at, const Vector3& _up)
{
    Vector3 zaxis = (_at - _eye).Normalize();
    Vector3 xaxis = _up.Cross(zaxis).Normalize();
    if (xaxis.Length() == 0.0f)
    {
        xaxis = { 1.0f, 0.0f, 0.0f };
    }

    Vector3 yaxis = zaxis.Cross(xaxis);
    if (yaxis.Length() == 0.0f)
    {
        yaxis = { 0.0f, 1.0f, 0.0f };
    }

    Matrix4x4 result =
    {
        {
            {xaxis.x, yaxis.x, zaxis.x, 0.0f},
            {xaxis.y, yaxis.y, zaxis.y, 0.0f},
            {xaxis.z, yaxis.z, zaxis.z, 0.0f},
            {-xaxis.Dot(_eye), -yaxis.Dot(_eye), -zaxis.Dot(_eye), 1.0f}
        }
    };

    return result;

}
