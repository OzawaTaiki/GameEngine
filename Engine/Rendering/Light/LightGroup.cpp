#include "LightGroup.h"
#include <Core/DirectX/DXCommon.h>


void LightGroup::Initialize()
{
    directionalLight_ = {
        .color = {1.0f, 1.0f, 1.0f, 1.0f},
        .direction = {0.0f, -1.0f, 0.0f},
        .intensity = 1.0f,
        .isHalf = 1
    };
}

void LightGroup::Update()
{
}

void LightGroup::Draw()
{
}

void LightGroup::SetDirectionalLight(const DirectionalLight& _light)
{
    directionalLight_ = _light;
    dirty_ = true;
}

void LightGroup::AddPointLight(const PointLight& _light)
{
    if (pointLights_.size() >= MAX_POINT_LIGHT)
        return;

    pointLights_.push_back(_light);
    dirty_ = true;
}

void LightGroup::AddSpotLight(const SpotLight& _light)
{
    if (spotLights.size() >= MAX_SPOT_LIGHT)
        return;
    spotLights.push_back(_light);
    dirty_ = true;
}

LightGroup::LightTransferData LightGroup::GetLightData()
{
    LightTransferData data;

    data.directionalLight = directionalLight_;

    data.numPointLight = static_cast<uint32_t>(pointLights_.size());
    data.numSpotLight = static_cast<uint32_t>(spotLights.size());

    for (size_t i = 0; i < pointLights_.size(); i++)
    {
        data.pointLights[i] = pointLights_[i];
    }

    for (size_t i = 0; i < spotLights.size(); i++)
    {
        data.spotLights[i] = spotLights[i];
    }

    dirty_ = false;

    return data;
}

LightGroup::LightTransferData LightGroup::GetDefaultLightData()
{
    LightTransferData data;
    data.directionalLight = {
        .color = {1.0f, 1.0f, 1.0f, 1.0f},
        .direction = {0.0f, -1.0f, 0.0f},
        .intensity = 1.0f,
        .isHalf = 1
    };
    data.numPointLight = 0;
    data.numSpotLight = 0;
    return data;

}
