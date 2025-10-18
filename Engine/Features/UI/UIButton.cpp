#include "UIButton.h"
#include <System/Input/Input.h>
#include <Debug/ImGuiManager.h>
#include <Utility/ConvertString/ConvertString.h>

UIButton::UIButton()
{
}

void UIButton::Initialize(const std::string& _label)
{
    UISelectable::Initialize(_label,true);

    jsonBinder_->RegisterVariable(label_ + "backgroundColor", &backgroundColor_);

    // ボタンのデフォルト色設定
    SetDefaultColor({ 0.8f, 0.8f, 0.8f, 1.0f });
    SetHoverColor({ 0.9f, 0.9f, 0.9f, 1.0f });
    SetPressedColor({ 0.6f, 0.6f, 0.6f, 1.0f });
    SetSelectedColor({ 0.7f, 0.7f, 1.0f, 1.0f });
    SetFocusedColor({ 1.0f, 1.0f, 0.8f, 1.0f });


}

void UIButton::Initialize(const std::string& _label, const std::wstring& _text)
{
    Initialize(_label, _text, FontConfig());
}

void UIButton::Initialize(const std::string& _label, const std::wstring& _text, const FontConfig& _config)
{
    Initialize(_label);

    CraeteUIText(_text, _config);
}

void UIButton::UpdateSelf()
{
    UISelectable::UpdateSelf();

    // クリック処理が完了していたらクリック終了
    if (isClickProcessing_ && !isPressed_)
    {
        OnClickEnd();
    }
}

void UIButton::Draw()
{
    if (!isVisible_)
        return;

    UIBase::Draw();
    text_->Draw();
}

void UIButton::SetText(const std::wstring& _text)
{
    text_->SetText(_text);
}

const std::wstring& UIButton::GetText() const
{
    return text_->GetText();
}

void UIButton::SetOnClickStart(std::function<void()> _callback)
{
    onClickCallback_ = _callback;
}

void UIButton::SetOnClickEnd(std::function<void()> _callback)
{
    onClickEnd_ = _callback;
}

void UIButton::SetBackgroundColor(const Vector4& _color)
{
    backgroundColor_ = _color;
    SetDefaultColor(backgroundColor_);
}

void UIButton::ImGuiContent()
{
#ifdef _DEBUG

    UISelectable::ImGuiContent();

    if (ImGui::TreeNode("UIButton"))
    {
        ImGui::ColorEdit4("backgroundColor", &backgroundColor_.x);

        if (text_)
        {
            //    // テキスト編集
            //    char buf[256];
            //std::string str = ConvertString(text_->GetText());
            //strcpy_s(buf, str.c_str());
            //if (ImGui::InputText("text", buf, 256))
            //{
            //    str = buf;
            //    SetText(std::wstring(str.begin(), str.end()));
            //}
        }
        else
        {
            if (ImGui::Button("Enable Text"))
            {
                CraeteUIText(L"Button", FontConfig());
            }
        }

        ImGui::TreePop();
    }


#endif // _DEBUG
}
//
//void UIButton::ImGui()
//{
//#ifdef _DEBUG
//    UISelectable::ImGui();
//
//    ImGui::PushID("Button");
//
//    if (ImGui::TreeNode("Button Settings"))
//    {
//        if (ImGui::ColorEdit4("backgroundColor", &backgroundColor_.x))
//        {
//            SetBackgroundColor(backgroundColor_);
//        }
//
//        if (ImGui::ColorEdit4("textColor", &textColor_.x))
//        {
//            SetTextColor(textColor_);
//        }
//
//        // テキスト編集
//        char buf[256];
//        std::string str = ConvertString(text_);
//        strcpy_s(buf, str.c_str());
//        if (ImGui::InputText("text", buf, 256))
//        {
//            str = buf;
//            SetText(std::wstring(str.begin(), str.end()));
//        }
//
//        ImGui::TreePop();
//    }
//
//    ImGui::PopID();
//#endif
//}

void UIButton::OnClick()
{
    UISelectable::OnClick();

    if (!isClickProcessing_)
    {
        OnClickStart();
        isClickProcessing_ = true;
    }
}

void UIButton::OnMouseDown()
{
    UISelectable::OnMouseDown();
}

void UIButton::OnMouseUp()
{
    UISelectable::OnMouseUp();
}

void UIButton::OnFocusGained()
{
    UISelectable::OnFocusGained();

    if (onFocusGainedCallback_)
    {
        onFocusGainedCallback_();
    }
}

void UIButton::OnFocusLost()
{
    UISelectable::OnFocusLost();

    if (onFocusLostCallback_)
    {
        onFocusLostCallback_();
    }
}

void UIButton::OnClickStart()
{
    if (onClickStart_)
    {
        onClickStart_();
    }
}

void UIButton::OnClickEnd()
{
    if (onClickEnd_)
    {
        onClickEnd_();
    }
    isClickProcessing_ = false;
}

void UIButton::CraeteUIText(const std::wstring& _text, const FontConfig& _config)
{
    text_ = std::make_shared<UIText>();
    text_->Initialize(label_ + "_text", _text, _config, false);

    AddChild(text_);
}
