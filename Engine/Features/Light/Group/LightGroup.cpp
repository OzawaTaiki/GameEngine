#define NOMINMAX

#include <Features/Light/Group/LightGroup.h>
#include <Core/DXCommon/DXCommon.h>
#include <Features/Light/System/LightingSystem.h>
#include <Debug/ImGuiManager.h>
#include <Math/Matrix/MatrixFunction.h>

#include <cmath>

Vector2 LightGroup::shadowMapSize_ = { 128,128 };

void LightGroup::Initialize()
{
    directionalLight_ = {
        .color = {1.0f, 1.0f, 1.0f, 1.0f},
        .direction = {0.0f, -1.0f, 0.0f},
        .intensity = 1.0f,
        .isHalf = 1
    };

    selectablePointLights_.clear();
    selectableSpotLights_.clear();

    dirty_ = true;
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

void LightGroup::AddPointLight(const PointLight& _light, const std::string& _name, Vector3* _parent)
{
    if (selectablePointLights_.size() >= MAX_POINT_LIGHT)
        return;

    // ベース名を取得
    std::string baseName = _name;
    if (baseName == "")
    {
        // デフォルト名を設定
        baseName = "PointLight";
    }

    // 同じ名前があるかどうか
    int32_t count = -1;
    for (auto& light : selectablePointLights_)
    {
        // 名前が一致するか
        if (light.name == baseName)
        {
            count=0;
        }
        else if( light.name.rfind(baseName + " ", 0) == 0)
        {
            // 末尾の数字を取得
            size_t pos = light.name.find_last_of(' ');
            if (pos != std::string::npos)
            {
                // 数字を取得
                std::string num = light.name.substr(pos + 1);
                int32_t curCount = std::stoi(num);
                // 最大値を取得
                count = std::max(count, curCount);
            }
        }
    }

    // 要素を追加
    NamedLight<PointLight>& pl = selectablePointLights_.emplace_back();
    // ライトを設定
    pl.light = _light;

    // 名前を設定
    pl.name = baseName;
    if (count != -1)
    {
        // Countが0はないときはナンバリング
        pl.name += " " + std::to_string(count + 1);
    }

    pl.select = false;

    if (_parent)
    {
        pl.parent = _parent;
    }

    dirty_ = true;
}

void LightGroup::AddSpotLight(const SpotLight& _light, const std::string& _name, Vector3* _parent)
{
    if (selectableSpotLights_.size() >= MAX_SPOT_LIGHT)
        return;

    // ベース名を取得
    std::string baseName = _name;
    if (baseName == "")
    {
        // デフォルト名を設定
        baseName = "SpotLight";
    }

    // 同じ名前があるかどうか
    int32_t count = -1;
    for (auto& light : selectableSpotLights_)
    {
        // 名前が一致するか
        if (light.name == baseName)
        {
            count = 0;
        }
        else if( light.name.rfind(baseName + " ", 0) == 0)
        {
            // 末尾の数字を取得
            size_t pos = light.name.find_last_of(' ');
            if (pos != std::string::npos)
            {
                // 数字を取得
                std::string num = light.name.substr(pos + 1);
                int32_t curCount = std::stoi(num);
                // 最大値を取得
                count = std::max(count, curCount);
            }
        }
    }

    // 要素を追加
    NamedLight<SpotLight>& sl = selectableSpotLights_.emplace_back();
    // ライトを設定
    sl.light = _light;

    // 名前を設定
    sl.name = baseName;
    if (count != -1)
    {
        // Countが0はないときはナンバリング
        sl.name += " " + std::to_string(count + 1);
    }

    sl.select = false;

    if (_parent)
    {
        sl.parent = _parent;
    }

    dirty_ = true;
}

PointLight& LightGroup::GetPointLight(const std::string& _name)
{
    for (auto& light : selectablePointLights_)
    {
        if (light.name == _name)
        {
            return light.light;
        }
    }

    return selectablePointLights_.front().light;
}

SpotLight& LightGroup::GetSpotLight(const std::string& _name)
{
    for (auto& light : selectableSpotLights_)
    {
        if (light.name == _name)
        {
            return light.light;
        }
    }
    return selectableSpotLights_.front().light;
}

void LightGroup::DeletePointLight(const std::string& _name)
{
    for (auto it = selectablePointLights_.begin(); it != selectablePointLights_.end(); it++)
    {
        if (it->name == _name)
        {
            selectablePointLights_.erase(it);
            break;
        }
    }
    dirty_ = true;
}

void LightGroup::DeleteSpotLight(const std::string& _name)
{
    for (auto it = selectableSpotLights_.begin(); it != selectableSpotLights_.end(); it++)
    {
        if (it->name == _name)
        {
            selectableSpotLights_.erase(it);
            break;
        }
    }
    dirty_ = true;
}

LightGroup::LightTransferData LightGroup::GetLightData()
{
    if (!dirty_)
        return lightData_;

    LightTransferData data;

    directionalLight_.direction = directionalLight_.direction.Normalize();

    Vector3 up = { 0.0f,1.0f,0.0f };
    if (std::abs(up.Dot(directionalLight_.direction)) == 1.0f)
    {
        up = { 1.0f,0.0f,0.0f };
    }

    const float distance = 100.0f;
    //Matrix4x4 viewMat = LookAt(-directionalLight_.direction * distance, (0,0,0), up);
    Matrix4x4 viewMat = LookAt(-directionalLight_.direction * distance, (0.0f, 0.0f,0.0f), up);

    //float fovY_ = 0.45f;
    //float aspectRatio_ = 16.0f / 9.0f;
    float nearClip_ = 0.1f;
    float farClip_ = 1000.0f;

    float halfWidth = shadowMapSize_.x / 2.0f;
    float halfHeight = shadowMapSize_.y / 2.0f;

    Matrix4x4 projMat =
        //MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
        MakeOrthographicMatrix(-halfWidth, -halfWidth, halfHeight, halfHeight , nearClip_, farClip_);
    directionalLight_.viewProjection = viewMat * projMat;

    data.directionalLight = directionalLight_;

    if(!enableDirectionalLight_)
        data.directionalLight.intensity = 0;



    size_t i = 0;
    if(enablePointLight_)
    {
        data.numPointLight = static_cast<uint32_t>(selectablePointLights_.size());
        for (auto it = selectablePointLights_.begin(); it != selectablePointLights_.end(); it++)
        {
            data.pointLights[i] = it->light;
            if (it->parent)
            {
                data.pointLights[i].position += *it->parent;
            }
            i++;
        }
    }

    if(enableSpotLight_)
    {
        i = 0;
        data.numSpotLight = static_cast<uint32_t>(selectableSpotLights_.size());
        for (auto it = selectableSpotLights_.begin(); it != selectableSpotLights_.end(); it++)
        {
            data.spotLights[i] = it->light;
            if (it->parent)
            {
                data.spotLights[i].position += *it->parent;
            }
            i++;
        }
    }

    dirty_ = false;

    lightData_ = data;

    return data;
}

void LightGroup::DrawDebugWindow()
{
#ifdef _DEBUG

    ImGui::Begin("LightGroup");

    if(ImGui::Checkbox("DirectionalLight", &enableDirectionalLight_))
        dirty_ = true;
    if(ImGui::Checkbox("PointLight", &enablePointLight_))
        dirty_ = true;
    if(ImGui::Checkbox("SpotLight", &enableSpotLight_))
        dirty_ = true;

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.18f, 0.22f, 0.58f, 0.71f));
    if(ImGui::CollapsingHeader("Directional Light"))
    {
        ImGui::ColorEdit4("Color", &directionalLight_.color.x);
        ImGui::DragFloat3("Direction", &directionalLight_.direction.x, 0.01f);
        ImGui::DragFloat("Intensity", &directionalLight_.intensity, 0.01f,0.0f);
        bool isHalf = directionalLight_.isHalf;
        ImGui::Checkbox("IsHalf", &isHalf);
        directionalLight_.isHalf = isHalf;
        dirty_ = true;
    }

    if (ImGui::CollapsingHeader("Point Light"))
    {
        dirty_ = true;
        ImGui::InputText("create Light Name", addPointLightName_, sizeof(addPointLightName_));

        if (ImGui::Button("Add"))
        {
            PointLight light = {};
            std::string name = addPointLightName_;
            AddPointLight(light, name);
            strcpy_s(addPointLightName_, sizeof(addPointLightName_), "");
            dirty_ = true;
        }
        ImGui::Separator();

        if (ImGui::BeginTable("Point Light", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
        {
            uint32_t i = 0;
            for (auto it = selectablePointLights_.begin(); it != selectablePointLights_.end(); it++)
            {
                ImGui::TableNextColumn();
                ImGui::Selectable(it->name.c_str(), &(*it).select);
            }
            ImGui::EndTable();
        }

        ImGuiTabBarFlags tabBarFlags = ImGuiTabBarFlags_AutoSelectNewTabs;
        tabBarFlags |= ImGuiTabBarFlags_TabListPopupButton;

        ImGui::BeginTabBar("Point Light", tabBarFlags);

        uint32_t i = 0;
        for (auto it = selectablePointLights_.begin(); it != selectablePointLights_.end();)
        {
            if (ImGui::BeginTabItem(it->name.c_str(), &(*it).select, ImGuiTabItemFlags_None))
            {
                PointLight& pointLight = (*it).light;

                ImGui::PushID(static_cast<int>(i));
                ImGui::ColorEdit4("Color", &pointLight.color.x);
                ImGui::DragFloat3("Position", &pointLight.position.x, 0.01f);
                ImGui::DragFloat("Intensity", &pointLight.intensity, 0.01f, 0.0f);
                ImGui::DragFloat("Radius", &pointLight.radius, 0.01f, 0.0f);
                ImGui::DragFloat("Decay", &pointLight.decay, 0.01f, 0.0f);
                bool isHalf = pointLight.isHalf;
                ImGui::Checkbox("IsHalf", &isHalf);
                pointLight.isHalf = isHalf;

                if (ImGui::Button("Delete"))
                {
                    it = selectablePointLights_.erase(it);
                    ImGui::PopID();
                    ImGui::EndTabItem();
                    continue;
                }
                ImGui::PopID();
                ImGui::EndTabItem();
            }
            ++i;
            ++it;
        }
        ImGui::EndTabBar();
    }

    ImGui::PushID("Spot Light");
    if (ImGui::CollapsingHeader("Spot Light"))
    {
        dirty_ = true;
        ImGui::InputText("create Light Name", addSpotLightName_, sizeof(addSpotLightName_));

        if (ImGui::Button("Add"))
        {
            SpotLight light = {};
            std::string name = addSpotLightName_;
            AddSpotLight(light, name);
            strcpy_s(addSpotLightName_, sizeof(addSpotLightName_), "");
            dirty_ = true;
        }

        if (ImGui::BeginTable("Spot Light", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
        {
            uint32_t i = 0;
            for (auto it = selectableSpotLights_.begin(); it != selectableSpotLights_.end(); it++)
            {
                ImGui::TableNextColumn();
                ImGui::Selectable(it->name.c_str(), &(*it).select);
            }
            ImGui::EndTable();
        }

        ImGuiTabBarFlags tabBarFlags = ImGuiTabBarFlags_AutoSelectNewTabs;
        tabBarFlags |= ImGuiTabBarFlags_TabListPopupButton;

        ImGui::BeginTabBar("Spot Light", tabBarFlags);

        uint32_t i = 0;
        for (auto it = selectableSpotLights_.begin(); it != selectableSpotLights_.end();)
        {
            if (ImGui::BeginTabItem(it->name.c_str(), &(*it).select, ImGuiTabItemFlags_None))
            {
                SpotLight& spotLight = (*it).light;

                ImGui::PushID(static_cast<int>(i));
                ImGui::ColorEdit4("Color", &spotLight.color.x);
                ImGui::DragFloat3("Position", &spotLight.position.x, 0.01f);
                ImGui::DragFloat("Intensity", &spotLight.intensity, 0.01f, 0.0f);
                ImGui::DragFloat3("Direction", &spotLight.direction.x, 0.01f);
                ImGui::DragFloat("Distance", &spotLight.distance, 0.01f, 0.0f);
                ImGui::DragFloat("Decay", &spotLight.decay, 0.01f, 0.0f);
                ImGui::DragFloat("CosAngle", &spotLight.cosAngle, 0.01f, 0.0f, 1.0f);
                ImGui::DragFloat("FalloutStartAngle", &spotLight.falloutStartAngle, 0.01f, 0.0f, 1.0f);
                bool isHalf = spotLight.isHalf;
                ImGui::Checkbox("IsHalf", &isHalf);
                spotLight.isHalf = isHalf;

                if (ImGui::Button("Delete"))
                {
                    it = selectableSpotLights_.erase(it);
                    ImGui::PopID();
                    ImGui::EndTabItem();
                    continue;
                }

                ImGui::PopID();
                ImGui::EndTabItem();
            }
            ++i;
            ++it;
        }
        ImGui::EndTabBar();
    }
    ImGui::PopID();
    ImGui::PopStyleColor();

    ImGui::End();
#endif // _DEBUG

}

Matrix4x4 LightGroup::LookAt(const Vector3& _eye, const Vector3& _at, const Vector3& _up)
{
    Vector3 zaxis = (_at - _eye).Normalize();
    Vector3 xaxis = _up.Cross(zaxis).Normalize();
    if (xaxis.Length() == 0)
    {
        xaxis = { 1.0f,0.0f,0.0f };
    }
    Vector3 yaxis = zaxis.Cross(xaxis);
    if (yaxis.Length() == 0)
    {
        yaxis = { 0.0f,1.0f,0.0f };
    }

    Matrix4x4 result =
    {
        {
            {xaxis.x, yaxis.x, zaxis.x, 0.0f},
            {xaxis.y, yaxis.y, zaxis.y, 0.0f},
            {xaxis.z, yaxis.z, zaxis.z, 0.0f},
            { -xaxis.Dot(_eye), -yaxis.Dot(_eye), -zaxis.Dot(_eye), 1.0f }
        }
    };

    return result;
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
