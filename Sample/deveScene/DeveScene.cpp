#include "DeveScene.h"

#include <Core/DXCommon/TextureManager/TextureManager.h>

#include <Features/Scene/Manager/SceneManager.h>
#include <Features/Sprite/Sprite.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Features/Model/Primitive/Triangle.h>
#include <Features/Model/Primitive/Plane.h>
#include <Features/Collision/Manager/CollisionManager.h>
#include <Features/PostEffects/DepthBasedOutLine.h>

#include <Debug/ImGuiDebugManager.h>
#include <Debug/ImGuiHelper.h>
#include  <Core/DXCommon/DXCommon.h>


#include "LevelEditorLoader.h"

DeveScene::~DeveScene()
{
}

void DeveScene::Initialize(SceneData* _sceneData)
{

    // --------------------------------------------------
    // シーン関連 初期化
    SceneCamera_.Initialize();
    SceneCamera_.translate_ = { 0,5,-20 };
    SceneCamera_.rotate_ = { 0.26f,0,0 };
    SceneCamera_.UpdateMatrix();
    debugCamera_.Initialize();


    lineDrawer_ = LineDrawer::GetInstance();
    //lineDrawer_->Initialize();
    lineDrawer_->SetCameraPtr(&SceneCamera_);

    input_ = Input::GetInstance();

    // パーティクルシステムの初期化
    // カメラのポイントを設定する
    ParticleSystem::GetInstance()->SetCamera(&SceneCamera_);


    // ライトの設定
    // ライトグループの初期化 (関数内でDLは初期化される)
    lights_ = std::make_shared<LightGroup>();
    lights_->Initialize();

    // DLを取得して初期化 (任意)
    auto DL = lights_->GetDirectionalLight();
    DL->SetDirection(Vector3(-1.0f, -1.0f, 0.0f).Normalize());

    // ライトの追加(任意)
    // 追加するライトを初期化する
    std::shared_ptr<PointLightComponent> pointLight = std::make_shared<PointLightComponent>();
    pointLight->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    pointLight->SetIntensity(1.0f);
    pointLight->SetRadius(30.0f);
    pointLight->SetPosition({ 0.0f, 5.0f, 0.0f });

    // ライトグループにポイントライトを追加する
    lights_->AddPointLight("pointlight", pointLight);

    // アクティブなライトグループを設定する
    LightingSystem::GetInstance()->SetActiveGroup(lights_);



    // --------------------------------------------------
    // シーン固有の初期化



    // 地面ようのいたポリを生成する
    Plane groundPlane;
    groundPlane.SetSize(Vector2{ 100, 100 });   //サイズは100*100
    groundPlane.SetPivot({ 0,0,0 });        // 基準点は中央
    groundPlane.SetNormal({ 0,1,0 });       //上向き
    groundPlane.SetFlipU(false);            // U軸の反転はなし
    groundPlane.SetFlipV(false);            // V軸の反転はなし

    // 任意の名前で生成する
    groundPlane.Generate("groundPlane");

    // 地面を生成する
    ground_ = std::make_unique<ObjectModel>("ground");
    // 生成した板ポリを使用する (生成時に設定した名前を渡す)
    ground_->Initialize("groundPlane");
    // UV変換を設定する
    ground_->GetUVTransform().SetScale({ 10,10 });

    // 地面のテクスチャを読み込む 描画時に使用する
    groundTextureHandle_ = TextureManager::GetInstance()->Load("tile.png");


    // 2Dスプライトの初期化
    uint32_t textureHandle = TextureManager::GetInstance()->Load("uvChecker.png");
    sprite_ = Sprite::Create("uvChecker", textureHandle);


    // 音声データの読み込み
    soundInstance_ = AudioSystem::GetInstance()->Load("Resources/Sounds/Alarm01.wav");

    skyBox_ = std::make_unique<SkyBox>();
    skyBox_->Initialize(30.0f);
    skyBox_->SetTexture("rosendal_plains_2_2k.dds");

    Rand();
    

    textRenderer_ = TextRenderer::GetInstance();
}

