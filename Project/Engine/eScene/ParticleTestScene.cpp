#include "ParticleTestScene.h"

#include "SceneManager.h"
#include "Utility/ConfigManager.h"
#include "Particle/ParticleManager.h"
#include  "ImGuiManager/ImGuiManager.h"

std::unique_ptr<BaseScene>ParticleTestScene::Create()
{
    return std::make_unique<ParticleTestScene>();
}

ParticleTestScene::~ParticleTestScene()
{
}

void ParticleTestScene::Initialize()
{
    addEmitterName_ = "";
}

void ParticleTestScene::Update()
{
    ImGui();
    for (auto& e : effects_)
    {
        e.Update();
    }
}

void ParticleTestScene::Draw()
{
    for (auto& emitter : emitters_)
        emitter->Draw();
}

#ifdef _DEBUG
void ParticleTestScene::ImGui()
{

    ImGui::Begin("ParticleTestScene");

    // stringをcharに変換
    static char buf[256];
    if (ImGui::InputText("EmitterName", buf, 256))
        addEmitterName_ = buf;


    ImGui::BeginDisabled(strcmp(buf, "") == 0);
    // エミッターの追加
    if (ImGui::Button("Create New Emitter"))
    {
        addEmitterName_ = buf;
        emitters_.push_back(std::make_unique<ParticleEmitter>());
        emitters_.back()->Setting(addEmitterName_);
        addEmitterName_ = "";

    }
    ImGui::EndDisabled();

    //エミッターの名前一覧

    static bool isSelect[kMaxEmitterNum];
    if (ImGui::TreeNode("Emitters"))
    {
        size_t index = 0;
        for (auto& emitter : emitters_)
        {
            ImGui::Selectable(emitter->GetName().c_str(), &isSelect[index]);
            ++index;
        }

        ImGui::TreePop();
    }

    int cnt = 0;
    for (auto& emitter : emitters_)
    {
        if (isSelect[cnt++])
        {
            emitter->ShowDebugWinsow();
        }
    }


    ImGui::End();
}
#endif // _DEBUG