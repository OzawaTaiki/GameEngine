#include "UIText.h"
#include <Utility/ConvertString/ConvertString.h>

void UIText::Initialize(const std::string& _label, const std::wstring& _text, const FontConfig& _config, bool _registerDebugWindow)
{
    UIBase::Initialize(_label, _registerDebugWindow);

    text_ = _text;
    fontConfig_ = _config;

    textGenerator_.Initialize(fontConfig_);

    jsonBinder_->RegisterVariable(label_ + "_text", &text_);
    jsonBinder_->RegisterVariable(label_ + "_textParam", &textParam_);

}

void UIText::Draw()
{
    if (!isVisible_)
        return;

    TextParam param = textParam_;
    param.position += GetWorldPos();
    textGenerator_.Draw(text_, param);

    for (auto& child : children_)
    {
        if (child && child->IsVisible())
        {
            child->Draw();
        }
    }
}

void UIText::SetFontConfig(const FontConfig& _config)
{
    fontConfig_ = _config;
    textGenerator_.Initialize(fontConfig_);
}

void UIText::ImGuiContent()
{
#ifdef _DEBUG
    if (ImGui::TreeNode("UIText"))
    {
        std::string str = ConvertString(text_);
        char buffer[256];
        strcpy_s(buffer, str.c_str());
        if (ImGui::InputText("text", buffer, sizeof(buffer)))
        {
            text_ = ConvertString(buffer);
        }

        ImGui::SeparatorText("TextParam");
        textParam_.ImGui();

        ImGui::TreePop();
    }

#endif // _DEBUG
}
