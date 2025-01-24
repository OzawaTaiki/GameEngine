#include "FeatureScene.h"

#include <Features/Scene/Manager/SceneManager.h>
#include <Debug/ImGuiManager.h>
#include <Features/Sprite/Sprite.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>


FeatureScene::~FeatureScene()
{
}

void FeatureScene::Initialize()
{
    SceneCamera_.Initialize();
    SceneCamera_.translate_ = { 0,5,-20 };
    SceneCamera_.rotate_ = { 0.26f,0,0 };
    SceneCamera_.UpdateMatrix();
    debugCamera_.Initialize();


    lineDrawer_ = LineDrawer::GetInstance();
    lineDrawer_->Initialize();
    lineDrawer_->SetCameraPtr(&SceneCamera_);

    input_ = Input::GetInstance();

    sphere_ = std::make_unique<ObjectModel>();
    sphere_->Initialize("Sphere/Sphere64.obj", "sphere");

    sphereTexture_ = TextureManager::GetInstance()->Load("monsterBall.png");



    lights_ = std::make_unique<LightGroup>();
    lights_->Initialize();

}

void FeatureScene::Update()
{
    // シーン関連更新
#ifdef _DEBUG
    if (Input::GetInstance()->IsKeyTriggered(DIK_RETURN) &&
        Input::GetInstance()->IsKeyPressed(DIK_RSHIFT))
        enableDebugCamera_ = !enableDebugCamera_;

    sphere_->ImGui();

    lights_->DrawDebugWindow();
#endif // _DEBUG
    LightingSystem::GetInstance()->SetLightGroup(lights_.get());


    sphere_->Update();

    if (input_->IsKeyTriggered(DIK_TAB))
    {
        SceneManager::GetInstance()->ReserveScene("ParticleTest");
    }

    if (enableDebugCamera_)
    {
        debugCamera_.Update();
        SceneCamera_.matView_ = debugCamera_.matView_;
        SceneCamera_.TransferData();
        ParticleManager::GetInstance()->Update(debugCamera_.rotate_);
    }
    else
    {
        SceneCamera_.Update();
        SceneCamera_.UpdateMatrix();
        ParticleManager::GetInstance()->Update(SceneCamera_.rotate_);
    }

}

void FeatureScene::Draw()
{

    sphere_->Draw(&SceneCamera_, sphereTexture_, { 1,1,1,1 });



    //button_->Draw();

    ParticleManager::GetInstance()->Draw(&SceneCamera_);

}
