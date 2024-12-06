#include "SceneManager.h"
#include "Input/Input.h"
#include "Utility/Time.h"
#include "Utility/Config.h"
#include <cassert>

SceneManager* SceneManager::GetInstance()
{
    static SceneManager instance;
    return &instance;
}

SceneManager::~SceneManager()
{
    scenes_.clear();
    currentScene_.reset();
}

void SceneManager::RegisterScene(const std::string& _name, SceneFactory _scene)
{
    auto instance = GetInstance();
    instance->scenes_[_name] = _scene;
}

void SceneManager::Initialize(const std::string& _name)
{
    assert(scenes_.size() > 0);
    auto it = scenes_.find(_name);
    assert(it != scenes_.end());

    configManager_ = ConfigManager::GetInstance();
    configManager_->Initialize();

    configManager_->LoadData();

    configManager_->SetSceneName(_name);

    currentScene_ = it->second();
    currentSceneName_ = _name;

    nextSceneName_ = "empty";
    currentScene_->Initialize();
}

void SceneManager::Update()
{
    Time::Update();
#ifdef _DEBUG
    ImGui();
#endif // _DEBUG
    Input::GetInstance()->Update();
    currentScene_->Update();
}

void SceneManager::Draw()
{
    currentScene_->Draw();
}

void SceneManager::ReserveScene(const std::string& _name)
{
    auto instance = GetInstance();

    assert(instance->scenes_.find(_name) != instance->scenes_.end());
    instance->nextSceneName_ = _name;
}

void SceneManager::ChangeScene()
{
    auto instance = GetInstance();

    if (instance->nextSceneName_ == "empty" ||
        instance->currentSceneName_ == instance->nextSceneName_)
    {
        return;
    }

    instance->currentScene_.reset();

    instance->currentScene_ = instance->scenes_[instance->nextSceneName_]();
    instance->currentScene_->Initialize();
    instance->currentSceneName_ = instance->nextSceneName_;
    instance->nextSceneName_ = "empty";

    instance->configManager_->SetSceneName(instance->currentSceneName_);
}

#ifdef _DEBUG
#include <imgui.h>
void SceneManager::ImGui()
{
    char comboLabel[128];

    ImGui::Begin("SceneManager");
    ImGui::Text("Frametate: %.3f fps", Time::GetFramerate());
    ImGui::Text("DeltaTime: %4.2f ms", Time::GetDeltaTime<double>() * 1000.0);

    for (auto& scene : scenes_)
    {
        strcpy_s(comboLabel, scene.first.c_str());
        if (ImGui::Button(comboLabel))
        {
            ReserveScene(scene.first);
            break;
        }
    }

    ImGui::Text("Current Scene : %s", currentSceneName_.c_str());


    ImGui::End();
}
#endif // _DEBUG
