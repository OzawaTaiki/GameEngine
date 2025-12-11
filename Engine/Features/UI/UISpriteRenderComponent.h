#pragma once

#include "UIComponent.h"
#include <Math/Vector/Vector4.h>
#include <Features/Sprite/Sprite.h>

#include <string>
#include <cstdint>

class UISpriteRenderComponent : public UIComponent
{
public:
    UISpriteRenderComponent(UIElement* owner, const std::string& texturePath = "while.png");
    ~UISpriteRenderComponent() override;

    void Initialize() override;
    void Update() override;
    void Draw() override;

    void DrawImGui() override;

    // ------------------
    // テクスチャ設定
    uint32_t GetTextureHandle() const { return textureHandle_; }
    void LoadAndSetTexture(const std::string& filePath);
    const std::string& GetTexturePath() const { return texturePath_; }

    Sprite* GetSprite() const { return sprite_.get(); }

    // ------------------
    // 色設定
    void SetColor(const Vector4& color) { color_ = color; }
    const Vector4& GetColor() const { return color_; }

private:

    std::shared_ptr<Sprite> sprite_;
    std::string texturePath_;
    Vector4 color_ = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

    uint32_t textureHandle_ = 0;
};
