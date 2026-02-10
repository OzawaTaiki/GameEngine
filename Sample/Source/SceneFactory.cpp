#include "SceneFactory.h"

#include <Scene/SampleScene.h>
#include <Scene/DeveScene.h>
#include <Features/Effect/Editor/EffectEditorScene.h>

using namespace Engine;

std::unique_ptr<BaseScene> SceneFactory::CreateScene(const std::string& _name)
{
    if (_name == "Sample")
    {
        return std::make_unique<SampleScene>();
    }
    else if (_name == "EffectTest")
    {
        return std::make_unique<EffectEditorScene>();
    }
    else if (_name == "Develop")
    {
        return std::make_unique<DeveScene>();
    }

    assert("Scene Not Found");

    return nullptr;
}

std::string SceneFactory::ShowDebugWindow()
{
#ifdef _DEBUG

    ImGui::SeparatorText("Scene Factory");

    if (ImGui::Button("Sample"))
    {
        return "Sample";
    }
    if (ImGui::Button("EffectTest"))
    {
        return "EffectTest";
    }
    if (ImGui::Button("Develop"))
    {
        return "Develop";
    }


#endif // _DEBUG
    return "";

}
