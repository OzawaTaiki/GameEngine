#include "UIButton.h"

#include <System/Input/Input.h>
#include <Debug/Debug.h>

UIButton::UIButton() :
    UIBase(),
    isTrigered_(false),
    isClickEnd_(false),
    isFocused_(false),
    defaultColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f)),
    upButton_(nullptr),
    downButton_(nullptr),
    leftButton_(nullptr),
    rightButton_(nullptr),
    onFocusGained_([this]() {OnFocusGained(); }),
    onFocusLost_([this]() {OnFocusLost(); }),
    onFocusUpdate_(nullptr),
    onClickStart_([this]() {OnClickStart(); }),
    onClickEnd_([this]() {OnClickEnd(); })
{
}

void UIButton::Initialize(const std::string& _label)
{
    UIBase::Initialize(_label);
}

void UIButton::Initialize(const std::string& _label, const std::wstring& _text)
{
    UIBase::Initialize(_label, _text);
}

void UIButton::Update()
{
#ifdef _DEBUG
    ImGui();
#endif // _DEBUG


    if (isTrigered_)
    {
        if (onClickStart_)
            onClickStart_(); // クリック開始時のコールバック
        else
            isTrigered_ = false;
    }
    else if (isClickEnd_)
    {
        if (onClickEnd_)
            onClickEnd_(); // クリック終了時のコールバック
        else
            isTrigered_ = false;
    }
    else if (isFocused_)
    {
        if (onFocusUpdate_)
            onFocusUpdate_(); // フォーカス更新時のコールバック
    }
}

void UIButton::Draw()
{
    UIBase::Draw();
}

bool UIButton::IsPressed()
{
    if (IsMousePointerInside() &&
        Input::GetInstance()->IsMouseTriggered(0))
    {
        isTrigered_ = true;
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
        isTrigered_ = true;
        return true;
    }

    return false;
}

void UIButton::Pressed()
{
    isTrigered_ = true;
}

void UIButton::SetColor(const Vector4& _color)
{
    color_ = _color;
    defaultColor = _color; // デフォルトカラーも更新
}

void UIButton::SetNavigationTarget(UIButton* _target, Direction _dir)
{
    // null を代入してリンクを解除できるように nullチェックは行わない

    switch (_dir)
    {
    case Direction::Up:
        upButton_ = _target;
        if (_target)
            _target->downButton_ = this; // 双方向のナビゲーションを設定
        break;
    case Direction::Down:
        downButton_ = _target;
        if (_target)
            _target->upButton_= this; // 双方向のナビゲーションを設定
        break;
    case Direction::Left:
        leftButton_ = _target;
        if (_target)
            _target->rightButton_ = this; // 双方向のナビゲーションを設定
        break;
    case Direction::Right:
        rightButton_ = _target;
        if (_target)
            _target->leftButton_= this; // 双方向のナビゲーションを設定
        break;

    default:
        Debug::Log("Invalid direction for UIButton navigation target.");
        return;
    }

}

UIButton* UIButton::GetNavigationTarget(Direction _dir) const
{
    switch (_dir)
    {
    case Direction::Up:
        return upButton_;
    case Direction::Down:
        return downButton_;
    case Direction::Left:
        return leftButton_;
    case Direction::Right:
        return rightButton_;
    default:
        Debug::Log("Invalid direction for UIButton navigation target.");
        return nullptr;
    }
}

void UIButton::SetFocused(bool _isFocused)
{
    isFocused_ = _isFocused;

    if (isFocused_)
        onFocusGained_();
    else
        onFocusLost_();
}

void UIButton::SetCallBacks(std::function<void(void)> _onFocusGained,
    std::function<void(void)> _onFocusLost,
    std::function<void(void)> _onFocusUpdate,
    std::function<void(void)> _onClickStart,
    std::function<void(void)> _onClickEnd)
{
    onFocusGained_ = _onFocusGained;
    onFocusLost_ = _onFocusLost;
    onFocusUpdate_ = _onFocusUpdate;
    onClickStart_ = _onClickStart;
    onClickEnd_ = _onClickEnd;
}

void UIButton::OnFocusGained()
{
    // 少し暗く
    color_ = defaultColor * 0.8f;
}

void UIButton::OnFocusLost()
{
    color_ = defaultColor;
}

void UIButton::OnClickStart()
{
    // クリック開始時の処理
    isClickEnd_ = true; // クリック終了フラグをリセット
    isTrigered_ = false;
}

void UIButton::OnClickEnd()
{
    // クリック終了時の処理
    isClickEnd_ = false; // クリック終了フラグをリセット
}
