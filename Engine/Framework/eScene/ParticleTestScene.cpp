#include "ParticleTestScene.h"

#include <Rendering/Model/ModelManager.h>

#include "SceneManager.h"
#include <Framework/Particle/ParticleManager.h>
#include <UI/ImGuiManager/ImGuiManager.h>
#include <Systems/Input/Input.h>
#include <Rendering/Sprite/Sprite.h>

ParticleTestScene::~ParticleTestScene()
{
    ParticleManager::GetInstance()->ClearGroup();
}

void ParticleTestScene::Initialize()
{
    addEmitterName_ = "";

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

}

void ParticleTestScene::Update()
{
#ifdef _DEBUG
    ImGui();
#endif // _DEBUG

    for (auto& e : effects_)
    {
        e.Update();
    }

    // シーン関連更新
#ifdef _DEBUG
    if(Input::GetInstance()->IsKeyTriggered(DIK_RETURN) &&
       Input::GetInstance()->IsKeyPressed(DIK_RSHIFT))
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

void ParticleTestScene::Draw()
{
    ModelManager::GetInstance()->PreDrawForObjectModel();
    plane_->Draw(&SceneCamera_, { 1,1,1,1 });

    ModelManager::GetInstance()->PreDrawForAnimationModel();

    Sprite::PreDraw();

    for (auto& emitter : emitters_)
        emitter->Draw();

    ParticleManager::GetInstance()->Draw(&SceneCamera_);
    lineDrawer_->Draw();
}

#ifdef _DEBUG
void ParticleTestScene::ImGui()
{
    ImGui::Begin("ParticleTestScene");

#pragma region エフェクト


    static char effectBuf[256];
    if(ImGui::InputText("EffectName", effectBuf, 256))
        addEffectName_ = effectBuf;

    ImGui::BeginDisabled(strcmp(effectBuf, "") == 0);
    // エフェクトの追加
    if(ImGui::Button(" Create New Effect"))
    {
        addEffectName_ = effectBuf;
        effects_.emplace_back();
        effects_.back().Initialize(addEffectName_);
        strcpy_s(effectBuf, sizeof(effectBuf), "");
        addEffectName_ = "";
    }
    ImGui::EndDisabled();

    static std::vector<std::string> effectNames;
    effectNames.clear();
    std::vector<const char*> items;
    for (auto& effect : effects_)
    {
        effectNames.push_back(effect.GetName());
        items.push_back(effectNames.back().c_str());
    }
    static int selectedEffect = 0;
    if (ImGui::ListBox("Effects", &selectedEffect, items.data(), static_cast<int>(effects_.size())))
    {
        auto it = effects_.begin();
        std::advance(it, selectedEffect);
        selectedEffect_ = it;
        emitters_ = selectedEffect_->GetEmitters();
    }
    static bool isActive = false;
    if (ImGui::Button("Active"))
    {
        isActive = true;
        if (selectedEffect_ != effects_.end())
        {
            selectedEffect_->SetActive(isActive);
        }
    }

    if (selectedEffect_ != effects_.end())
    {
        if (ImGui::Button("Save"))
        {
            selectedEffect_->Save();
        }
    }

#pragma endregion

#pragma region エミッター

    if (selectedEffect_ != effects_.end())
    {
        selectedEffect_->DebugShowForEmitterCreate();
        //// stringをcharに変換
        //static char emitterBuf[256];
        //if (ImGui::InputText("EmitterName", emitterBuf, 256))
        //    addEmitterName_ = emitterBuf;
        //
        //
        //ImGui::BeginDisabled(strcmp(emitterBuf, "") == 0);
        //// エミッターの追加
        //if (ImGui::Button("Create New Emitter"))
        //{
        //    addEmitterName_ = emitterBuf;
        //    emitters_.push_back(selectedEffect_->AddEmitter(addEmitterName_));
        //    //emitters_.back()->Setting(addEmitterName_);
        //    strcpy_s(emitterBuf, sizeof(emitterBuf), "");
        //    addEmitterName_ = "";
        //}
        //ImGui::EndDisabled();
        //
        ////エミッターの名前一覧
        //static bool isSelect[kMaxEmitterNum];
        //if (ImGui::TreeNode("Emitters"))
        //{
        //    size_t index = 0;
        //    for (auto& emitter : emitters_)
        //    {
        //        ImGui::Selectable(emitter->GetName().c_str(), &isSelect[index]);
        //        ++index;
        //    }
        //
        //    ImGui::TreePop();
        //}
        //
        //int cnt = 0;
        //for (auto& emitter : emitters_)
        //{
        //    if (isSelect[cnt++])
        //    {
        //        if (emitter->ShowDebugWinsow())
        //        {
        //            if (ImGui::Button("Exclusion"))
        //            {
        //                selectedEffect_->ExclusionEmitter(emitter->GetName());
        //                emitters_.remove(emitter);
        //                break;
        //            }
        //        }
        //    }
        //}
    }
#pragma endregion

    ImGui::End();
}
#endif // _DEBUG
