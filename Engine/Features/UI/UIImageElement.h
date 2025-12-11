#pragma once

#include "UIElement.h"
#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector4.h>
#include <string>

class UISpriteRenderComponent;

class UIImageElement : public UIElement
{
public:
    UIImageElement(
        const std::string& name,
        const Vector2& position,
        const Vector2& size,
        const std::string& texturePath = "white.png"
    );
    ~UIImageElement() override = default;

    void Initialize() override;

    // ------------------
    // テクスチャ設定
    void SetTexture(const std::string& texturePath);
    const std::string& GetTexturePath() const;

    // ------------------
    // 色設定
    void SetColor(const Vector4& color);
    const Vector4& GetColor() const;

private:
    UISpriteRenderComponent* sprite_ = nullptr;
    std::string texturePath_;
    Vector4 color_ = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
};
