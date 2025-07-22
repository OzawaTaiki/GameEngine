#include "UISprite.h"

void UISprite::Initialize(const std::string& _label)
{
    UIBase::Initialize(_label);
}

void UISprite::Initialize(const std::string& _label, const std::wstring& _text)
{
    UIBase::Initialize(_label, _text);
}

void UISprite::Update()
{
#ifdef _DEBUG
    //UIBase::ImGui();
#endif // _DEBUG
}

void UISprite::Draw()
{
    UIBase::Draw();
}
