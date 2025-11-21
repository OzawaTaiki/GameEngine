#include "EffectEditorScene.h"
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <Debug/ImGuiManager.h>
#include <System/Time/Time.h>
#include <algorithm>
#include <filesystem>
#include <Framework/LayerSystem/LayerSystem.h>
#include <Core/WinApp/WinApp.h>

EffectEditorScene::~EffectEditorScene()
{
#ifdef _DEBUG
    effects_.clear();
#endif
}

void EffectEditorScene::Initialize([[maybe_unused]] SceneData* _sceneData)
{
#ifdef _DEBUG
    // カメラ初期化
    sceneCamera_.Initialize();
    sceneCamera_.translate_ = { 0, 5, -10 };
    sceneCamera_.rotate_ = { 0.2f, 0, 0 };
    sceneCamera_.UpdateMatrix();

    debugCamera_.Initialize();

    LineDrawer::GetInstance()->SetCameraPtr(&sceneCamera_);

    // システム初期化
    input_ = Input::GetInstance();
    particleSystem_ = ParticleSystem::GetInstance();
    particleSystem_->SetCamera(&sceneCamera_);

    // ライト初期化
    lights_ = std::make_shared<LightGroup>();
    lights_->Initialize();

    auto DL = lights_->GetDirectionalLight();
    DL->SetDirection(Vector3(-1.0f, -1.0f, 0.0f).Normalize());

    LightingSystem::GetInstance()->SetActiveGroup(lights_);

    // スカイボックス初期化
    //skyBox_ = std::make_unique<SkyBox>();
    //skyBox_->Initialize(30.0f);
    //skyBox_->SetTexture("rosendal_plains_2_2k.dds");

    // デフォルトエフェクト作成
    auto defaultEffect = Effect::Create("DefaultEffect");
    if (defaultEffect)
    {
        effects_.emplace_back(std::move(defaultEffect));
        selectedEffectIndex_ = 0;
    }

    // テンプレート初期化
    InitializeTemplates();

    // フレーム履歴初期化
    frameTimeHistory_.resize(kFrameHistorySize, 0.0f);

    uint32_t texture = TextureManager::GetInstance()->Load("white.png");
    backgroundSprite_ = Sprite::Create("particle_Editor_back", texture,{0.0f,0.0f });
    backgroundSprite_->scale_ = WinApp::kWindowSize_;

    LayerSystem::Initialize();
    LayerSystem::CreateLayer("EffectEditorScene_Layer_0", 0, PSOFlags::BlendMode::Normal);
    LayerSystem::CreateLayer("EffectEditorScene_Layer_1", 1, PSOFlags::BlendMode::Normal);

#endif
}

void EffectEditorScene::Update()
{
#ifdef _DEBUG

    // ホットキー処理
    ProcessHotKeys();

    // カメラ更新
    UpdateCamera();

    // パフォーマンスデータ更新
    UpdatePerformanceData();

    for (auto& model : objectModels_)
        model->Update();

    // プレビュー時間更新
    if (isPlaying_ && !isPaused_)
    {
        previewTime_ += Time::GetDeltaTime<float>() * playbackSpeed_;
    }

    // エフェクト更新
    Effect* currentEffect = GetCurrentEffect();
    if (currentEffect && isPlaying_ && !isPaused_)
    {
        currentEffect->Update();

        // ループ処理
        if (currentEffect->IsComplete() && isLooping_)
        {
            currentEffect->Reset();
            currentEffect->Play();
            previewTime_ = 0.0f;
        }
    }

    // パーティクルシステム更新
    particleSystem_->Update();
#endif
}

void EffectEditorScene::Draw()
{
#ifdef _DEBUG
    LayerSystem::SetLayer("EffectEditorScene_Layer_0");

    Sprite::PreDraw();
    backgroundSprite_->Draw();

    LayerSystem::SetLayer("EffectEditorScene_Layer_1");

    // 背景描画
    if (usesSkyBox_ && skyBox_)
    {
        skyBox_->Draw(&sceneCamera_);
    }

    // モデル描画用PSO設定
    ModelManager::GetInstance()->PreDrawForObjectModel();
    if (skyBox_)
    {
        skyBox_->QueueCmdCubeTexture();
    }

    for (auto& model : objectModels_)
        model->Draw(&sceneCamera_);

    // グリッド・ガイド描画
    if (showGrid_) DrawGrid();
    if (showAxis_) DrawAxis();
    if (showBounds_) DrawBounds();

    // パーティクル描画
    particleSystem_->DrawParticles();

    // ImGui描画
    ImGuiMainMenuBar();

    if (showEffectLibrary_)
        ImGuiEffectLibrary();

    if (showEffectEditor_)
        ImGuiEffectEditor();

    if (showEmitterEditor_)
        ImGuiEmitterEditor();

    if (showPreviewControls_)
        ImGuiPreviewControls();

    if (showPerformanceMonitor_)
        ImGuiPerformanceMonitor();

    ImGuiModelControls();

    primitiveCreator_.DrawImGui();
#endif
}

