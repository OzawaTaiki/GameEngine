#include "UITextElement.h"
#include "../Component/UITextRenderComponent.h"

UITextElement::UITextElement(
    const std::string& name,
    const Vector2& position,
    const std::string& text,
    const FontConfig& fontConfig):
    UIElement(name),
    text_(text),
    fontConfig_(fontConfig)
{
    SetPosition(position);
}

void UITextElement::Initialize()
{
    UIElement::Initialize();

    // テキストレンダーコンポーネントを追加
    textComponent_ = AddComponent<UITextRenderComponent>(
        this,
        text_,
        fontConfig_
    );
}

void UITextElement::SetText(const std::string& text)
{
    text_ = text;
    if (textComponent_)
    {
        textComponent_->SetText(text_);
    }
}

const std::string& UITextElement::GetText() const
{
    return text_;
}

void UITextElement::SetFontConfig(const FontConfig& config)
{
    fontConfig_ = config;
    if (textComponent_)
    {
        textComponent_->SetFontConfig(config);
    }
}

const FontConfig& UITextElement::GetFontConfig() const
{
    return fontConfig_;
}

void UITextElement::SetTextParam(const TextParam& param)
{
    if (textComponent_)
    {
        textComponent_->SetTextParam(param);
    }
}

const TextParam& UITextElement::GetTextParam() const
{
    if (textComponent_)
    {
        return textComponent_->GetTextParam();
    }
    static TextParam defaultParam;
    return defaultParam;
}

void UITextElement::SetColor(const Vector4& color)
{
    if (textComponent_)
    {
        TextParam param = textComponent_->GetTextParam();
        param.SetColor(color);
        textComponent_->SetTextParam(param);
    }
}

Vector4 UITextElement::GetColor() const
{
    if (textComponent_)
    {
        return textComponent_->GetTextParam().topColor;
    }
    return Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}