void DeveScene::Update()
{
    textRenderer_->BeginFrame();
    // シーン関連更新
    static std::string str = "Hello, World!";
#ifdef _DEBUG

    // デバッグカメラ
    if (Input::GetInstance()->IsKeyTriggered(DIK_F1))
        enableDebugCamera_ = !enableDebugCamera_;


    {
        if(ImGuiDebugManager::GetInstance()->Begin("Engine"))
        {
            // サウンドの再生
            static float volume = 1.0f;
            if (ImGui::Button("play Sound"))
            {
                if (soundInstance_)
                {
                    // 返り値で VoiceInstanceを受け取る
                    // これを使用して音量やピッチの変更ができる
                    voiceInstance_ = soundInstance_->Play(volume);
                }
            }

            if (ImGui::DragFloat("Volume", &volume, 0.0f, 1.0f))
            {
                if (voiceInstance_)
                {
                    voiceInstance_->SetVolume(volume);
                }
            }
            if (ImGui::Button("Stop Sound"))
            {
                if (voiceInstance_)
                {
                    voiceInstance_->Stop();
                    voiceInstance_ = nullptr; // VoiceInstanceを解放
                }
            }
            ImGui::End();
        }



        // light調整用
        lights_->ImGui();

    }

    if (input_->IsKeyTriggered(DIK_F2))
    {
        // ランダムにモデルを生成
        Rand();
    }

#endif // _DEBUG

    for (auto& col : colliders_)
        CollisionManager::GetInstance()->RegisterCollider(col.get());


    // モデルの更新
    ground_->Update();

    for (const auto& model : models_)
    {
        model->Update();
    }

    std::wstring wstr = std::wstring(str.begin(), str.end());
    wstr += L"\nこんにちは 世界";
    textRenderer_->DrawText(wstr, Vector2{ 100, 100 }, Vector4{ 1, 1, 1, 1 });

    // --------------------------------
    // シーン共通更新処理

    if (enableDebugCamera_)
    {
        debugCamera_.Update();
        SceneCamera_.matView_ = debugCamera_.matView_;
        SceneCamera_.TransferData();
    }
    else
    {
        SceneCamera_.Update();
        SceneCamera_.UpdateMatrix();
    }

    ParticleSystem::GetInstance()->Update();
    CollisionManager::GetInstance()->Update();
}

void DeveScene::Draw()
{
    // skyBooxの描画
    //skyBox_->Draw(&SceneCamera_);

    // Model描画用のPSO等をセット
    ModelManager::GetInstance()->PreDrawForObjectModel();

    // SkyBoxのキューブマップを描画キューに追加(任意)
    //skyBox_->QueueCmdCubeTexture();

    // groundの描画
    ground_->Draw(&SceneCamera_, groundTextureHandle_, drawColor_);

    for (const auto& model : models_)
    {
        model->Draw(&SceneCamera_, 0, drawColor_);
    }

    // Sprite用のPSO等をセット
    Sprite::PreDraw();
    // スプライトの描画
    sprite_->Draw();


    ParticleSystem::GetInstance()->DrawParticles();
    textRenderer_->EndFrame();
}

void DeveScene::DrawShadow()
{
}

void DeveScene::Rand()
{
    models_.clear();
    colliders_.clear();

    RandomGenerator* rand = RandomGenerator::GetInstance();
    for (size_t i = 0; i < 100; ++i)
    {
        auto model = std::make_unique<ObjectModel>("model");
        model->Initialize("cube/cube.obj");
        model->translate_ = rand->GetRandValue(Vector3(-48, 1, -48), Vector3(48, 1, 48));

        model->Update();


        auto col = std::make_unique<OBBCollider>("col");
        col->Initialize();
        col->SetHalfExtents((model->GetMax() - model->GetMin()) * 0.5f);
        col->SetCollisionLayer("box");
        col->SetWorldTransform(model->GetWorldTransform());


        models_.push_back(std::move(model));
        colliders_.push_back(std::move(col));
    }

    auto model = std::make_unique<ObjectModel>("model");
    model->Initialize("cube/cube.obj");
    model->translate_ = rand->GetRandValue(Vector3(-0, 0, -0), Vector3(0, 0, 0));
    model->scale_ = Vector3(20, 1, 10);
    model->Update();


    auto col = std::make_unique<OBBCollider>("col");
    col->Initialize();
    col->SetHalfExtents((model->GetMax() - model->GetMin()) * 0.5f);
    col->SetCollisionLayer("box");
    col->SetWorldTransform(model->GetWorldTransform());

    models_.push_back(std::move(model));
    colliders_.push_back(std::move(col));
}
