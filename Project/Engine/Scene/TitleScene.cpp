#include "TitleScene.h"

#include "SceneManager.h"

std::unique_ptr<BaseScene>TitleScene::Create()
{
    return std::make_unique<TitleScene>();
}

TitleScene::~TitleScene()
{
    if(true)
    {
        if (true)
        {

        }
        // do something
    }
}

void TitleScene::Initialize()
{
}

void TitleScene::Update()
{
#ifdef _DEBUG
    ImGui();
#endif // _DEBUG

}

void TitleScene::Draw()
{
}

#ifdef _DEBUG
#include <imgui.h>
void TitleScene::ImGui()
{
    ImGui::BeginTabBar("Scene");
    if (ImGui::BeginTabItem("Title"))
    {
        ImGui::Text("Title Scene");
        char buf[128];
        strcpy_s(buf, nextSceneName_.c_str());
        if(ImGui::InputText("Next Scene", buf, sizeof(buf)))
        {
            nextSceneName_ = buf;
        }
        if (ImGui::Button("Change Scene"))
        {
            SceneManager::ReserveScene(nextSceneName_);
        }
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();

}
#endif // _DEBUG