#pragma once

#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>

#include <Features/Light/Directional/DirectionalLight.h>
#include <Features/Light/Point/PointLight.h>
#include <Features/Light/Spot/SpotLight.h>

#include <map>
#include <string>
#include <memory>
#include <vector>

class LightGroup
{
public:
    static constexpr uint32_t MAX_POINT_LIGHT = 32;
    static constexpr uint32_t MAX_SPOT_LIGHT = 32;

    struct LightTransferData
    {
        DirectionalLight directionalLight;
        PointLight pointLights[MAX_POINT_LIGHT];
        SpotLight spotLights[MAX_SPOT_LIGHT];
        uint32_t numPointLight;
        uint32_t numSpotLight;
    };

    LightGroup() = default;
    ~LightGroup() = default;

    void Initialize();

    void SetDirectionalLight(std::shared_ptr<DirectionalLightComponent> _light);
    std::shared_ptr<DirectionalLightComponent> GetDirectionalLight() { return directionalLight_; }

    void AddPointLight(const std::string& _name, std::shared_ptr<PointLightComponent> _light);
    void RemovePointLight(const std::string& _name);
    std::shared_ptr<PointLightComponent> GetPointLight(const std::string& _name);
    std::vector<std::shared_ptr<PointLightComponent>> GetAllPointLights() const;

    void AddSpotLight(const std::string& _name, std::shared_ptr<SpotLightComponent> _light);
    void RemoveSpotLight(const std::string& _name);
    std::shared_ptr<SpotLightComponent> GetSpotLight(const std::string& _name);
    std::vector<std::shared_ptr<SpotLightComponent>> GetAllSpotLights() const;

    void SetEnableDirectionalLight(bool _enable) { enableDirectionalLight_ = _enable; }
    void SetEnablePointLight(bool _enable) { enablePointLight_ = _enable; }
    void SetEnableSpotLight(bool _enable) { enableSpotLight_ = _enable; }

    void Update();

    LightTransferData GetLightData();


    static void SetShadowMapSize(uint32_t _size) { shadowMapSize_ = _size; }
    static uint32_t GetShadowMapSize() { return shadowMapSize_; }

    void ImGui();

private:

    void DrawDirectionalLightImGui();
    void DrawPointLightsImGui();
    void DrawSpotLightsImGui();


    char newPointLightName_[128] = {};
    char newSpotLightName_[128] = {};


    std::shared_ptr<DirectionalLightComponent> directionalLight_ = nullptr;
    std::map<std::string, std::shared_ptr<PointLightComponent>> pointLights_;
    std::map<std::string, std::shared_ptr<SpotLightComponent>> spotLights_;

    bool enableDirectionalLight_ = true;
    bool enablePointLight_ = true;
    bool enableSpotLight_ = true;

    bool dirty_ = true;

    static uint32_t shadowMapSize_;
};


