#include "UIImageElement.h"
#include "UISpriteRenderComponent.h"

UIImageElement::UIImageElement(
    const std::string& name,
    const Vector2& position,
    const Vector2& size,
    const std::string& texturePath):
    UIElement(name),
    texturePath_(texturePath)
{
    SetPosition(position);
    SetSize(size);
}

void UIImageElement::Initialize()
{
    UIElement::Initialize();

    RegisterVariable("texturePath", &texturePath_);
    RegisterVariable("color", &color_);


    // スプライトレンダーコンポーネントを追加
    sprite_ = AddComponent<UISpriteRenderComponent>(this, texturePath_);
    sprite_->SetColor(color_);
}

void UIImageElement::SetTexture(const std::string& texturePath)
{
    texturePath_ = texturePath;
    if (sprite_)
    {
        sprite_->LoadAndSetTexture(texturePath_);
    }
}

const std::string& UIImageElement::GetTexturePath() const
{
    return texturePath_;
}

void UIImageElement::SetColor(const Vector4& color)
{
    color_ = color;
    if (sprite_)
    {
        sprite_->SetColor(color_);
    }
}

const Vector4& UIImageElement::GetColor() const
{
    return color_;
}
