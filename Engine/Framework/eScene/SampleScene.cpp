#include "SampleScene.h"

#include "SceneManager.h"
#include <UI/ImGuiManager/ImGuiManager.h>
#include <Rendering/Sprite/Sprite.h>
#include <Rendering/Model/ModelManager.h>


std::unique_ptr<BaseScene>SampleScene::Create()
{
    return std::make_unique<SampleScene>();
}

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
    plane_->uvScale_ = { 100,100 };

    lineDrawer_ = LineDrawer::GetInstance();
    lineDrawer_->Initialize();
    lineDrawer_->SetCameraPtr(&SceneCamera_);

    input_ = Input::GetInstance();

    aModel_ = std::make_unique<AnimationModel>();
    aModel_->Initialize("AnimSample/AnimSample.gltf");

    oModel_ = std::make_unique<ObjectModel>();
    oModel_->Initialize("AnimatedCube/AnimatedCube.gltf", "c");

}

void SampleScene::Update()
{
    // シーン関連更新
#ifdef _DEBUG
    if (Input::GetInstance()->IsKeyTriggered(DIK_RETURN) &&
        Input::GetInstance()->IsKeyPressed(DIK_RSHIFT))
        enableDebugCamera_ = !enableDebugCamera_;
#endif // _DEBUG

    if(ImGui::Button("rotate"))
    {
        aModel_->SetAnimation("RotateAnim");
    }
    if (ImGui::Button("scale"))
    {
        aModel_->SetAnimation("ScaleAnim",true);
    }
    if (ImGui::Button("stop"))
    {
        aModel_->StopAnimation();
    }
    if (ImGui::Button("idle"))
    {
        aModel_->ToIdle(1.0f);
    }


    plane_->Update();
    aModel_->Update();
    oModel_->Update();

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

    ModelManager::GetInstance()->PreDrawForObjectModel();
    //plane_->Draw(&SceneCamera_, { 1,1,1,1 });
    //oModel_->Draw(&SceneCamera_, { 1,1,1,1 });

    ModelManager::GetInstance()->PreDrawForAnimationModel();
    aModel_->Draw(&SceneCamera_, { 1,1,1,1 });

    Sprite::PreDraw();


    ParticleManager::GetInstance()->Draw(&SceneCamera_);
    lineDrawer_->Draw();

}

#ifdef _DEBUG
#include <imgui.h>
void SampleScene::ImGui()
{

}
#endif // _DEBUG
