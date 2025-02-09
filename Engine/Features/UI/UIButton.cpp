#include "UIButton.h"

#include <System/Input/Input.h>

void UIButton::Initialize(const std::string& _label)
{
    UIBase::Initialize(_label);
}

void UIButton::Update()
{
#ifdef _DEBUG
    ImGui();
#endif // _DEBUG

}

void UIButton::Draw()
{
    UIBase::Draw();
}

bool UIButton::IsPressed() const
{
    if (IsMousePointerInside() &&
        Input::GetInstance()->IsMouseTriggered(0))
    {
        return true;
    }

    return false;
}

bool UIButton::IsPressed(PadButton _button) const
{
    // TODO:
    // ボタンが選択されているか

    if (Input::GetInstance()->IsPadTriggered(_button))
    {
        return true;
    }

    return false;
}
