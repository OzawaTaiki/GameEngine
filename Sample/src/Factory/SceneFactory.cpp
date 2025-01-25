#include "SceneFactory.h"

#include <src/Scene/SampleScene.h>
#include <src/Scene/EssentialScene.h>
#include <src/Scene/LightScene.h>

#include <Features/Scene/ParticleTestScene.h>

std::unique_ptr<BaseScene> SceneFactory::CreateScene(const std::string& _name)
{
    if (_name == "Sample")
    {
        return std::make_unique<SampleScene>();
    }
    else if (_name == "ParticleTest")
    {
        return std::make_unique<ParticleTestScene>();
    }
    else if (_name == "Feature")
    {
        return std::make_unique<EssentialScene>();
    }
    else if (_name == "Light")
    {
        return std::make_unique<LightScene>();
    }

    assert("Scene Not Found");

    return nullptr;
}

std::string SceneFactory::ShowDebugWindow()
{
#ifdef _DEBUG
    std::string name = "";

    if (ImGui::BeginTabBar("Scene"))
    {
        if (ImGui::BeginTabItem("Scene"))
        {
            if (ImGui::Button("Sample"))
            {
                name = "Sample";
            }
            if (ImGui::Button("ParticleTest"))
            {
                name = "ParticleTest";
            }
            if (ImGui::Button("Feature"))
            {
                name = "Feature";
            }
            if (ImGui::Button("Light"))
            {
                name = "Light";
            }

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    return name;

#endif // DEBUG

}
