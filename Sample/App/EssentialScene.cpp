#include "EssentialScene.h"
#include <Rendering/Model/ModelManager.h>
#include <Rendering/Sprite/Sprite.h>
#include <UI/ImGuiManager/ImGuiManager.h>
#include <Framework/eScene/SceneManager.h>


std::unique_ptr<BaseScene>EssentialScene::Create()
{
    return std::make_unique<EssentialScene>();
}

EssentialScene::~EssentialScene()
{
}

void EssentialScene::Initialize()
{
    SceneCamera_.Initialize();
    SceneCamera_.translate_ = { 0,5,-20 };
    SceneCamera_.rotate_ = { 0.26f,0,0 };
    SceneCamera_.UpdateMatrix();

    debugCamera_.Initialize();

    plane_ = std::make_unique<ObjectModel>();
    plane_->Initialize("Tile/Tile.gltf");
    plane_->uvScale_ = { 100,100 };

    lineDrawer_ = LineDrawer::GetInstance();
    lineDrawer_->Initialize();
    lineDrawer_->SetCameraPtr(&SceneCamera_);

    input_ = Input::GetInstance();

    ParticleManager::GetInstance()->CreateParticleGroup("Essential", "plane/plane.gltf", nullptr, BlendMode::Normal);
    for (int i = 0; i < 10; i++)
    {
        ParticleInitParam param;
        param.position = { i * 0.3f,i * 0.1f,0 };
        param.isInfiniteLife = true;
        param.color = { 1,1,1,1 };
        param.size = { 1,1,1 };

        Particle& particle = particles_.emplace_back();
        particle.Initialize(param);

    }

    ParticleManager::GetInstance()->AddParticleToGroup("Essential", particles_);

}

void EssentialScene::Update()
{

#ifdef _DEBUG
    ImGui();

    if(input_->IsKeyTriggered(DIK_RETURN) &&
       input_->IsKeyPressed(DIK_RSHIFT))
        enableDebugCamera_ = !enableDebugCamera_;
#endif // _DEBUG

    plane_->Update();

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

void EssentialScene::Draw()
{
    ModelManager::GetInstance()->PreDrawForObjectModel();
   // plane_->Draw(&SceneCamera_, { 1,1,1,1 });

//-------------------------------------------------------------------------
    ModelManager::GetInstance()->PreDrawForAnimationModel();


//-------------------------------------------------------------------------
    Sprite::PreDraw();


//-------------------------------------------------------------------------
    ParticleManager::GetInstance()->Draw(&SceneCamera_);
    lineDrawer_->Draw();
}

#ifdef _DEBUG
#include <imgui.h>
void EssentialScene::ImGui()
{

}
#endif // _DEBUG
