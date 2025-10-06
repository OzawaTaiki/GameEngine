#include "UISprite.h"

void UISprite::Initialize(const std::string& _label)
{
    UIBase::Initialize(_label, true);
}

void UISprite::Initialize(const std::string& _label, const std::wstring& _text)
{
    Initialize(_label);

    textUI_ = std::make_shared<UIText>();
    textUI_->Initialize(_label + "_text", _text, FontConfig(), false);

    AddChild(textUI_);
}

void UISprite::Update()
{
#ifdef _DEBUG
    //UIBase::ImGui();
#endif // _DEBUG
}

void UISprite::Draw()
{
    if (!isVisible_)
        return;

    UIBase::Draw();
}
