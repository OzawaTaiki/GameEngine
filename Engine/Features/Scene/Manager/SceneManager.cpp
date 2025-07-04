#include <Features/Scene/Manager/SceneManager.h>

#include <Debug/ImGuiDebugManager.h>

#include <cassert>

SceneManager* SceneManager::GetInstance()
{
    static SceneManager instance;
    return &instance;
}

SceneManager::SceneManager()
    : sceneFactory_(nullptr),
    currentSceneName_("empty"),
    nextSceneName_("empty"),
    isTransition_(false)
{
#ifdef _DEBUG
    ImGuiDebugManager::GetInstance()->RegisterMenuItem("SceneManager", [this](bool* _open) {ImGui(_open); });
#endif // _DEBUG

}

SceneManager::~SceneManager()
{
#ifdef _DEBUG
    ImGuiDebugManager::GetInstance()->RemoveDebugWindow("SceneManager");
#endif // _DEBUG

    delete sceneFactory_;
}

void SceneManager::SetSceneFactory(ISceneFactory* _sceneFactory)
{
    if (_sceneFactory != nullptr)
    {
        sceneFactory_ = _sceneFactory;
    }
}

void SceneManager::Initialize(const std::string& _name)
{
    currentScene_ = sceneFactory_->CreateScene(_name);
    currentSceneName_ = _name;

    nextSceneName_ = "empty";
    currentScene_->Initialize(nullptr);
}

void SceneManager::Update()
{
    assert(sceneFactory_ != nullptr);

    currentScene_->Update();

    if(!transitionQueue_.empty())
    {
        SetTransition(std::move(transitionQueue_.front()));
        transitionQueue_.pop();
    }

    if (isTransition_)
    {
        transition_->Update();
        if (transition_->IsEnd())
        {
            isTransition_ = false;
        }
    }
}

void SceneManager::Draw()
{
    currentScene_->Draw();
    if (isTransition_)
    {
        transition_->Draw();
    }
}

void SceneManager::DrawShadow()
{
    currentScene_->DrawShadow();
}

void SceneManager::SetTransition(std::unique_ptr<ISceneTransition> _transition)
{
    if (_transition != nullptr)
    {
        if (isTransition_)
        {
            // すでにトランジション中なら、キューに追加
            transitionQueue_.push(std::move(_transition));
            return;
        }
        transition_ = std::move(_transition);
        transition_->Initialize();
    }
}

void SceneManager::ReserveScene(const std::string& _name, std::unique_ptr<SceneData> _sceneData)
{
    auto instance = GetInstance();

    instance->nextSceneName_ = _name;
    if (instance->sceneData_)
        instance->sceneData_.reset();
    if (_sceneData == nullptr)
    {
        _sceneData = std::make_unique<SceneData>();
    }
    _sceneData->beforeScene = instance->currentSceneName_;

    instance->sceneData_ = std::move(_sceneData);

    if (instance->transition_ != nullptr)
    {
        instance->transition_->Start();
        instance->isTransition_ = true;
    }
}


void SceneManager::ChangeScene()
{
    auto instance = GetInstance();

    if (instance->nextSceneName_ == "empty" ||
        instance->currentSceneName_ == instance->nextSceneName_)
    {
        return;
    }

    if (instance->transition_ != nullptr)
    {
        if (!instance->transition_->CanSwitch())
        {
            return;
        }
    }


    instance->currentScene_.reset();

    instance->currentScene_ = instance->sceneFactory_->CreateScene(instance->nextSceneName_);
    instance->currentScene_->Initialize(instance->sceneData_.get());
    instance->currentSceneName_ = instance->nextSceneName_;
    instance->nextSceneName_ = "empty";

}

void SceneManager::Finalize()
{
    if (currentScene_ != nullptr)
    {
        currentScene_.reset();
    }
    if (sceneFactory_ != nullptr)
    {
        sceneFactory_ = nullptr;
    }
}

#ifdef _DEBUG
#include <imgui.h>
void SceneManager::ImGui(bool* _open)
{
    ImGui::Begin("SceneManager", _open);
    {
        ImGui::Text("Current Scene : %s", currentSceneName_.c_str());

        std::string name = sceneFactory_->ShowDebugWindow();
        if (!name.empty())
        {
            ReserveScene(name, nullptr);
        }

        ImGui::Text("-----Transition-----");
        ImGui::Text("Transitioning : %s", isTransition_ ? "true" : "false");
    }
    ImGui::End();
}
#endif // _DEBUG


/*

    ImGui::SeparatorText("TIME_MT");
    ImGui::Text("Frametate: %.3f fps", Time_MT::GetFramerate());
    ImGui::Text("DeltaTime: %4.2f ms", Time_MT::GetDeltaTime<double>() * 1000.0);

    bool isFixed = Time_MT::IsDeltaTimeFixed();
    if(ImGui::Checkbox("Fixed DeltaTime", &isFixed))
        Time_MT::SetDeltaTimeFixed(isFixed);

    ImGui::SeparatorText("GameTime");

    ImGui::Text("Frametate: %.3f fps", GameTime::GetInstance()->GetFramerate());
    ImGui::Text("DeltaTime: %4.2f ms", GameTime::GetInstance()->GetDeltaTime() * 1000.0);
*/