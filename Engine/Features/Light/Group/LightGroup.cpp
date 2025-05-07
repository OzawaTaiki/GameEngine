#include "LightGroup.h"

uint32_t LightGroup::shadowMapSize_ = 1024;

void LightGroup::Initialize()
{
    // デフォルトのライトを設定
    auto defaultDirLight= std::make_shared<DirectionalLightComponent>();
    SetDirectionalLight(defaultDirLight);

    pointLights_.clear();

    dirty_ = true;
}

void LightGroup::SetDirectionalLight(std::shared_ptr<DirectionalLightComponent> _light)
{
    directionalLight_ = _light;
    if (directionalLight_) {
        // シャドウマップサイズを更新
        directionalLight_->UpdateViewProjection(shadowMapSize_);
    }
    dirty_ = true;
}

void LightGroup::AddPointLight(const std::string& _name, std::shared_ptr<PointLightComponent> _light)
{
    if (pointLights_.size() >= MAX_POINT_LIGHT)
    {
        return;
    }

    _light->SetName(_name);
    pointLights_[_name] = _light;

    if (_light->IsCastShadow())
    {
        _light->CreateShadowMaps(shadowMapSize_);
    }

    dirty_ = true;
}

void LightGroup::RemovePointLight(const std::string& _name)
{
    auto it = pointLights_.find(_name);
    if (it != pointLights_.end()) {
        // ポイントライトを削除
        pointLights_.erase(it);
        dirty_ = true;
    }
}

