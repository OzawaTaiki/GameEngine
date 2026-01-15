#include "UIButtonElement.h"
#include <Features/UI/Component/UISpriteRenderComponent.h>
#include <Features/UI/Component/UITextRenderComponent.h>
#include <Features/UI/Component/UIButtonComponent.h>
#include <Features/UI/Component/UINavigationComponent.h>
#include <Features/UI/UINavigationManager.h>
#include <Features/UI/Component/UIColliderComponent.h>


namespace Engine {

UIButtonElement::UIButtonElement(const std::string& name,
                                 const Vector2& pos,
                                 const Vector2& size,
                                 const std::string& text,
                                 bool child)
    : UIElement(name, child)
    , text_(text)
{
    SetPosition(pos);
    SetSize(size);
    SetAnchor(Vector2(0.0f, 0.0f));  // 左上基準
}

void UIButtonElement::Initialize()
{
    UIElement::Initialize();

    RegisterVariable("normalColor", &normalColor_);
    RegisterVariable("hoverColor", &hoverColor_);
    RegisterVariable("pressedColor", &pressedColor_);
    RegisterVariable("disabledColor", &disabledColor_);
    RegisterVariable("focusColor", &focusColor_);

    RegisterVariable("textColor", &textColor_);
    RegisterVariable("textAlignment", reinterpret_cast<int*>(&textAlignment_));

    // 背景スプライト作成
    background_ = AddComponent<UISpriteRenderComponent>(this, "white.png");
    background_->SetColor(normalColor_);

    // テキスト作成
    FontConfig fontConfig;
    fontConfig.fontFilePath = "Resources/Fonts/NotoSansJP-Regular.ttf";
    fontConfig.fontSize = 24;

    textComponent_ = AddComponent<UITextRenderComponent>(this, text_, fontConfig);
    textComponent_->Initialize();
    // テキストパラメータ設定
    TextParam textParam;
    //textParam.position = CalculateTextPosition();
    textParam.SetColor(textColor_)
             .SetPivot(CalculateTextPivot())
             .SetScale(Vector2(1.0f, 1.0f));
    textComponent_->SetTextParam(textParam);

    // ボタンコンポーネント追加（自動的にColliderが追加される）
    button_ = AddComponent<UIButtonComponent>();

    // 状態変更時のコールバック設定
    button_->SetOnHover([this]() {
        OnStateChanged();
    });

    // ナビゲーションコンポーネント追加
    navigation_ = AddComponent<UINavigationComponent>();
    navigation_->SetFocusable(true);

    // NavigationManagerに登録
    UINavigationManager::GetInstance()->Register(this);


}

void UIButtonElement::Update()
{
    UIElement::Update();

    // 状態に応じた色更新
    OnStateChanged();
}

void UIButtonElement::Draw()
{
    UIElement::Draw();
}

void UIButtonElement::SetOnClick(std::function<void()> callback)
{
    if (button_)
        button_->SetOnClick(callback);
}

void UIButtonElement::SetOnHover(std::function<void()> callback)
{
    if (button_)
        button_->SetOnHover(callback);
}

void UIButtonElement::SetOnClickDown(std::function<void()> callback)
{
    if (button_)
        button_->SetOnClickDown(callback);
}

void UIButtonElement::SetOnClickUp(std::function<void()> callback)
{
    if (button_)
        button_->SetOnClickUp(callback);
}

void UIButtonElement::SetOnHoverEnter(std::function<void()> callback)
{
    if (button_)
        button_->SetOnHoverEnter(callback);
}

void UIButtonElement::SetOnHoverExit(std::function<void()> callback)
{
    if (button_)
        button_->SetOnHoverExit(callback);
}

void UIButtonElement::SetOnHovering(std::function<void()> callback)
{
    if (button_)
        button_->SetOnHovering(callback);
}

void UIButtonElement::SetOnFocusEnter(std::function<void()> callback)
{
    if (navigation_)
        navigation_->SetOnFocusEnter(callback);
}

void UIButtonElement::SetOnFocusExit(std::function<void()> callback)
{
    if (navigation_)
        navigation_->SetOnFocusExit(callback);
}

void UIButtonElement::SetText(const std::string& text)
{
    text_ = text;
    if (textComponent_)
    {
        textComponent_->SetText(text_);
    }
}

void UIButtonElement::SetTextColor(const Vector4& color)
{
    textColor_ = color;
    if (textComponent_)
    {
        TextParam param = textComponent_->GetTextParam();
        param.SetColor(textColor_);
        textComponent_->SetTextParam(param);
    }
}

void UIButtonElement::SetButtonEnabled(bool enabled)
{
    SetEnabled(enabled);
    if (button_)
        button_->SetEnabled(enabled);
}

void UIButtonElement::OnStateChanged()
{
    if (!button_ || !background_)
        return;

    ButtonState state = button_->GetState();
    bool isFocused = navigation_ && navigation_->IsFocused();

    Vector4 targetColor;
    switch (state)
    {
    case ButtonState::Normal:
        // Normal状態でフォーカスされている場合はフォーカス色
        targetColor = isFocused ? focusColor_ : normalColor_;
        break;
    case ButtonState::Hovered:
        targetColor = hoverColor_;
        break;
    case ButtonState::Pressed:
        targetColor = pressedColor_;
        break;
    case ButtonState::Disabled:
        targetColor = disabledColor_;
        break;
    default:
        targetColor = normalColor_;
        break;
    }

    background_->SetColor(targetColor);
}

void UIButtonElement::SetTextAlignment(TextAlignment alignment)
{
    textAlignment_ = alignment;

    if (textComponent_)
    {
        TextParam param = textComponent_->GetTextParam();
        param.position = CalculateTextPosition();
        param.SetPivot(CalculateTextPivot());
        textComponent_->SetTextParam(param);
    }
}

Vector2 UIButtonElement::CalculateTextPosition() const
{
    Vector2 size = GetSize();

    switch (textAlignment_)
    {
    case TextAlignment::TopLeft:
        return Vector2(0.0f, 0.0f);
    case TextAlignment::TopCenter:
        return Vector2(size.x * 0.5f, 0.0f);
    case TextAlignment::TopRight:
        return Vector2(size.x, 0.0f);
    case TextAlignment::CenterLeft:
        return Vector2(0.0f, size.y * 0.5f);
    case TextAlignment::Center:
        return Vector2(size.x * 0.5f, size.y * 0.5f);
    case TextAlignment::CenterRight:
        return Vector2(size.x, size.y * 0.5f);
    case TextAlignment::BottomLeft:
        return Vector2(0.0f, size.y);
    case TextAlignment::BottomCenter:
        return Vector2(size.x * 0.5f, size.y);
    case TextAlignment::BottomRight:
        return Vector2(size.x, size.y);
    default:
        return Vector2(size.x * 0.5f, size.y * 0.5f);
    }
}

Vector2 UIButtonElement::CalculateTextPivot() const
{
    switch (textAlignment_)
    {
    case TextAlignment::TopLeft:
        return Vector2(0.0f, 0.0f);
    case TextAlignment::TopCenter:
        return Vector2(0.5f, 0.0f);
    case TextAlignment::TopRight:
        return Vector2(1.0f, 0.0f);
    case TextAlignment::CenterLeft:
        return Vector2(0.0f, 0.5f);
    case TextAlignment::Center:
        return Vector2(0.5f, 0.5f);
    case TextAlignment::CenterRight:
        return Vector2(1.0f, 0.5f);
    case TextAlignment::BottomLeft:
        return Vector2(0.0f, 1.0f);
    case TextAlignment::BottomCenter:
        return Vector2(0.5f, 1.0f);
    case TextAlignment::BottomRight:
        return Vector2(1.0f, 1.0f);
    default:
        return Vector2(0.5f, 0.5f);
    }
}

void UIButtonElement::SetNavigation(NavigationDirection dir, UIElement* target)
{
    if (navigation_)
    {
        navigation_->SetNavigation(dir, target);
    }
}

void UIButtonElement::SetFocusable(bool focusable)
{
    if (navigation_)
    {
        navigation_->SetFocusable(focusable);
    }
}

bool UIButtonElement::IsFocused() const
{
    return navigation_ ? navigation_->IsFocused() : false;
}

void UIButtonElement::DrawImGuiInspector()
{
#ifdef _DEBUG

    ImGui::PushID(this);
    UIElement::DrawImGuiInspector();
    if (ImGui::TreeNode("UIButtonElement"))
    {
        // 色設定
        ImGui::ColorEdit4("Normal Color", &normalColor_.x);
        ImGui::ColorEdit4("Hover Color", &hoverColor_.x);
        ImGui::ColorEdit4("Pressed Color", &pressedColor_.x);
        ImGui::ColorEdit4("Disabled Color", &disabledColor_.x);
        ImGui::ColorEdit4("Focus Color", &focusColor_.x);
        // テキストアラインメント
        const char* alignments[] = {
            "TopLeft", "TopCenter", "TopRight",
            "CenterLeft", "Center", "CenterRight",
            "BottomLeft", "BottomCenter", "BottomRight"
        };
        int currentAlignment = static_cast<int>(textAlignment_);
        if (ImGui::Combo("Text Alignment", &currentAlignment, alignments, IM_ARRAYSIZE(alignments)))
        {
            SetTextAlignment(static_cast<TextAlignment>(currentAlignment));
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
#endif
}

} // namespace Engine
