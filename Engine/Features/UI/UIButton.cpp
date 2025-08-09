#include "UIButton.h"
#include <System/Input/Input.h>
#include <Debug/ImGuiManager.h>

UIButton::UIButton() :
    UISelectable(),
    onClickStart_([this]() { OnClickStart(); }),
    onClickEnd_([this]() { OnClickEnd(); })

{
}

void UIButton::Initialize(const std::string& _label)
{
    UISelectable::Initialize(_label);
}

void UIButton::Initialize(const std::string& _label, const std::wstring& _text)
{
    UISelectable::Initialize(_label, _text);
}

void UIButton::Initialize(const std::string& _label, const std::wstring& _text, const FontConfig& _config)
{
    UISelectable::Initialize(_label, _text, _config);
}

void UIButton::Update()
{
#ifdef _DEBUG
    ImGui();
#endif // _DEBUG

    UISelectable::Update();

    // クリック処理
    if (isTriggered_)
    {
        if (onClickStart_)
        {
            onClickStart_();
            isClickEnd_ = true;
        }
        isTriggered_ = false;
    }
    else if (isClickEnd_)
    {
        if (onClickEnd_)
        {
            onClickEnd_();
            isClickEnd_ = false;
        }
        else
        {
            isClickEnd_ = false;
        }
    }
}

void UIButton::Draw()
{
    UISelectable::Draw();
}

bool UIButton::HandleInput()
{
    if (!isFocused_)
        return false;

    Input* input = Input::GetInstance();

    // Enter、Space、パッドAボタンでクリック
    if (IsConfirmed())
    {
        Pressed();
        return true;
    }

    return false;
}

bool UIButton::IsPressed()
{
    if (IsMousePointerInside() &&
        Input::GetInstance()->IsMouseTriggered(0))
    {
        Pressed();
        return true;
    }

    return false;
}

bool UIButton::IsPressed(PadButton _button)
{
    // 選択されていないときは押されていないとする
    if (!isFocused_)
        return false;

    if (Input::GetInstance()->IsPadTriggered(_button))
    {
        Pressed();
        return true;
    }

    return false;
}

void UIButton::Pressed()
{
    isTriggered_ = true;
}

void UIButton::SetColor(const Vector4& _color)
{
    UISelectable::SetColor(_color);
    defaultColor_ = _color; // デフォルトカラーも更新
}

void UIButton::SetCallBacks(std::function<void()> _onFocusGained,
    std::function<void()> _onFocusLost,
    std::function<void()> _onFocusUpdate,
    std::function<void()> _onClickStart,
    std::function<void()> _onClickEnd)
{
    SetCallBackOnFocusGained(_onFocusGained);
    SetCallBackOnFocusLost(_onFocusLost);
    SetCallBackOnFocusUpdate(_onFocusUpdate);
    onClickStart_ = _onClickStart;
    onClickEnd_ = _onClickEnd;
}

void UIButton::OnClickStart()
{
    // クリック開始時の処理
    isClickEnd_ = true;
    isTriggered_ = false;
}

void UIButton::OnClickEnd()
{
    // クリック終了時の処理
    isClickEnd_ = false;
}