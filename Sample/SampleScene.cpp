#include "SampleScene.h"

#include <Features/Scene/Manager/SceneManager.h>
#include <Debug/ImGuiManager.h>
#include <Features/Sprite/Sprite.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Collision/Manager/CollisionManager.h>
#include <Debug/ImguITools.h>
#include <Features/Model/Primitive/Ring.h>
#include <Features/Model/Primitive/Cylinder.h>
#include <Features/PostEffects/DepthBasedOutLine.h>

#include <Features/Effect/Emitter/ParticleEmitter.h>
#include <Features/Model/Primitive/Triangle.h>
#include <Features/Model/Primitive/Plane.h>

#include <Features/Model/Primitive/Plane.h>
#include <Features/Model/Primitive/Triangle.h>


SampleScene::~SampleScene()
{
    for (auto& col : colliders_)
    {
        CollisionManager::GetInstance()->UnregisterCollider(col);
        delete col;
        col = nullptr;
    }
    colliders_.clear();

    for (auto& model : models_)
    {
        delete model;
        model = nullptr;
    }
    models_.clear();

    for (auto& collider : s_colliders_)
    {
        CollisionManager::GetInstance()->UnregisterCollider(collider);
        delete collider;
        collider = nullptr;
    }

    s_colliders_.clear();

    for (auto& model : s_models_)
    {
        delete model;
        model = nullptr;
    }
    s_models_.clear();


}

void SampleScene::Initialize(SceneData* _sceneData)
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

    oModel_ = std::make_unique<ObjectModel>("plane");
    oModel_->Initialize("plane/plane.gltf");
    oModel_->translate_.x = 3;

    oModel2_ = std::make_unique<ObjectModel>("cube");
    oModel2_->Initialize("Cube/Cube.obj");
    oModel2_->translate_.x = -3;

    aModel_ = std::make_unique<ObjectModel>("sample");
    aModel_->Initialize("AnimSample/AnimSample.gltf");

    plane_ = std::make_unique<ObjectModel>("ground");
    plane_->Initialize("terrain.obj");
    plane_->GetUVTransform().SetScale({ 100,100 });

    uint32_t textureHandle = TextureManager::GetInstance()->Load("uvChecker.png");
    sprite_ = Sprite::Create("uvChecker", textureHandle);

    lights_ = std::make_shared<LightGroup>();
    lights_->Initialize();

    //auto pl = std::make_shared<PointLightComponent>();
    //lights_->AddPointLight("PointLight", pl);


    LightingSystem::GetInstance()->SetActiveGroup(lights_);

    sequence_ = std::make_unique<AnimationSequence>("test");
    sequence_->Initialize("Resources/Data/");

    Cylinder* cylinder = new Cylinder(1.0f, 2.0f,1.0f);
    cylinder->SetDivide(32);
    cylinder->SetLoop(true);

    Triangle* triangle = new Triangle();
    triangle->SetVertices(
        Vector3(0.0f, 1.0f, 0.0f),
        Vector3(1.0f, -1.0f, 0.0f),
        Vector3(-1.0f, -1.0f, 0.0f)
    );
    triangle->SetNormal(Vector3(0.0f, 0.0f, -1.0f).Normalize());

    Plane* plane = new Plane();
    plane->SetNormal(Vector3(1.0f, 1.0f, 0.0f).Normalize());


    test_ = std::make_unique<ObjectModel>("Cylinder");
    test_->Initialize(cylinder->Generate("Cylinder"));
    test_->translate_.y = 1.0f;

    //emitter_ = std::make_unique<ParticleEmitter>();
    //emitter_->Initialize("test");

    DepthBasedOutLine::GetInstance()->SetCamera(&SceneCamera_);

    ParticleSystem::GetInstance()->SetCamera(&SceneCamera_);

    soundInstance_ = AudioSystem::GetInstance()->Load("Resources/Sounds/Alarm01.wav");
    //voiceInstance_ = soundInstance_->Play(1.0f, true);

    skyBox_ = std::make_unique<SkyBox>();
    skyBox_->Initialize(30.0f);
    skyBox_->SetTexture("rosendal_plains_2_2k.dds");

    Create();

}

void SampleScene::Update()
{
    // シーン関連更新
#ifdef _DEBUG
    if (Input::GetInstance()->IsKeyTriggered(DIK_RETURN) &&
        Input::GetInstance()->IsKeyPressed(DIK_RSHIFT))
        enableDebugCamera_ = !enableDebugCamera_;

    if (ImGui::Button("rot"))
    {
        aModel_->ChangeAnimation("RotateAnim", 0.5f, true);
    }

    if (ImGui::Button("scale"))
    {
        aModel_->ChangeAnimation("ScaleAnim", 0.5f);
    }

    //ImGuiTool::TimeLine("TimeLine", sequence_.get());

    lights_->ImGui();
    static float volume = 1.0f;
    ImGui::DragFloat("Volume", &volume, 0.01f, 0.0f, 1.0f);
    if (ImGui::Button("Apply##volme"))
    {
        voiceInstance_->SetVolume(volume);
    }

    static bool play = false;
    ImGui::Checkbox("Play", &play);

    if (play)
        testColor_= sequence_->GetValue<Vector4>("color");

    //emitter_->ShowDebugWindow();


#endif // _DEBUG

    static bool isSta = false;
    ImGui::Checkbox("Static", &isSta);

    size_t loop = colliders_.size() - s_colliders_.size();
    if(isSta)
        loop+= s_colliders_.size();
    ImGui::Text("Collider Count: %d", loop);

    for (int i = 0; i < loop; ++i)
    {
        CollisionManager::GetInstance()->RegisterCollider(colliders_[i]);
    }


    /*test_->Update();
    oModel_->Update();
    oModel2_->Update();
    aModel_->Update();
    plane_->Update();
    sprite_->Update();*/

    if (input_->IsKeyTriggered(DIK_TAB))
    {
        //SceneManager::GetInstance()->ReserveScene("ParticleTest");
    }

    if (enableDebugCamera_)
    {
        debugCamera_.Update();
        SceneCamera_.matView_ = debugCamera_.matView_;
        SceneCamera_.TransferData();
        //ParticleManager::GetInstance()->Update(debugCamera_.rotate_);
    }
    else
    {
        SceneCamera_.Update();
        SceneCamera_.UpdateMatrix();
    }

    ParticleSystem::GetInstance()->Update();
    CollisionManager::GetInstance()->Update();
}

