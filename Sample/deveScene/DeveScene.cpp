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
#include <Debug/Debug.h>
#include <Features/UI/Collider/UICollisionManager.h>
#include <Features/UI/UINavigationManager.h>


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

    text_.Initialize(FontConfig());

    //uiGroup_ = std::make_unique<UIGroup>();
    //uiGroup_->Initialize();
    //auto button = uiGroup_->CreateButton("TestButton", L"ボタン");
    //button->SetPos({ 100,300 });
    //button->SetSize({ 200,100 });
    //button->SetOnClick([]()
    //                   {
    //                       Debug::Log("Button Clicked");
    //                   });

    //auto slider = uiGroup_->CreateSlider("TestSlider", 0.0f, 100.0f);
    //slider->SetPos({ 100,450 });
    //slider->SetSize({ 300,50 });
    //slider->SetValue(50.0f);
    //slider->SetOnValueChanged([](float value)
    //                          {
    //                              Debug::Log("Slider Value: " + std::to_string(value));
    //                          });

    //auto sprite = uiGroup_->CreateSprite("TestSprite", L"スプライト");
    //sprite->SetPos({ 500,300 });
    //sprite->SetSize({ 128,128 });
    //sprite->SetTextureNameAndLoad("uvChecker.png");

    // --------------------------------------------------
    // UIRenderComponent テスト

    // 1. UITextRenderComponent のテスト
    testTextElement_ = std::make_unique<UIElement>("TestText");
    testTextElement_->SetPosition(Vector2(100.0f, 50.0f));
    testTextElement_->SetSize(Vector2(400.0f, 50.0f));
    testTextElement_->SetOrder(100);

    FontConfig fontConfig;
    fontConfig.fontFilePath = "Resources/Fonts/NotoSansJP-Regular.ttf";
    fontConfig.fontSize = 32;

    auto* textComp = testTextElement_->AddComponent<UITextRenderComponent>(
        testTextElement_.get(),
        "UITextRenderComponent Test",
        fontConfig
    );

    TextParam textParam;
    textParam.position = Vector2(0.0f, 0.0f);
    textParam.bottomColor = Vector4(1.0f, 1.0f, 0.0f, 1.0f);  // 黄色
    textParam.scale = { 1.0f,1.0f };
    textComp->SetTextParam(textParam);

    // 2. UISpriteRenderComponent のテスト
    testSpriteElement_ = std::make_unique<UIElement>("TestSprite");
    testSpriteElement_->SetPosition(Vector2(500.0f, 200.0f));
    testSpriteElement_->SetSize(Vector2(128.0f, 128.0f));
    testSpriteElement_->SetOrder(50);

    auto* spriteComp = testSpriteElement_->AddComponent<UISpriteRenderComponent>(
        testSpriteElement_.get(),
        "uvChecker.png"
    );
    spriteComp->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));

    // 3. 背景付きテキストのテスト（親子関係）
    testBackgroundElement_ = std::make_unique<UIElement>("Background");
    testBackgroundElement_->SetPosition(Vector2(100.0f, 500.0f));
    testBackgroundElement_->SetSize(Vector2(400.0f, 100.0f));
    testBackgroundElement_->SetOrder(10);

    auto* bgSprite = testBackgroundElement_->AddComponent<UISpriteRenderComponent>(
        testBackgroundElement_.get(),
        "white.png"
    );
    bgSprite->SetColor(Vector4(0.2f, 0.2f, 0.8f, 0.8f));  // 半透明の青

    // 子要素（テキスト）
    auto childText = std::make_unique<UIElement>("ChildText");
    childText->SetPosition(Vector2(20.0f, 30.0f));  // 親からの相対位置
    childText->SetSize(Vector2(360.0f, 40.0f));
    childText->SetOrder(20);
    childText->SetAnchor(Vector2(0.0f, 0.0f));  // アンカーを左上に設定

    FontConfig childFont;
    childFont.fontFilePath = "Resources/Fonts/NotoSansJP-Regular.ttf";
    childFont.fontSize = 24;

    auto* childTextComp = childText->AddComponent<UITextRenderComponent>(
        childText.get(),
        "Background + Text Test",
        childFont
    );

    TextParam childParam;
    childParam.bottomColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);  // 白色
    childTextComp->SetTextParam(childParam);

    // 子要素を追加
    auto* addedChild = testBackgroundElement_->AddChild(std::move(childText));
    // 子要素のInitializeを呼び出す
    addedChild->Initialize();

    // --------------------------------------------------
    // UIButtonElement テスト

    // ボタン1: クリックでログ出力
    testButton1_ = std::make_unique<UIButtonElement>(
        "TestButton1",
        Vector2(700.0f, 100.0f),
        Vector2(200.0f, 60.0f),
        "Click Me!"
    );
    testButton1_->Initialize();
    testButton1_->SetOnClick([]() {
        Debug::Log("Button 1 Clicked!");
    });
    testButton1_->SetNormalColor(Vector4(0.2f, 0.7f, 0.2f, 1.0f));   // 緑
    testButton1_->SetHoverColor(Vector4(0.3f, 0.9f, 0.3f, 1.0f));    // 明るい緑
    testButton1_->SetPressedColor(Vector4(0.1f, 0.5f, 0.1f, 1.0f));  // 暗い緑

    // ボタン2: クリックで色変更
    testButton2_ = std::make_unique<UIButtonElement>(
        "TestButton2",
        Vector2(920.0f, 100.0f),
        Vector2(200.0f, 60.0f),
        "Toggle Color"
    );
    testButton2_->Initialize();
    static bool colorToggle = false;
    testButton2_->SetOnClick([this]() {
        colorToggle = !colorToggle;
        if (colorToggle)
        {
            testButton2_->SetNormalColor(Vector4(0.7f, 0.2f, 0.2f, 1.0f));  // 赤
            Debug::Log("Button 2: Red");
        }
        else
        {
            testButton2_->SetNormalColor(Vector4(0.2f, 0.2f, 0.7f, 1.0f));  // 青
            Debug::Log("Button 2: Blue");
        }
    });
    testButton2_->SetNormalColor(Vector4(0.2f, 0.2f, 0.7f, 1.0f));   // 青
    testButton2_->SetHoverColor(Vector4(0.4f, 0.4f, 0.9f, 1.0f));    // 明るい青
    testButton2_->SetPressedColor(Vector4(0.1f, 0.1f, 0.5f, 1.0f));  // 暗い青
    testButton2_->SetTextAlignment(TextAlignment::Center);

    // ボタン3: 左下
    testButton3_ = std::make_unique<UIButtonElement>(
        "TestButton3",
        Vector2(700.0f, 300.0f),
        Vector2(200.0f, 60.0f),
        "Button 3"
    );
    testButton3_->Initialize();
    testButton3_->SetOnClick([]() {
        Debug::Log("Button 3 Clicked!");
    });
    testButton3_->SetNormalColor(Vector4(0.7f, 0.5f, 0.2f, 1.0f));   // オレンジ
    testButton3_->SetHoverColor(Vector4(0.9f, 0.7f, 0.3f, 1.0f));    // 明るいオレンジ
    testButton3_->SetPressedColor(Vector4(0.5f, 0.3f, 0.1f, 1.0f));  // 暗いオレンジ

    // ボタン4: 右下
    testButton4_ = std::make_unique<UIButtonElement>(
        "TestButton4",
        Vector2(920.0f, 300.0f),
        Vector2(200.0f, 60.0f),
        "Button 4"
    );
    testButton4_->Initialize();
    testButton4_->SetOnClick([]() {
        Debug::Log("Button 4 Clicked!");
    });
    testButton4_->SetNormalColor(Vector4(0.7f, 0.2f, 0.7f, 1.0f));   // 紫
    testButton4_->SetHoverColor(Vector4(0.9f, 0.4f, 0.9f, 1.0f));    // 明るい紫
    testButton4_->SetPressedColor(Vector4(0.5f, 0.1f, 0.5f, 1.0f));  // 暗い紫

    // ナビゲーション設定（2x2グリッド）
    // Button1 (左上)
    testButton1_->SetNavigation(NavigationDirection::Down, testButton3_.get());
    testButton1_->SetNavigation(NavigationDirection::Right, testButton2_.get());

    // Button2 (右上)
    testButton2_->SetNavigation(NavigationDirection::Down, testButton4_.get());
    testButton2_->SetNavigation(NavigationDirection::Left, testButton1_.get());

    // Button3 (左下)
    testButton3_->SetNavigation(NavigationDirection::Up, testButton1_.get());
    testButton3_->SetNavigation(NavigationDirection::Right, testButton4_.get());

    // Button4 (右下)
    testButton4_->SetNavigation(NavigationDirection::Up, testButton2_.get());
    testButton4_->SetNavigation(NavigationDirection::Left, testButton3_.get());

    // 初期フォーカスをButton1に設定
    UINavigationManager::GetInstance()->SetFocus(testButton1_.get());

    // --------------------------------------------------
    // UISliderElement テスト

    // スライダー1: ボリューム調整風
    testSlider1_ = std::make_unique<UISliderElement>(
        "TestSlider1",
        Vector2(700.0f, 400.0f),
        Vector2(300.0f, 10.0f)  // トラックサイズ
    );
    testSlider1_->Initialize();
    testSlider1_->SetValue(50.0f);
    testSlider1_->SetOnValueChanged([](float value) {
        Debug::Log("Slider Value: " + std::to_string(value));
    });

    // --------------------------------------------------
    // UIImageElement テスト

    testImage1_ = std::make_unique<UIImageElement>(
        "TestImage1",
        Vector2(100.0f, 650.0f),
        Vector2(128.0f, 128.0f),
        "uvChecker.png"
    );
    testImage1_->Initialize();
    testImage1_->SetColor(Vector4(1.0f, 0.5f, 0.5f, 1.0f));  // 赤みがかった色

    // --------------------------------------------------
    // UITextElement テスト

    FontConfig testFontConfig;
    testFontConfig.fontFilePath = "Resources/Fonts/NotoSansJP-Regular.ttf";
    testFontConfig.fontSize = 28;

    testText1_ = std::make_unique<UITextElement>(
        "TestText1",
        Vector2(250.0f, 650.0f),
        "UITextElement Test!",
        testFontConfig
    );
    testText1_->Initialize();
    testText1_->SetColor(Vector4(0.2f, 0.8f, 0.2f, 1.0f));  // 緑色

    // 注: スライダーの色設定は自動的に保存されます（UISliderElement内でJsonBinderを使用）
}

