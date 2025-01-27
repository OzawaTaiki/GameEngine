#include "SceneFactory.h"

#include <src/Scene/SampleScene.h>
#include <src/Scene/EssentialScene.h>
#include <src/Scene/LightScene.h>
#include <src/Scene/LoadPlaneScene.h>
#include <src/Scene/AnimationNodeMisc.h>


#include <Features/Scene/ParticleTestScene.h>

std::unique_ptr<BaseScene> SceneFactory::CreateScene(const std::string& _name)
{
    if (_name == "Sample")
    {
        return std::make_unique<SampleScene>();
    }
    //else if (_name == "ParticleTest")
    //{
    //    return std::make_unique<ParticleTestScene>();
    //}
    else if (_name == "Essential")
    {
        return std::make_unique<EssentialScene>();
    }
    else if (_name == "Light")
    {
        return std::make_unique<LightScene>();
    }
    else if (_name == "LoadPlane")
    {
        return std::make_unique<LoadPlaneScene>();
    }
    else if (_name == "Animation_NodeMisc")
    {
        return std::make_unique<AnimationNodeMisc>();
    }

    assert("Scene Not Found");

    return nullptr;
}

std::string SceneFactory::ShowDebugWindow()
{
    std::string name = "";
#ifdef _DEBUG

    if (ImGui::BeginTabBar("Scene"))
    {
        if (ImGui::BeginTabItem("Scene"))
        {
            //if (ImGui::Button("ParticleTest"))
            //{
            //    name = "ParticleTest";
            //}
            if (ImGui::Button("Essential"))
            {
                name = "Essential";
            }
            if (ImGui::Button("Light"))
            {
                name = "Light";
            }
            if (ImGui::Button("LoadPlane"))
            {
                name = "LoadPlane";
            }
            if (ImGui::Button("AnimationNode"))
            {
                name = "Sample";
            }
            if (ImGui::Button("Animation_NodeMisc"))
            {
                name = "Animation_NodeMisc";
            }

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }


#endif // DEBUG
    return name;

}
