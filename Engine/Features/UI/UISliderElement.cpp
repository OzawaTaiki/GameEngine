#include "UISliderElement.h"
#include <Features/UI/UISpriteRenderComponent.h>
#include <Features/UI/UISliderComponent.h>
#include <Features/UI/Collider/UIColliderComponent.h>

UISliderElement::UISliderElement(const std::string& name,
                                 const Vector2& pos,
                                 const Vector2& size,
                                 bool child)
    : UIElement(name, child)
{
    SetPosition(pos);
    SetSize(size);
    SetAnchor(Vector2(0.0f, 0.0f));  // 左上基準
}

void UISliderElement::Initialize()
{
    UIElement::Initialize();

    RegisterVariable("handleHoverColor", &handleHoverColor_);

    // トラック作成
    auto track = std::make_unique<UIElement>(GetName() + "_Track", true);
    track->SetPosition(Vector2(0.0f, 0.0f));
    track->SetSize(GetSize());
    track->SetPivot(Vector2(0.0f, 0.5f));  // 左中央基準
    track->SetAnchor(Vector2(0.0f, 0.5f));
    track->Initialize();

    track_ = AddChild(std::move(track));

    // トラックのスプライト
    trackSprite_ = track_->AddComponent<UISpriteRenderComponent>(track_, "white.png");
    trackSprite_->GetSprite()->SetAnchor(Vector2(0.0f, 0.5f));

    // トラックのコライダー
    track_->AddComponent<UIColliderComponent>(ColliderType::Rectangle);

    // ハンドル作成
    auto handle = std::make_unique<UIElement>(GetName() + "_Handle",true);
    handle->SetSize(Vector2(handleWidth_, handleHeight_));
    handle->SetPivot(Vector2(0.5f, 0.5f));  // 中央基準
    handle->SetAnchor(Vector2(0.0f, 0.5f));
    handle->Initialize();

    handle_ = AddChild(std::move(handle));

    // ハンドルのスプライト
    handleSprite_ = handle_->AddComponent<UISpriteRenderComponent>(handle_, "white.png");
    handleSprite_->GetSprite()->SetAnchor(Vector2(0.5f, 0.5f));

    // ハンドルのコライダー
    handle_->AddComponent<UIColliderComponent>(ColliderType::Rectangle);

    // スライダーコンポーネント追加
    slider_ = AddComponent<UISliderComponent>();
    slider_->SetTrack(track_);
    slider_->SetHandle(handle_);

}

void UISliderElement::Update()
{
    UIElement::Update();

    // 状態に応じた色更新
    OnStateChanged();
}

void UISliderElement::Draw()
{
    UIElement::Draw();
}

void UISliderElement::SetValue(float value)
{
    if (slider_)
        slider_->SetValue(value);
}

float UISliderElement::GetValue() const
{
    return slider_ ? slider_->GetValue() : 0.0f;
}

void UISliderElement::SetRange(float min, float max)
{
    if (slider_)
        slider_->SetRange(min, max);
}

void UISliderElement::SetOnValueChanged(std::function<void(float)> callback)
{
    if (slider_)
        slider_->SetOnValueChanged(callback);
}

void UISliderElement::SetTrackColor(const Vector4& color)
{
    trackColor_ = color;
    if (trackSprite_)
        trackSprite_->SetColor(trackColor_);
}

void UISliderElement::SetHandleColor(const Vector4& color)
{
    handleColor_ = color;
    if (handleSprite_)
        handleSprite_->SetColor(handleColor_);
}

void UISliderElement::SetHandleHoverColor(const Vector4& color)
{
    handleHoverColor_ = color;
}

void UISliderElement::SetSliderEnabled(bool enabled)
{
    SetEnabled(enabled);
    if (slider_)
        slider_->SetEnabled(enabled);
}

void UISliderElement::DrawImGuiTree()
{
}

void UISliderElement::DrawImGuiInspector()
{
#ifdef _DEBUG

    ImGui::PushID(this);
    UIElement::DrawImGuiInspector();
    ImGui::BeginDisabled(1);
    if (ImGui::TreeNode("UISliderElement"))
    {
        ImGui::TreePop();
    }
    ImGui::EndDisabled();
    ImGui::PopID();

#endif
}

void UISliderElement::OnStateChanged()
{
    // TODO: ハンドルのHover状態を検出して色を変更
    // 現状は基本色を使用
}