void DeveScene::Update()
{
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


#endif // _DEBUG


    // モデルの更新
    ground_->Update();
    //uiGroup_->Update();

    std::wstring wstr = std::wstring(str.begin(), str.end());
    wstr += L"\nこんにちは 世界";
    //text_.Draw(wstr, Vector2{ 100, 100 }, Vector4{ 1, 1, 1, 1 });

    // UIRenderComponent テストの更新
    if (testTextElement_)
    {
        testTextElement_->Update();
        if (auto* comp = testTextElement_->GetComponent<UITextRenderComponent>())
            comp->Update();
    }

    if (testSpriteElement_)
    {
        testSpriteElement_->Update();
        if (auto* comp = testSpriteElement_->GetComponent<UISpriteRenderComponent>())
            comp->Update();
    }

    if (testBackgroundElement_)
    {
        testBackgroundElement_->Update();
        if (auto* comp = testBackgroundElement_->GetComponent<UISpriteRenderComponent>())
            comp->Update();

        // 子要素の更新
        for (const auto& child : testBackgroundElement_->GetChildren())
        {
            child->Update();
            if (auto* childComp = child->GetComponent<UITextRenderComponent>())
                childComp->Update();
        }
    }

    // UIButtonElement テストの更新
    if (testButton1_)
        testButton1_->Update();

    if (testButton2_)
        testButton2_->Update();

    if (testButton3_)
        testButton3_->Update();

    if (testButton4_)
        testButton4_->Update();

    // UISliderElement テストの更新
    if (testSlider1_)
        testSlider1_->Update();

    // UIImageElement テストの更新
    if (testImage1_)
        testImage1_->Update();

    // UITextElement テストの更新
    if (testText1_)
        testText1_->Update();

    // UICollisionManagerでマウス衝突判定
    Vector2 mousePos = input_->GetMousePosition();
    UICollisionManager::GetInstance()->CheckCollision(mousePos);

    // UINavigationManagerで入力処理
    UINavigationManager::GetInstance()->HandleInput();

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

    // Sprite用のPSO等をセット
    Sprite::PreDraw();
    // スプライトの描画
    sprite_->Draw();
    //uiGroup_->Draw();

    // UIRenderComponent テストの描画
    if (testSpriteElement_)
    {
        if (auto* comp = testSpriteElement_->GetComponent<UISpriteRenderComponent>())
            comp->Draw();
    }

    if (testBackgroundElement_)
    {
        if (auto* comp = testBackgroundElement_->GetComponent<UISpriteRenderComponent>())
            comp->Draw();
    }

    if (testTextElement_)
    {
        if (auto* comp = testTextElement_->GetComponent<UITextRenderComponent>())
            comp->Draw();
    }

    if (testBackgroundElement_)
    {
        // 子要素の描画
        for (const auto& child : testBackgroundElement_->GetChildren())
        {
            if (auto* childComp = child->GetComponent<UITextRenderComponent>())
                childComp->Draw();
        }
    }

    // UIButtonElement テストの描画
    if (testButton1_)
        testButton1_->Draw();

    if (testButton2_)
        testButton2_->Draw();

    if (testButton3_)
        testButton3_->Draw();

    if (testButton4_)
        testButton4_->Draw();

    // UISliderElement テストの描画
    if (testSlider1_)
        testSlider1_->Draw();

    // UIImageElement テストの描画
    if (testImage1_)
        testImage1_->Draw();

    // UITextElement テストの描画
    if (testText1_)
        testText1_->Draw();

    ParticleSystem::GetInstance()->DrawParticles();
}

void DeveScene::DrawShadow()
{
}
