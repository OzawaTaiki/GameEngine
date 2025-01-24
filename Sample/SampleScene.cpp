#include "SampleScene.h"

#include <Features/Scene/Manager/SceneManager.h>
#include <Debug/ImGuiManager.h>
#include <Features/Sprite/Sprite.h>
#include <Features/Model/Manager/ModelManager.h>


SampleScene::~SampleScene()
{
}

void SampleScene::Initialize()
{
    SceneCamera_.Initialize();
    SceneCamera_.translate_ = { 0,5,-20 };
    SceneCamera_.rotate_ = { 0.26f,0,0 };
    SceneCamera_.UpdateMatrix();
    debugCamera_.Initialize();

    plane_ = std::make_unique<ObjectModel>();
    plane_->Initialize("Tile/Tile.gltf", "Ground");
    plane_->GetUVTransform().SetScale(Vector2(100, 100));

    lineDrawer_ = LineDrawer::GetInstance();
    lineDrawer_->Initialize();
    lineDrawer_->SetCameraPtr(&SceneCamera_);

    input_ = Input::GetInstance();

    aModel_ = std::make_unique<AnimationModel>();
    aModel_->Initialize("AnimSample/AnimSample.gltf");

    oModel_ = std::make_unique<ObjectModel>();
    oModel_->Initialize("Cylinder.gltf", "c");

    lights_ = std::make_unique<LightGroup>();
    lights_->Initialize();

    button_ = std::make_unique<UIButton>();
    button_->Initialize("button");

}

void SampleScene::Update()
{
    // シーン関連更新
#ifdef _DEBUG
    if (Input::GetInstance()->IsKeyTriggered(DIK_RETURN) &&
        Input::GetInstance()->IsKeyPressed(DIK_RSHIFT))
        enableDebugCamera_ = !enableDebugCamera_;

    lights_->DrawDebugWindow();
#endif // _DEBUG
    LightingSystem::GetInstance()->SetLightGroup(lights_.get());


    plane_->Update();
    aModel_->Update();
    oModel_->Update();
    button_->Update();

    if (button_->IsPressed()||input_->IsKeyTriggered(DIK_TAB))
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

void SampleScene::Draw()
{

    plane_->Draw(&SceneCamera_, { 1,1,1,1 });
    //oModel_->Draw(&SceneCamera_, { 1,1,1,1 });

    //aModel_->Draw(&SceneCamera_, { 1,1,1,1 });


    Sprite::PreDraw();
    //button_->Draw();

    ParticleManager::GetInstance()->Draw(&SceneCamera_);
    //lineDrawer_->Draw();

}

#ifdef _DEBUG
#include <imgui.h>
void SampleScene::ImGui()
{

}
#endif // _DEBUG
