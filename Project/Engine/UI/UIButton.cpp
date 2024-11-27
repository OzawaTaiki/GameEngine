#include "UIButton.h"

#include "ConfigManager.h"
#include "Input.h"

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

bool UIButton::IsClicked() const
{
    if (IsMousePointerInside() &&
        Input::GetInstance()->IsMouseTriggered(0))
    {
        return true;
    }

    return false;
}