std::shared_ptr<PointLightComponent> LightGroup::GetPointLight(const std::string& _name)
{
    auto it = pointLights_.find(_name);
    if (it != pointLights_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<PointLightComponent>> LightGroup::GetAllPointLights() const
{
    std::vector<std::shared_ptr<PointLightComponent>> lights;
    for (auto& light : pointLights_) {
        lights.push_back(light.second);
    }
    return lights;
}

void LightGroup::AddSpotLight(const std::string& _name, std::shared_ptr<SpotLightComponent> _light)
{
    if (spotLights_.size() >= MAX_SPOT_LIGHT)
    {
        return;
    }
    _light->SetName(_name);
    spotLights_[_name] = _light;
    if (_light->IsCastShadow())
    {
        _light->CreateShadowMap(shadowMapSize_);
    }
    dirty_ = true;
}

void LightGroup::RemoveSpotLight(const std::string& _name)
{
    auto it = spotLights_.find(_name);
    if (it != spotLights_.end()) {
        // スポットライトを削除
        spotLights_.erase(it);
        dirty_ = true;
    }
}

std::shared_ptr<SpotLightComponent> LightGroup::GetSpotLight(const std::string& _name)
{
    auto it = spotLights_.find(_name);
    if (it != spotLights_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<SpotLightComponent>> LightGroup::GetAllSpotLights() const
{
    std::vector<std::shared_ptr<SpotLightComponent>> lights;
    for (auto& light : spotLights_) {
        lights.push_back(light.second);
    }
    return lights;
}

void LightGroup::Update()
{
    if (directionalLight_)
    {
        directionalLight_->Update();
    }

    for (auto& light : pointLights_)
    {
        light.second->Update();
    }

    dirty_ = true;
}

LightGroup::LightTransferData LightGroup::GetLightData()
{
    LightTransferData data;

    if (directionalLight_ && enableDirectionalLight_)
    {
        data.directionalLight = directionalLight_->GetData();
    }
    else
    {
        data.directionalLight.color = { 1.0f, 1.0f, 1.0f, 1.0f };
        data.directionalLight.direction = { 0.0f, -1.0f, 0.0f };
        data.directionalLight.intensity = enableDirectionalLight_ ? 1.0f : 0.0f;
        data.directionalLight.isHalf = 1;
        data.directionalLight.castShadow = 0;
    }

    if (enablePointLight_)
    {
        uint32_t index = 0;
        for (auto& [name,light] : pointLights_)
        {
            if (index >= MAX_POINT_LIGHT)
            {
                break;
            }
            data.pointLights[index] = light->GetData();
            ++index;
        }
        data.numPointLight = index;
    }
    else
    {
        data.numPointLight = 0;
    }

    if (enableSpotLight_)
    {
        uint32_t index = 0;
        for (auto& [name, light] : spotLights_)
        {
            if (index >= MAX_SPOT_LIGHT)
            {
                break;
            }
            data.spotLights[index] = light->GetData();
            ++index;
        }
        data.numSpotLight = index;
    }
    else
    {
        data.numSpotLight = 0;
    }

    dirty_ = false;

    return data;

}

void LightGroup::ImGui()
{
#ifdef _DEBUG

    ImGui::Begin("Light Settings"); 
    {
        if (ImGui::CollapsingHeader("Light Group Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Enable Directional Light", &enableDirectionalLight_);
            ImGui::Checkbox("Enable Point Lights", &enablePointLight_);
            ImGui::Checkbox("Enable Spot Lights", &enableSpotLight_);
        }

        if (enableDirectionalLight_) {
            DrawDirectionalLightImGui();
        }

        if (enablePointLight_) {
            DrawPointLightsImGui();
        }

        if (enableSpotLight_) {
            DrawSpotLightsImGui();
        }

    }
    ImGui::End();
#endif // _DEBUG
}

void LightGroup::DrawDirectionalLightImGui()
{
#ifdef _DEBUG
    if (ImGui::CollapsingHeader("Directional Light")) {
        if (!directionalLight_) {
            if (ImGui::Button("Create Directional Light")) {
                auto light = std::make_shared<DirectionalLightComponent>();
                SetDirectionalLight(light);
            }
            return;
        }

        DirectionalLight& data = directionalLight_->GetData();

        ImGui::ColorEdit4("Color", &data.color.x);

        // Direction setting (before normalization)
        Vector3 dir = data.direction;
        if (ImGui::DragFloat3("Direction", &dir.x, 0.01f)) {
            directionalLight_->SetDirection(dir); // Normalized internally
        }

        ImGui::DragFloat("Intensity", &data.intensity, 0.01f, 0.0f, 10.0f);

        bool isHalf = data.isHalf != 0;
        if (ImGui::Checkbox("Half Lambert", &isHalf)) {
            directionalLight_->SetIsHalf(isHalf);
        }

        bool castShadow = data.castShadow != 0;
        if (ImGui::Checkbox("Cast Shadow", &castShadow)) {
            directionalLight_->SetCastShadow(castShadow);
        }
    }
#endif // _DEBUG

}

void LightGroup::DrawPointLightsImGui()
{
#ifdef _DEBUG
    if (ImGui::CollapsingHeader("Point Lights")) {
        // Adding new point light
        ImGui::InputText("New Light Name", newPointLightName_, sizeof(newPointLightName_));

        ImGui::SameLine();
        if (ImGui::Button("Add##PointLight")) {
            if (newPointLightName_[0] != '\0') {
                std::string name = newPointLightName_;
                auto light = std::make_shared<PointLightComponent>();
                AddPointLight(name, light);

                // Clear input field
                memset(newPointLightName_, 0, sizeof(newPointLightName_));
            }
        }

        if (pointLights_.empty()) {
            ImGui::Text("No point lights available");
            return;
        }

        // Displaying/editing existing point lights
        if (ImGui::BeginTabBar("PointLightTabs")) {
            // List to store lights to remove
            std::vector<std::string> lightsToRemove;

            for (auto& [name, light] : pointLights_) {
                if (ImGui::BeginTabItem(name.c_str())) {
                    PointLight& data = light->GetData();

                    ImGui::PushID(name.c_str());
                    ImGui::ColorEdit4("Color", &data.color.x);
                    ImGui::DragFloat3("Position", &data.position.x, 0.1f);
                    ImGui::DragFloat("Intensity", &data.intensity, 0.01f, 0.0f, 10.0f);
                    ImGui::DragFloat("Radius", &data.radius, 0.1f, 0.1f, 100.0f);
                    ImGui::DragFloat("Decay", &data.decay, 0.01f, 0.1f, 5.0f);

                    bool isHalf = data.isHalf != 0;
                    if (ImGui::Checkbox("Half Lambert", &isHalf)) {
                        light->SetIsHalf(isHalf);
                    }

                    bool castShadow = data.castShadow != 0;
                    if (ImGui::Checkbox("Cast Shadow", &castShadow)) {
                        light->SetCastShadow(castShadow);
                    }

                    if (ImGui::Button("Delete")) {
                        lightsToRemove.push_back(name);
                    }

                    ImGui::PopID();
                    ImGui::EndTabItem();
                }
            }

            // Process lights to remove
            for (const auto& name : lightsToRemove) {
                RemovePointLight(name);
            }

            ImGui::EndTabBar();
        }
    }
#endif // _DEBUG
}

void LightGroup::DrawSpotLightsImGui()
{
#ifdef _DEBUG
    if (ImGui::CollapsingHeader("Spot Lights")) {
        // Adding new spot light
        ImGui::InputText("New Light Name", newSpotLightName_, sizeof(newSpotLightName_));

        ImGui::SameLine();
        if (ImGui::Button("Add##SpotLight")) {
            if (newSpotLightName_[0] != '\0') {
                std::string name = newSpotLightName_;
                auto light = std::make_shared<SpotLightComponent>();
                AddSpotLight(name, light);


                // Clear input field
                memset(newSpotLightName_, 0, sizeof(newSpotLightName_));
            }
        }

        if (spotLights_.empty()) {
            ImGui::Text("No spot lights available");
            return;
        }

        // Displaying/editing existing spot lights
        if (ImGui::BeginTabBar("SpotLightTabs")) {
            // List to store lights to remove
            std::vector<std::string> lightsToRemove;

            for (auto& [name, light] : spotLights_) {
                if (ImGui::BeginTabItem(name.c_str())) {
                    SpotLight& data = light->GetData();

                    ImGui::PushID(name.c_str());
                    ImGui::ColorEdit4("Color", &data.color.x);
                    ImGui::DragFloat3("Position", &data.position.x, 0.1f);
                    ImGui::DragFloat3("Direction", &data.direction.x, 0.01f);
                    light->SetDirection(data.direction); // Ensure normalization

                    ImGui::DragFloat("Intensity", &data.intensity, 0.01f, 0.0f, 10.0f);
                    ImGui::DragFloat("Distance", &data.distance, 0.1f, 0.1f, 100.0f);
                    ImGui::DragFloat("Decay", &data.decay, 0.01f, 0.1f, 5.0f);

                    // Convert angles from radians to degrees
                    float angleDegrees = acos(data.cosAngle) * 180.0f / 3.14159f;
                    if (ImGui::SliderFloat("Angle", &angleDegrees, 1.0f, 90.0f)) {
                        light->SetCosAngle(angleDegrees);
                    }

                    float falloutAngleDegrees = acos(data.falloutStartAngle) * 180.0f / 3.14159f;
                    if (ImGui::SliderFloat("Fallout Start Angle", &falloutAngleDegrees, 1.0f, angleDegrees)) {
                        light->SetFalloutStartAngle(falloutAngleDegrees);
                    }

                    bool isHalf = data.isHalf != 0;
                    if (ImGui::Checkbox("Half Lambert", &isHalf)) {
                        light->SetIsHalf(isHalf);
                    }

                    bool castShadow = data.castShadow != 0;
                    if (ImGui::Checkbox("Cast Shadow", &castShadow)) {
                        light->SetCastShadow(castShadow);
                    }

                    if (ImGui::Button("Delete")) {
                        lightsToRemove.push_back(name);
                    }

                    ImGui::PopID();
                    ImGui::EndTabItem();
                }
            }

            // Process lights to remove
            for (const auto& name : lightsToRemove) {
                RemoveSpotLight(name);
            }

            ImGui::EndTabBar();
        }
    }
#endif // _DEBUG
}
