#pragma once

#include "UIComponent.h"

#include <Features/TextRenderer/TextGenerator.h>

#include <string>


namespace Engine {

class UITextRenderComponent : public UIComponent
{
public:
    UITextRenderComponent(UIElement* owner, const std::string& text = "Text", const FontConfig& fontConfig = {});
    ~UITextRenderComponent() override;

    void Initialize() override;
    void Update() override;
    void Draw() override;

    void DrawImGui() override;

    // ------------------
    // 文字列設定
    void SetText(const std::string& text) { text_ = text; }
    const std::string& GetText() const { return text_; }

    // ------------------
    // 文字パラメータ設定
    void SetTextParam(const TextParam& param) { textParam_ = param; }
    const TextParam& GetTextParam() const { return textParam_; }
    TextParam& GetTextParam() { return textParam_; }

    // ------------------
    // フォント設定
    void SetFontConfig(const FontConfig& config) { fontConfig_ = config; }
    const FontConfig& GetFontConfig() const { return fontConfig_; }
    FontConfig& GetFontConfig() { return fontConfig_; }

    // ------------------
    // クリッピング矩形設定
    void SetClipRect(const Rect& rect);
    const Rect& GetClipRect() const { return clipRect_; }


private:
    std::string text_;
    FontConfig fontConfig_;

    bool hasRect_;
    Rect clipRect_;

    TextParam textParam_;
};

} // namespace Engine
