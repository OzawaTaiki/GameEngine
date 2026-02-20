#pragma once

#include <Features/UI/UIBase.h>
#include <Features/TextRenderer/TextGenerator.h>


namespace Engine {

class UIText : public UIBase
{
public:
    UIText() = default;
    ~UIText() override = default;

    void Initialize(const std::string& _label, const std::wstring& _text, const FontConfig& _config = {}, bool _registerDebugWindow = true);
    void Draw() override;

    void SetText(const std::wstring& _text) { text_ = _text; }
    const std::wstring& GetText() const { return text_; }

    void SetTextParam(const TextParam& _param) { textParam_ = _param; }
    TextParam& GetTextParam() { return textParam_; }

    void SetFontConfig(const FontConfig& _config);

    void ImGuiContent() override;

protected:

    std::wstring text_ = L"";
    TextGenerator textGenerator_;
    TextParam textParam_;
    FontConfig fontConfig_;
};

} // namespace Engine