void EffectEditorScene::DrawShadow()
{
    // シャドウ描画が必要な場合はここに実装
}

#ifdef _DEBUG
void EffectEditorScene::ImGuiMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Effect", "Ctrl+N"))
            {
                auto newEffect = Effect::Create("NewEffect");
                if (newEffect)
                {
                    effects_.emplace_back(std::move(newEffect));
                    selectedEffectIndex_ = static_cast<int>(effects_.size()) - 1;
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Save Effect", "Ctrl+S"))
            {
                SaveCurrentEffect();
            }

            if (ImGui::MenuItem("Save As..."))
            {
                // ファイルダイアログ表示
            }

            if (ImGui::MenuItem("Load Effect", "Ctrl+O"))
            {
                // ファイルダイアログ表示
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("Recent Files"))
            {
                for (const auto& file : recentFiles_)
                {
                    if (ImGui::MenuItem(std::filesystem::path(file).filename().string().c_str()))
                    {
                        LoadEffectFromFile(file);
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit"))
            {
                // シーン終了処理
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Duplicate Effect"))
            {
                DuplicateCurrentEffect();
            }

            if (ImGui::MenuItem("Delete Effect"))
            {
                DeleteCurrentEffect();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Copy Emitter Properties", "Ctrl+C"))
            {
                CopyEmitterProperties();
            }

            if (ImGui::MenuItem("Paste Emitter Properties", "Ctrl+V", false, hasClipboardData_))
            {
                PasteEmitterProperties();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Effect Library", nullptr, &showEffectLibrary_);
            ImGui::MenuItem("Effect Editor", nullptr, &showEffectEditor_);
            ImGui::MenuItem("Emitter Editor", nullptr, &showEmitterEditor_);
            ImGui::MenuItem("Preview Controls", nullptr, &showPreviewControls_);
            ImGui::MenuItem("Performance Monitor", nullptr, &showPerformanceMonitor_);

            ImGui::Separator();

            ImGui::MenuItem("Grid", nullptr, &showGrid_);
            ImGui::MenuItem("Axis", nullptr, &showAxis_);
            ImGui::MenuItem("Bounds", nullptr, &showBounds_);

            ImGui::Separator();

            ImGui::MenuItem("Debug Camera", "F1", &enableDebugCamera_);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools"))
        {
            if (ImGui::MenuItem("Reset Camera"))
            {
                sceneCamera_.translate_ = { 0, 5, -10 };
                sceneCamera_.rotate_ = { 0.2f, 0, 0 };
                sceneCamera_.UpdateMatrix();
            }

            if (ImGui::MenuItem("Template Manager"))
            {
                // テンプレート管理ウィンドウを開く
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About"))
            {
                // Aboutダイアログ表示
            }

            if (ImGui::MenuItem("User Guide"))
            {
                // ヘルプウィンドウ表示
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void EffectEditorScene::ImGuiEffectLibrary()
{
    ImGui::Begin("Effect Library", &showEffectLibrary_);

    // 新しいエフェクト作成
    ImGui::InputText("Effect Name", newEffectNameBuf_, sizeof(newEffectNameBuf_));
    if (ImGui::Button("Create Effect"))
    {
        if (strlen(newEffectNameBuf_) > 0)
        {
            auto newEffect = Effect::Create(newEffectNameBuf_);
            if (newEffect)
            {
                effects_.emplace_back(std::move(newEffect));
                selectedEffectIndex_ = static_cast<int>(effects_.size()) - 1;
                memset(newEffectNameBuf_, 0, sizeof(newEffectNameBuf_));
            }
        }
    }

    ImGui::Separator();

    // テンプレートからの作成
    if (ImGui::BeginCombo("Template", "Select Template"))
    {
        for (const auto& tmpl : effectTemplates_)
        {
            if (ImGui::Selectable(tmpl.name.c_str()))
            {
                CreateEffectFromTemplate(tmpl.name);
            }
            if (ImGui::IsItemHovered())
            {
                ShowTooltip(tmpl.description);
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Separator();

    // エフェクト一覧
    ImGui::Text("Effects (%zu)", effects_.size());

    if (ImGui::BeginChild("EffectList", ImVec2(0, 200)))
    {
        for (int i = 0; i < effects_.size(); ++i)
        {
            bool isSelected = (selectedEffectIndex_ == i);

            ImGui::PushID(i);

            // 再生状態アイコン
            const char* statusIcon = "⏸";
            if (effects_[i]->IsPlaying())
            {
                statusIcon = effects_[i]->IsPaused() ? "⏸" : "▶";
            }
            else
            {
                statusIcon = "⏹";
            }

            std::string label = std::string(statusIcon) + " " + effects_[i]->GetName();

            if (ImGui::Selectable(label.c_str(), isSelected))
            {
                SelectEffect(i);
            }

            // 右クリックメニュー
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Play"))
                {
                    selectedEffectIndex_ = i;
                    PlayCurrentEffect();
                }
                if (ImGui::MenuItem("Stop"))
                {
                    selectedEffectIndex_ = i;
                    StopCurrentEffect();
                }
                if (ImGui::MenuItem("Duplicate"))
                {
                    selectedEffectIndex_ = i;
                    DuplicateCurrentEffect();
                }
                if (ImGui::MenuItem("Delete"))
                {
                    selectedEffectIndex_ = i;
                    DeleteCurrentEffect();
                }
                ImGui::EndPopup();
            }

            ImGui::PopID();
        }
    }
    ImGui::EndChild();

    ImGui::Separator();

    // クイックアクション
    if (ImGui::Button("Play All"))
    {
        for (auto& effect : effects_)
        {
            effect->Play();
        }
        isPlaying_ = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop All"))
    {
        for (auto& effect : effects_)
        {
            effect->Stop();
        }
        isPlaying_ = false;
        isPaused_ = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear All"))
    {
        effects_.clear();
        selectedEffectIndex_ = -1;
        selectedEmitterIndex_ = -1;
    }

    ImGui::End();
}

void EffectEditorScene::ImGuiEffectEditor()
{
    ImGui::Begin("Effect Editor", &showEffectEditor_);

    Effect* currentEffect = GetCurrentEffect();
    if (!currentEffect)
    {
        ImGui::Text("No effect selected");
        ImGui::End();
        return;
    }

    // エフェクト基本情報
    if (ImGui::CollapsingHeader("Basic Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // 名前
        static char nameBuf[256];
        strncpy_s(nameBuf, sizeof(nameBuf), currentEffect->GetName().c_str(), _TRUNCATE);
        if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf)))
        {
            currentEffect->SetName(nameBuf);
        }

        // ループ設定
        bool loop = currentEffect->IsLoop();
        if (ImGui::Checkbox("Loop", &loop))
        {
            currentEffect->SetLoop(loop);
            isLooping_ = loop;
        }

        // 可視性
        bool visible = currentEffect->IsVisible();
        if (ImGui::Checkbox("Visible", &visible))
        {
            currentEffect->SetVisible(visible);
        }

        // 再生速度
        float speed = currentEffect->GetPlaybackSpeed();
        if (ImGui::DragFloat("Playback Speed", &speed, 0.01f, 0.0f, 5.0f))
        {
            currentEffect->SetPlaybackSpeed(speed);
            playbackSpeed_ = speed;
        }

        // 開始遅延
        float delay = currentEffect->GetStartDelay();
        if (ImGui::DragFloat("Start Delay", &delay, 0.01f, 0.0f, 10.0f))
        {
            currentEffect->SetStartDelay(delay);
        }
    }

    // 位置・回転・スケール
    if (ImGui::CollapsingHeader("Transform"))
    {
        Vector3 pos = currentEffect->GetPosition();
        if (ImGui::DragFloat3("Position", &pos.x, 0.01f))
        {
            currentEffect->SetPosition(pos);
        }

        Quaternion rot = currentEffect->GetRotation();
        if (ImGui::DragFloat4("Rotation", &rot.x, 0.01f))
        {
            currentEffect->SetRotation(rot);
        }

        Vector3 scale = currentEffect->GetScale();
        if (ImGui::DragFloat3("Scale", &scale.x, 0.01f))
        {
            currentEffect->SetScale(scale);
        }
    }

    // エミッター管理
    if (ImGui::CollapsingHeader("Emitters", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // 新しいエミッター追加
        ImGui::InputText("Emitter Name", newEmitterNameBuf_, sizeof(newEmitterNameBuf_));
        if (ImGui::Button("Add Emitter"))
        {
            if (strlen(newEmitterNameBuf_) > 0)
            {
                if (currentEffect->AddEmitter(newEmitterNameBuf_))
                {
                    memset(newEmitterNameBuf_, 0, sizeof(newEmitterNameBuf_));
                }
                else
                {
                    ShowErrorDialog("Failed to add emitter: " + currentEffect->GetLastError());
                }
            }
        }

        ImGui::Separator();

        // エミッター一覧
        auto emitters = currentEffect->GetEmitters();
        int emitterIndex = 0;
        for (auto* emitter : emitters)
        {
            bool isSelected = (selectedEmitterIndex_ == emitterIndex);

            ImGui::PushID(emitterIndex);

            // アクティブ状態チェックボックス
            bool active = emitter->IsActive();
            if (ImGui::Checkbox("##Active", &active))
            {
                emitter->SetActive(active);
            }
            ImGui::SameLine();

            // エミッター名
            if (ImGui::Selectable(emitter->GetName().c_str(), isSelected))
            {
                SelectEmitter(emitterIndex);
            }

            // 右クリックメニュー
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Emit Once"))
                {
                    emitter->EmitSingle();
                }
                if (ImGui::MenuItem("Duplicate"))
                {
                    // エミッター複製処理
                }
                if (ImGui::MenuItem("Remove"))
                {
                    currentEffect->RemoveEmitter(emitter->GetName());
                    if (selectedEmitterIndex_ >= emitterIndex)
                    {
                        selectedEmitterIndex_--;
                    }
                }
                ImGui::EndPopup();
            }

            ImGui::PopID();
            emitterIndex++;
        }

        ImGui::Separator();

        // バッチ操作
        if (ImGui::Button("Activate All"))
        {
            currentEffect->SetAllEmittersActive(true);
        }
        ImGui::SameLine();
        if (ImGui::Button("Deactivate All"))
        {
            currentEffect->SetAllEmittersActive(false);
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear All"))
        {
            currentEffect->ClearEmitters();
            selectedEmitterIndex_ = -1;
        }
    }

    // 統計情報
    if (ImGui::CollapsingHeader("Statistics"))
    {
        auto stats = currentEffect->GetStatistics();
        ImGui::Text("Total Particles: %u", stats.totalParticles);
        ImGui::Text("Active Particles: %u", stats.activeParticles);
        ImGui::Text("Update Time: %.3f ms", stats.averageUpdateTime);
        ImGui::Text("Draw Time: %.3f ms", stats.averageDrawTime);
        ImGui::Text("Draw Calls: %u", stats.drawCalls);
        ImGui::Text("Emitter Count: %zu", currentEffect->GetEmitterCount());

        if (ImGui::Button("Reset Statistics"))
        {
            currentEffect->ResetStatistics();
        }
    }

    // エラー表示
    if (currentEffect->HasError())
    {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", currentEffect->GetLastError().c_str());
        if (ImGui::Button("Clear Error"))
        {
            currentEffect->ClearError();
        }
    }

    ImGui::End();
}

void EffectEditorScene::ImGuiEmitterEditor()
{
    ImGui::Begin("Emitter Editor", &showEmitterEditor_);

    ParticleEmitter* currentEmitter = GetCurrentEmitter();
    if (!currentEmitter)
    {
        ImGui::Text("No emitter selected");
        ImGui::Text("Select an emitter from the Effect Editor");
        ImGui::End();
        return;
    }

    // エミッターの詳細設定UIを表示
    currentEmitter->ShowDebugWindow();

    ImGui::End();
}

void EffectEditorScene::ImGuiPreviewControls()
{
    ImGui::Begin("Preview Controls", &showPreviewControls_);

    // 再生制御
    ImGui::Text("Playback Control");

    if (ImGui::Button("Play"))
    {
        PlayCurrentEffect();
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause"))
    {
        PauseCurrentEffect();
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
    {
        StopCurrentEffect();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset"))
    {
        ResetCurrentEffect();
    }

    // 状態表示
    ImGui::Separator();
    ImGui::Text("Status: %s", isPlaying_ ? (isPaused_ ? "Paused" : "Playing") : "Stopped");
    ImGui::Text("Time: %.2f", previewTime_);

    Effect* currentEffect = GetCurrentEffect();
    if (currentEffect)
    {
        ImGui::Text("Duration: %.2f", currentEffect->GetDuration());
        ImGui::Text("Progress: %.1f%%", currentEffect->GetProgress() * 100.0f);
    }

    // プレビュー設定
    ImGui::Separator();
    ImGui::Checkbox("Loop", &isLooping_);

    if (ImGui::DragFloat("Speed", &playbackSpeed_, 0.01f, 0.0f, 5.0f))
    {
        SetPlaybackSpeed(playbackSpeed_);
    }

    // カメラ制御
    ImGui::Separator();
    ImGuiCameraControls();

    // 環境設定
    ImGui::Separator();
    ImGuiEnvironmentSettings();

    ImGui::End();
}

void EffectEditorScene::ImGuiPerformanceMonitor()
{
    ImGui::Begin("Performance Monitor", &showPerformanceMonitor_);

    // リアルタイム統計
    ImGui::Text("Performance Statistics");
    ImGui::Text("Frame Time: %.3f ms", performanceData_.frameTime);
    ImGui::Text("Update Time: %.3f ms", performanceData_.updateTime);
    ImGui::Text("Draw Time: %.3f ms", performanceData_.drawTime);
    ImGui::Text("Total Particles: %u", performanceData_.totalParticles);
    ImGui::Text("Active Particles: %u", performanceData_.activeParticles);
    ImGui::Text("Draw Calls: %u", performanceData_.drawCalls);

    // フレーム時間グラフ
    ImGui::Separator();
    ImGui::Text("Frame Time History");
    // フレーム時間グラフ
    ImGui::Separator();
    ImGui::Text("Frame Time History");
    ImGui::PlotLines("##FrameTime", frameTimeHistory_.data(),
        static_cast<int>(frameTimeHistory_.size()),
        0, nullptr, 0.0f, 33.33f, ImVec2(0, 80));

    // パフォーマンス警告
    if (performanceData_.frameTime > 16.66f) // 60FPS閾値
    {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Warning: Frame time exceeds 60FPS target");
    }

    if (performanceData_.activeParticles > 10000)
    {
        ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Warning: High particle count");
    }

    ImGui::End();
}

void EffectEditorScene::ImGuiCameraControls()
{
    ImGui::Text("Camera Control");

    if (ImGui::DragFloat3("Target Position", &cameraTargetPosition_.x, 0.01f))
    {
        UpdateCamera();
    }

    if (ImGui::DragFloat("Distance", &cameraDistance_, 0.1f, 1.0f, 50.0f))
    {
        UpdateCamera();
    }

    if (ImGui::DragFloat("Height", &cameraHeight_, 0.1f, -10.0f, 20.0f))
    {
        UpdateCamera();
    }

    if (ImGui::DragFloat("Rotation Y", &cameraRotationY_, 0.01f))
    {
        UpdateCamera();
    }

    if (ImGui::Checkbox("Auto Rotate", &autoRotateCamera_))
    {
        if (autoRotateCamera_)
        {
            cameraRotationY_ = 0.0f;
        }
    }

    if (ImGui::Button("Reset Camera"))
    {
        cameraTargetPosition_ = { 0, 0, 0 };
        cameraDistance_ = 10.0f;
        cameraHeight_ = 5.0f;
        cameraRotationY_ = 0.0f;
        autoRotateCamera_ = false;
        UpdateCamera();
    }
}

void EffectEditorScene::ImGuiEnvironmentSettings()
{
    ImGui::Text("Environment");

    ImGui::Checkbox("Use SkyBox", &usesSkyBox_);

    if (!usesSkyBox_)
    {
        ImGui::ColorEdit4("Background Color", &backgroundColor_.x);
    }

    ImGui::Separator();

    ImGui::Checkbox("Show Grid", &showGrid_);
    if (showGrid_)
    {
        ImGui::DragFloat("Grid Size", &gridSize_, 0.1f, 0.1f, 10.0f);
        ImGui::DragInt("Grid Count", &gridCount_, 1, 5, 50);
    }

    ImGui::Checkbox("Show Axis", &showAxis_);
    ImGui::Checkbox("Show Bounds", &showBounds_);
}

void EffectEditorScene::ImGuiModelControls()
{
    ImGui::Begin("Model Controls");

    if (ImGui::Button("Create Model"))
    {
        ImGui::OpenPopup("CreateModelPopup");
    }

    if (ImGui::BeginPopupModal("CreateModelPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char modelNameBuf[256] = "NewModel";
        static char modelPathBuf[256] = "cube/cube.obj";
        std::string modelName(modelNameBuf);
        std::string modelPath(modelPathBuf);
        if(ImGui::InputText("Model Name", modelNameBuf, sizeof(modelNameBuf)))
            modelName = modelNameBuf;
        if (ImGui::InputText("Model Path", modelPathBuf, sizeof(modelPathBuf)))
            modelPath = modelPathBuf;

        if (ImGui::Button("Create"))
        {
            if (!modelPath.empty())
            {
                auto newModel = std::make_unique<ObjectModel>(modelNameBuf);
                newModel->Initialize(modelPathBuf);
                if (newModel)
                {
                    objectModels_.emplace_back(std::move(newModel));
                    // モデル作成成功
                    ImGui::CloseCurrentPopup();
                }
                else
                {
                    ImGui::Text("Failed to create model: %s", modelNameBuf);
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Separator();

    for (auto it = objectModels_.begin(); it != objectModels_.end();)
    {
        auto& model = *it;
        ImGui::PushID(model.get());
        bool isDelete = ImGui::Button("Delete");

        ImGui::SameLine();
        ImGui::Text("Model: %s", model->GetName().c_str());

        if (isDelete)
            it = objectModels_.erase(it);
        else
            ++it;

        ImGui::PopID();
    }


    ImGui::End();
}

// ヘルパー関数の実装
void EffectEditorScene::CreateEffectFromTemplate(const std::string& _templateName)
{
    auto it = std::find_if(effectTemplates_.begin(), effectTemplates_.end(),
        [&_templateName](const EffectTemplate& _tmpl) {
            return _tmpl.name == _templateName;
        });

    if (it != effectTemplates_.end())
    {
        auto newEffect = Effect::Create(_templateName + "_Copy");
        if (newEffect)
        {
            // テンプレートからエミッターを追加
            for (const auto& emitterTemplate : it->emitterTemplates)
            {
                newEffect->AddEmitter(emitterTemplate);
            }

            effects_.emplace_back(std::move(newEffect));
            selectedEffectIndex_ = static_cast<int>(effects_.size()) - 1;
        }
    }
}

void EffectEditorScene::UpdatePerformanceData()
{
    // フレーム時間測定
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto frameTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastTime);
    lastTime = currentTime;

    performanceData_.frameTime = frameTime.count() / 1000.0f; // ms

    // フレーム履歴更新
    frameTimeHistory_.erase(frameTimeHistory_.begin());
    frameTimeHistory_.push_back(performanceData_.frameTime);

    // エフェクト統計収集
    performanceData_.totalParticles = 0;
    performanceData_.activeParticles = 0;
    performanceData_.drawCalls = 0;

    for (const auto& effect : effects_)
    {
        if (effect)
        {
            auto stats = effect->GetStatistics();
            performanceData_.totalParticles += stats.totalParticles;
            performanceData_.activeParticles += stats.activeParticles;
            performanceData_.drawCalls += stats.drawCalls;
            performanceData_.updateTime += stats.averageUpdateTime;
            performanceData_.drawTime += stats.averageDrawTime;
        }
    }
}

void EffectEditorScene::UpdateCamera()
{
    if (enableDebugCamera_)
    {
        debugCamera_.Update();
        sceneCamera_.matView_ = debugCamera_.matView_;
        sceneCamera_.TransferData();
        return;
    }

    // 自動回転
    if (autoRotateCamera_)
    {
        cameraRotationY_ += Time::GetDeltaTime<float>() * 0.5f; // 0.5 rad/sec
    }

    Vector3 cameraPos;
    cameraPos.x = cameraTargetPosition_.x + cameraDistance_ * std::sinf(cameraRotationY_);
    cameraPos.y = cameraTargetPosition_.y + cameraHeight_;
    cameraPos.z = cameraTargetPosition_.z + cameraDistance_ * std::cosf(cameraRotationY_);
    sceneCamera_.translate_ = cameraPos;

    // カメラの向き計算
    Vector3 direction = cameraTargetPosition_ - cameraPos;
    direction = direction.Normalize();

    sceneCamera_.rotate_.y = std::atan2f(direction.x, direction.z);
    sceneCamera_.rotate_.x = std::asinf(-direction.y);

    sceneCamera_.UpdateMatrix();
}

void EffectEditorScene::ProcessHotKeys()
{
    // デバッグカメラ切り替え
    if (input_->IsKeyTriggered(DIK_F1))
    {
        enableDebugCamera_ = !enableDebugCamera_;
    }

    // 再生制御
    if (input_->IsKeyTriggered(DIK_SPACE))
    {
        if (isPlaying_)
        {
            PauseCurrentEffect();
        }
        else
        {
            PlayCurrentEffect();
        }
    }

    if (input_->IsKeyTriggered(DIK_S) && !input_->IsKeyPressed(DIK_LCONTROL))
    {
        StopCurrentEffect();
    }

    if (input_->IsKeyTriggered(DIK_R))
    {
        ResetCurrentEffect();
    }

    // ファイル操作
    if (input_->IsKeyPressed(DIK_LCONTROL))
    {
        if (input_->IsKeyTriggered(DIK_S))
        {
            SaveCurrentEffect();
        }

        if (input_->IsKeyTriggered(DIK_N))
        {
            auto newEffect = Effect::Create("NewEffect");
            if (newEffect)
            {
                effects_.emplace_back(std::move(newEffect));
                selectedEffectIndex_ = static_cast<int>(effects_.size()) - 1;
            }
        }

        if (input_->IsKeyTriggered(DIK_C))
        {
            CopyEmitterProperties();
        }

        if (input_->IsKeyTriggered(DIK_V))
        {
            PasteEmitterProperties();
        }
    }
}

void EffectEditorScene::DrawGrid()
{
    // グリッド描画の実装
    // LineDrawerを使用してグリッドを描画
    // この部分は実際のLineDrawerのAPIに合わせて実装する必要があります
    const float gridSpacing = gridSize_ / static_cast<float>(gridCount_);

    for (int i = -gridCount_; i <= gridCount_; ++i)
    {
        float pos = i * gridSpacing;
        // X軸ライン
        LineDrawer::GetInstance()->RegisterPoint(Vector3(pos, 0, -gridSize_), Vector3(pos, 0, gridSize_));
        // Z軸ライン
        LineDrawer::GetInstance()->RegisterPoint(Vector3(-gridSize_, 0, pos), Vector3(gridSize_, 0, pos));
    }

}

void EffectEditorScene::DrawAxis()
{
    // 軸描画の実装
    // X軸: 赤、Y軸: 緑、Z軸: 青で軸を描画
}

void EffectEditorScene::DrawBounds()
{
    // バウンディングボックス描画の実装
    Effect* currentEffect = GetCurrentEffect();
    if (currentEffect)
    {
        // エフェクトのバウンディングボックスを描画
    }
}

bool EffectEditorScene::SaveCurrentEffect()
{
    Effect* currentEffect = GetCurrentEffect();
    if (!currentEffect)
    {
        ShowErrorDialog("No effect selected");
        return false;
    }

    bool success = currentEffect->Save();
    if (success)
    {
        ShowInfoDialog("Effect saved successfully");
    }
    else
    {
        ShowErrorDialog("Failed to save effect: " + currentEffect->GetLastError());
    }

    return success;
}

bool EffectEditorScene::LoadEffectFromFile(const std::string& _filePath)
{
    auto effect = Effect::CreateFromFile(_filePath);
    if (effect)
    {
        effects_.emplace_back(std::move(effect));
        selectedEffectIndex_ = static_cast<int>(effects_.size()) - 1;
        AddToRecentFiles(_filePath);
        return true;
    }
    else
    {
        ShowErrorDialog("Failed to load effect from file: " + _filePath);
        return false;
    }
}

void EffectEditorScene::AddToRecentFiles(const std::string& _filePath)
{
    auto it = std::find(recentFiles_.begin(), recentFiles_.end(), _filePath);
    if (it != recentFiles_.end())
    {
        recentFiles_.erase(it);
    }

    recentFiles_.insert(recentFiles_.begin(), _filePath);

    if (recentFiles_.size() > kMaxRecentFiles)
    {
        recentFiles_.resize(kMaxRecentFiles);
    }
}

void EffectEditorScene::InitializeTemplates()
{
    // 基本テンプレートの初期化
    effectTemplates_.clear();

    // 爆発エフェクト
    EffectTemplate explosion;
    explosion.name = "Explosion";
    explosion.description = "Basic explosion effect with fire and smoke";
    explosion.emitterTemplates = { "Fire", "Smoke", "Sparks" };
    effectTemplates_.push_back(explosion);

    // 魔法エフェクト
    EffectTemplate magic;
    magic.name = "Magic Spell";
    magic.description = "Magical spell effect with particles and glow";
    magic.emitterTemplates = { "MagicParticles", "Glow" };
    effectTemplates_.push_back(magic);

    // 雨エフェクト
    EffectTemplate rain;
    rain.name = "Rain";
    rain.description = "Rain weather effect";
    rain.emitterTemplates = { "Raindrops", "Splash" };
    effectTemplates_.push_back(rain);

    // 炎エフェクト
    EffectTemplate fire;
    fire.name = "Fire";
    fire.description = "Continuous fire effect";
    fire.emitterTemplates = { "Flames", "Embers" };
    effectTemplates_.push_back(fire);
}

Effect* EffectEditorScene::GetCurrentEffect()
{
    if (selectedEffectIndex_ >= 0 && selectedEffectIndex_ < effects_.size())
    {
        return effects_[selectedEffectIndex_].get();
    }
    return nullptr;
}

ParticleEmitter* EffectEditorScene::GetCurrentEmitter()
{
    Effect* effect = GetCurrentEffect();
    if (!effect) return nullptr;

    auto emitters = effect->GetEmitters();
    if (selectedEmitterIndex_ >= 0 && selectedEmitterIndex_ < emitters.size())
    {
        auto it = emitters.begin();
        std::advance(it, selectedEmitterIndex_);
        return *it;
    }
    return nullptr;
}

void EffectEditorScene::SelectEffect(int _index)
{
    selectedEffectIndex_ = _index;
    selectedEmitterIndex_ = -1; // エミッター選択をリセット
}

void EffectEditorScene::SelectEmitter(int _index)
{
    selectedEmitterIndex_ = _index;
}

void EffectEditorScene::PlayCurrentEffect()
{
    Effect* effect = GetCurrentEffect();
    if (effect)
    {
        effect->Play();
        isPlaying_ = true;
        isPaused_ = false;
        previewTime_ = 0.0f;
    }
}

void EffectEditorScene::StopCurrentEffect()
{
    Effect* effect = GetCurrentEffect();
    if (effect)
    {
        effect->Stop();
    }
    isPlaying_ = false;
    isPaused_ = false;
    previewTime_ = 0.0f;
}

void EffectEditorScene::PauseCurrentEffect()
{
    Effect* effect = GetCurrentEffect();
    if (effect)
    {
        if (isPlaying_ && !isPaused_)
        {
            effect->Pause();
            isPaused_ = true;
        }
        else if (isPaused_)
        {
            effect->Resume();
            isPaused_ = false;
        }
    }
}

void EffectEditorScene::ResetCurrentEffect()
{
    Effect* effect = GetCurrentEffect();
    if (effect)
    {
        effect->Reset();
    }
    previewTime_ = 0.0f;
}

void EffectEditorScene::SetPlaybackSpeed(float _speed)
{
    playbackSpeed_ = _speed;
    Effect* effect = GetCurrentEffect();
    if (effect)
    {
        effect->SetPlaybackSpeed(_speed);
    }
}

void EffectEditorScene::DuplicateCurrentEffect()
{
    Effect* currentEffect = GetCurrentEffect();
    if (currentEffect)
    {
        auto newEffect = Effect::Create(currentEffect->GetName() + "_Copy");
        if (newEffect)
        {
            // 設定をコピー（簡略化）
            newEffect->SetLoop(currentEffect->IsLoop());
            newEffect->SetPlaybackSpeed(currentEffect->GetPlaybackSpeed());
            newEffect->SetPosition(currentEffect->GetPosition());

            // エミッターをコピー
            auto emitters = currentEffect->GetEmitters();
            for (auto* emitter : emitters)
            {
                newEffect->AddEmitter(emitter->GetName() + "_Copy");
            }

            effects_.emplace_back(std::move(newEffect));
            selectedEffectIndex_ = static_cast<int>(effects_.size()) - 1;
        }
    }
}

void EffectEditorScene::DeleteCurrentEffect()
{
    if (selectedEffectIndex_ >= 0 && selectedEffectIndex_ < effects_.size())
    {
        effects_.erase(effects_.begin() + selectedEffectIndex_);
        selectedEffectIndex_ = (std::min)(selectedEffectIndex_, static_cast<int>(effects_.size()) - 1);
        selectedEmitterIndex_ = -1;
    }
}

void EffectEditorScene::CopyEmitterProperties()
{
    ParticleEmitter* emitter = GetCurrentEmitter();
    if (emitter)
    {
        // クリップボードにエミッターの設定をコピー
        // 実装簡略化のため、フラグのみ設定
        hasClipboardData_ = true;
    }
}

void EffectEditorScene::PasteEmitterProperties()
{
    if (!hasClipboardData_) return;

    ParticleEmitter* emitter = GetCurrentEmitter();
    if (emitter)
    {
        // クリップボードからエミッターの設定を適用
        // 実際の実装では、設定データの保存・復元が必要
    }
}

void EffectEditorScene::ShowErrorDialog(const std::string& _message)
{
    // TODO: ImGuiでエラーダイアログを表示する実装
    // 現在は何もしない（リンクエラー回避用）
    (void)_message;
}

void EffectEditorScene::ShowInfoDialog(const std::string& _message)
{
    // TODO: ImGuiで情報ダイアログを表示する実装
    // 現在は何もしない（リンクエラー回避用）
    (void)_message;
}

void EffectEditorScene::ShowTooltip(const std::string& _text)
{
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", _text.c_str());
    }
}

#endif // _DEBUG