#include "UIAnimationComponent.h"

#include <Features/UI/Element/UIElement.h>
#include <System/Time/GameTime.h>
#include <Debug/ImGuiHelper.h>
#include <Debug/ImguITools.h>


Engine::UIAnimationComponent::UIAnimationComponent(UIElement* owner, const std::string& animName)
    : UIComponent(),
    useTimeChannel_("default"),
    animationName_(animName),
    animationSequence_(nullptr)
{
    SetOwner(owner);
}

void Engine::UIAnimationComponent::Initialize()
{
    owner_->RegisterVariable("useTimeChannel", &useTimeChannel_);

    animationSequence_ = std::make_unique<AnimationSequence>(owner_->GetName() + animationName_);
    // アニメーションシーケンスの初期化
    animationSequence_->Initialize("Resources/Data/UI/");

    if (animationSequence_->GetSequenceEvents().empty())
    {// イベントがないときはデフォルトを用意
        animationSequence_->CreateSequenceEvent<Vector2>("PositionOffset", Vector2(0, 0));
        animationSequence_->CreateSequenceEvent<Vector2>("Size", Vector2(0, 0));
        animationSequence_->CreateSequenceEvent<Vector2>("Scale", Vector2(1, 1));
        animationSequence_->CreateSequenceEvent<float>("Rotation", 0.0f);
        //
        //animationSequence_->CreateSequenceEvent<Vector4>("Color", Vector4(1, 1, 1, 1));
    }

    baseTransform_.position = owner_->GetPosition();
    baseTransform_.size     = owner_->GetSize();
    baseTransform_.scale    = owner_->GetScale();
    baseTransform_.rotation = owner_->GetRotation();
}

void Engine::UIAnimationComponent::Update()
{
    if (!animationSequence_ || !isPlaying_)
        return;

    float deltaTime = GameTime::GetChannel(useTimeChannel_).GetDeltaTime<float>();

    // アニメーションシーケンスの更新
    animationSequence_->Update(deltaTime);

    if (animationSequence_->HasEvent("PositionOffset"))
        owner_->SetPosition(baseTransform_.position + animationSequence_->GetValue<Vector2>("PositionOffset"));
    if (animationSequence_->HasEvent("Size"))
        owner_->SetSize(baseTransform_.size + animationSequence_->GetValue<Vector2>("Size"));
    if (animationSequence_->HasEvent("Scale"))
        owner_->SetScale(baseTransform_.scale * animationSequence_->GetValue<Vector2>("Scale"));
    if (animationSequence_->HasEvent("Rotation"))
        owner_->SetRotation(baseTransform_.rotation + animationSequence_->GetValue<float>("Rotation"));

    if (animationSequence_->IsEnd())
    {
        isPlaying_ = false;
        if (onAnimationEnd_)
        {
            onAnimationEnd_();
        }
    }

}

void Engine::UIAnimationComponent::DrawImGui()
{
#ifdef _DEBUG
    ImGui::PushID(this);
    if (ImGui::TreeNode("UIAnimationComponent"))
    {
        if (animationSequence_)
        {
            ImGuiHelper::InputText("Animation Name", animationName_);
            ImGui::Checkbox("Show Timeline", &showSeq);
            if (ImGui::Checkbox("Play", &isPlaying_))
                Play();

            if (showSeq)
            {
                ImGuiTool::TimeLine((owner_->GetName() + animationName_).c_str(), animationSequence_.get());
            }
        }
        ImGui::TreePop();
    }

    ImGui::PopID();
#endif // _DEBUG
}

void Engine::UIAnimationComponent::Play()
{
    isPlaying_ = true;
    if (animationSequence_)
    {
        animationSequence_->SetCurrentTime(0.0f);
    }
}

void Engine::UIAnimationComponent::Pause()
{
    isPlaying_ = false;
}

void Engine::UIAnimationComponent::Stop()
{
    isPlaying_ = false;
    if (animationSequence_)
    {
        animationSequence_->SetCurrentTime(0.0f);
    }
}

bool Engine::UIAnimationComponent::IsEnded() const
{
    if (animationSequence_)
    {
        return animationSequence_->IsEnd();
    }

    return true; // シーケンスがない場合は常に終了とみなす
}
