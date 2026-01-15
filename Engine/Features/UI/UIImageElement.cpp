#include "UIImageElement.h"
#include "UISpriteRenderComponent.h"


namespace Engine {

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

    // スプライトレンダーコンポーネントを追加
    sprite_ = AddComponent<UISpriteRenderComponent>(this, texturePath_);
}

void UIImageElement::SetTexture(const std::string& texturePath)
{
    texturePath_ = texturePath;
    if (sprite_)
    {
        sprite_->LoadAndSetTexture(texturePath_);
    }
}

void UIImageElement::SetTexture(uint32_t textureHandle)
{
    if (sprite_)
    {
        sprite_->SetTexture(textureHandle);
    }
}
uint32_t UIImageElement::GetTextureHandle() const
{
    if (sprite_)
    {
        return sprite_->GetTextureHandle();
    }
    return 0;
}
const std::string& UIImageElement::GetTexturePath() const
{
    return texturePath_;
}

UVTransform& UIImageElement::GetUVTransform()
{
    if (sprite_)
    {
        return sprite_->GetUVTransform();
    }
    // ダミーを返す
    static UVTransform dummy;
    return dummy;
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

} // namespace Engine
