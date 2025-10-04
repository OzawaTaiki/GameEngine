#include "UISprite.h"

void UISprite::Initialize(const std::string& _label)
{
    UIBase::Initialize(_label);
}

void UISprite::Initialize(const std::string& _label, const std::wstring& _text)
{
    Initialize(_label);
    text_ = _text;

    textParam_.SetColor({ 0,0,0,1 });
    textParam_.SetPosition(GetWorldPos() + textOffset_);

    textGenerator_.Initialize(FontConfig());
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

    if (!text_.empty())
    {
        textParam_.position = GetWorldPos() + textOffset_;
        textGenerator_.Draw(text_, textParam_);
    }

    UIBase::Draw();
}
