#include <Framework/Framework.h>

#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Features/Light/System/LightingSystem.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Sprite/Sprite.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Features/Event/EventManager.h>
#include <System/Audio/AudioSystem.h>
#include <Framework/LayerSystem/LayerSystem.h>

#include <Debug/ImGuiDebugManager.h>



void Framework::Run()
{
    Initialize(L"");

    while (!IsEndRequested())
    {
        Update();
        Draw();
    }

    Finalize();
}

void Framework::Initialize(const std::wstring& _winTitle)
{
    winApp_ = WinApp::GetInstance();
    winApp_->Initilize(_winTitle.c_str());

    dxCommon_ = DXCommon::GetInstance();
    dxCommon_->Initialize(winApp_, WinApp::kWindowWidth_, WinApp::kWindowHeight_);

    srvManager_ = SRVManager::GetInstance();
    srvManager_->Initialize();


#ifdef _DEBUG
    ImGuiDebugManager::GetInstance()->Initialize();
#endif
    imguiManager_ = new ImGuiManager();
    imguiManager_->Initialize();

    rtvManager_ = RTVManager::GetInstance();
    rtvManager_->Initialize(dxCommon_->GetBackBufferSize(), WinApp::kWindowWidth_, WinApp::kWindowHeight_);


    PSOManager::GetInstance()->Initialize();

    LightingSystem::GetInstance()->Initialize();


    TextureManager* instance = TextureManager::GetInstance();
    instance->Initialize();

    Sprite::StaticInitialize(WinApp::kWindowWidth_, WinApp::kWindowHeight_);

    ModelManager::GetInstance()->Initialize();

    lineDrawer_ = LineDrawer::GetInstance();
    lineDrawer_->Initialize();

    input_ = Input::GetInstance();
    input_->Initilize(winApp_);

    particleManager_ = ParticleSystem::GetInstance();
    particleManager_->Initialize();

    Time::Initialize();

    audio_ = Audio::GetInstance();
    audio_->Initialize();

    AudioSystem::GetInstance()->Initialize();

    gameTime_ = GameTime::GetInstance();
    gameTime_->Initialize();

    fontCache_ = FontCache::GetInstance();
    fontCache_->Initialize(dxCommon_->GetDevice(), dxCommon_->GetCommandList(),
        Vector2(static_cast<float>(WinApp::kWindowWidth_), static_cast<float>(WinApp::kWindowHeight_)));

    textRenderer_ = TextRenderer::GetInstance();
    textRenderer_->Initialize(dxCommon_->GetDevice(), dxCommon_->GetCommandList(),
        Vector2(static_cast<float>(WinApp::kWindowWidth_), static_cast<float>(WinApp::kWindowHeight_)));

    collisionManager_ = CollisionManager::GetInstance();

    sceneManager_ = SceneManager::GetInstance();

    Time_MT::GetInstance()->Initialize();

    //LayerSystem::Initialize();

}

void Framework::Update()
{
    srvManager_->PreDraw();
    gameTime_->BeginFrame();
    if (winApp_->ProcessMessage())
    {
        endRequest_ = true;
    }
    imguiManager_->Begin();

    // フレーム始め
    TextRenderer::GetInstance()->BeginFrame();

    Time::Update();

    input_->Update();

}

void Framework::PreDraw()
{
}

void Framework::PostDraw()
{
#ifdef _DEBUG
    ImGuiDebugManager::GetInstance()->ShowDebugWindow();
#endif // _DEBUG

    imguiManager_->End();
    imguiManager_->Draw();

    dxCommon_->PostDraw();
}

void Framework::Finalize()
{
    sceneManager_->Finalize();
    LayerSystem::Finalize();

    Time_MT::GetInstance()->Finalize();
    collisionManager_->Finalize();
    textRenderer_->Finalize();
    imguiManager_->Finalize();
    delete imguiManager_;

    winApp_->Finalize();
}