void SampleScene::Draw()
{
    //skyBox_->Draw(&SceneCamera_);

    ModelManager::GetInstance()->PreDrawForObjectModel();

    skyBox_->QueueCmdCubeTexture();

    int i = 0;
    for (; i < models_.size(); ++i)
    {
        models_[i]->Draw(&SceneCamera_, 0, testColor_);
    }
    for (int j = 0; j < s_models_.size(); ++j)
    {
        s_models_[j]->Draw(&SceneCamera_, 0, { .7f,.7f,.7f,1 });
    }


    //oModel_->Draw(&SceneCamera_, testColor_);
    //oModel2_->Draw(&SceneCamera_, 0 ,{ 1, 1, 1, 1 });
    //plane_->Draw(&SceneCamera_, { 1,1,1,1 });

    ////aModel_->Draw(&SceneCamera_, { 1,1,1,1 });

    //test_->Draw(&SceneCamera_, 0, { 1,1,1,1 });

    //Sprite::PreDraw();
    //sprite_->Draw();


    //button_->Draw();

    ParticleSystem::GetInstance()->DrawParticles();

}

void SampleScene::DrawShadow()
{
    for (int i = 0; i < models_.size(); ++i)
    {
        models_[i]->DrawShadow(&SceneCamera_, 0);
    }

    //oModel_->DrawShadow(&SceneCamera_, 0);
    //oModel2_->DrawShadow(&SceneCamera_, 1);
    //aModel_->DrawShadow(&SceneCamera_, 2);

}

void SampleScene::Create()
{
    auto rand = RandomGenerator::GetInstance();

    for (int i = 0; i < 100; ++i)
    {
        ObjectModel* model = new ObjectModel("test");
        model->Initialize("Cube/Cube.obj");
        model->translate_.x = rand->GetRandValue(-0.0f, 80.0f);
        model->translate_.y = 0.0f;
        model->translate_.z = rand->GetRandValue(-0.0f, 80.0f);

        model->Update();

        AABBCollider* collider = new AABBCollider("test");
        collider->SetLayer("temp");
        collider->SetWorldTransform(model->GetWorldTransform());
        collider->SetMinMax(model->GetMin(), model->GetMax());

        models_.push_back(model);
        colliders_.push_back(collider);
    }

    for (int i = 0; i < 5; i++)
    {
        ObjectModel* model = new ObjectModel("test");
        model->Initialize("Cube/Cube.obj");
        model->translate_.x = rand->GetRandValue(-0.0f, 80.0f);
        model->translate_.y = -3.0f;
        model->translate_.z = rand->GetRandValue(-0.0f, 80.0f);
        model->scale_ = { 10,1,10 };
        model->scale_.x *= rand->GetRandValue(0.5f, 2.0f);
        model->scale_.z *= rand->GetRandValue(0.5f, 2.0f);

        model->Update();

        AABBCollider* collider = new AABBCollider("static_");
        collider->SetLayer("static_");
        collider->SetWorldTransform(model->GetWorldTransform());
        collider->SetMinMax(model->GetMin(), model->GetMax());

        AABBCollider* collider2 = new AABBCollider("s_temp");
        collider2->SetLayer("s_temp");
        collider2->SetWorldTransform(model->GetWorldTransform());
        collider2->SetMinMax(model->GetMin(), model->GetMax());

        s_models_.push_back(model);
        colliders_.push_back(collider2);
        s_colliders_.push_back(collider);
        CollisionManager::GetInstance()->RegisterStaticCollider(s_colliders_.back());
    }

    {
        ObjectModel* model = new ObjectModel("test");
        model->Initialize("Cube/Cube.obj");
        model->translate_.x = 40.0f;
        model->translate_.y = -3.0f;
        model->translate_.z = 40.0f;
        model->scale_ = { 80,1,80 };
        model->Update();

        AABBCollider* collider = new AABBCollider("static_");
        collider->SetLayer("static_");
        collider->SetWorldTransform(model->GetWorldTransform());
        collider->SetMinMax(model->GetMin(), model->GetMax());

        AABBCollider* collider2 = new AABBCollider("s_temp");
        collider2->SetLayer("s_temp");
        collider2->SetWorldTransform(model->GetWorldTransform());
        collider2->SetMinMax(model->GetMin(), model->GetMax());

        s_models_.push_back(model);
        colliders_.push_back(collider2);
        s_colliders_.push_back(collider);
        CollisionManager::GetInstance()->RegisterStaticCollider(s_colliders_.back());
    }

}

#ifdef _DEBUG
void SampleScene::ImGui()
{

}
#endif // _DEBUG
