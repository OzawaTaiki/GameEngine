#pragma once

#include "UIElement.h"
#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector4.h>
#include <Features/TextRenderer/TextGenerator.h>
#include <string>

class UITextRenderComponent;

class UITextElement : public UIElement
{
public:
    UITextElement(
        const std::string& name,
        const Vector2& position,
        const std::string& text = "Text",
        const FontConfig& fontConfig = FontConfig()
    );
    ~UITextElement() override = default;

    void Initialize() override;

    // ------------------
    // テキスト設定
    void SetText(const std::string& text);
    const std::string& GetText() const;

    // ------------------
    // フォント設定
    void SetFontConfig(const FontConfig& config);
    const FontConfig& GetFontConfig() const;

    // ------------------
    // テキストパラメータ設定
    void SetTextParam(const TextParam& param);
    const TextParam& GetTextParam() const;

    // ------------------
    // 色設定（簡易アクセス）
    void SetColor(const Vector4& color);
    Vector4 GetColor() const;

private:
    UITextRenderComponent* textComponent_ = nullptr;
    std::string text_;
    FontConfig fontConfig_;
};
