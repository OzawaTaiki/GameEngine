#include "LightScene.h"

LightScene::~LightScene()
{
}

void LightScene::Initialize()
{
    SceneCamera_.Initialize();
    SceneCamera_.translate_ = { 0,5,-20 };
    SceneCamera_.rotate_ = { 0.26f,0,0 };
    SceneCamera_.UpdateMatrix();

    lights_.Initialize();

    PointLight pointLight1 = {};
    pointLight1.position = { 1,1,0 };

    PointLight pointLight2 = {};
    pointLight2.position = { 1,1,1 };

    lights_.AddPointLight(pointLight1);
    lights_.AddPointLight(pointLight2);

    SpotLight spotLight1 = {};
    spotLight1.position = { -1,1,0 };
    spotLight1.direction = { 0,-1,0 };

    SpotLight spotLight2 = {};
    spotLight2.position = { -1,1,1 };
    spotLight2.direction = { 0,-1,0 };

    lights_.AddSpotLight(spotLight1);
    lights_.AddSpotLight(spotLight2);


    terrain_ = std::make_unique<ObjectModel>();
    terrain_->Initialize("terrain.obj", "terrain");


}

void LightScene::Update()
{
#ifdef _DEBUG
    lights_.DrawDebugWindow();
#endif // _DEBUG
    LightingSystem::GetInstance()->SetLightGroup(&lights_);


    terrain_->Update();

    SceneCamera_.Update();
    SceneCamera_.UpdateMatrix();

}

void LightScene::Draw()
{
    terrain_->Draw(&SceneCamera_,{1,1,1,1});
}
