#include "UISelectable.h"
#include <Debug/Debug.h>
#include <System/Input/Input.h>

UISelectable::UISelectable() :
    UIBase(),
    confirmKeys_({ DIK_RETURN,
        DIK_SPACE,
        static_cast<int32_t>(PadButton::iPad_A),
        static_cast<int32_t>(MouseButton::Left) })
{
    defaultColor_ = color_;
}

void UISelectable::Initialize(const std::string& _label)
{
    UIBase::Initialize(_label);
    defaultColor_ = color_;
}

void UISelectable::Initialize(const std::string& _label, const std::wstring& _text)
{
    UIBase::Initialize(_label, _text);
    defaultColor_ = color_;
}

void UISelectable::Initialize(const std::string& _label, const std::wstring& _text, const FontConfig& _config)
{
    UIBase::Initialize(_label, _text, _config);
    defaultColor_ = color_;
}

void UISelectable::Update()
{
    UIBase::Update();

    if (!isActive_ || !isFocusable_)
        return;

    // フォーカス中の更新処理
    if (isFocused_)
    {
        OnFocusUpdate();

        // 派生クラスの入力処理
        HandleInput();
    }
}

void UISelectable::SetFocused(bool _focused)
{
    if (isFocused_ == _focused)
        return;

    isFocused_ = _focused;

    if (isFocused_)
    {
        OnFocusGained();
    }
    else
    {
        OnFocusLost();
    }
}

void UISelectable::SetNavigationTarget(UISelectable* _target, Direction _dir)
{
    switch (_dir)
    {
    case Direction::Up:
        upTarget_ = _target;
        if (_target)
            _target->downTarget_ = this; // 双方向リンク
        break;
    case Direction::Down:
        downTarget_ = _target;
        if (_target)
            _target->upTarget_ = this; // 双方向リンク
        break;
    case Direction::Left:
        leftTarget_ = _target;
        if (_target)
            _target->rightTarget_ = this; // 双方向リンク
        break;
    case Direction::Right:
        rightTarget_ = _target;
        if (_target)
            _target->leftTarget_ = this; // 双方向リンク
        break;
    default:
        Debug::Log("Invalid direction for UISelectable navigation target.");
        return;
    }
}

UISelectable* UISelectable::GetNavigationTarget(Direction _dir) const
{
    switch (_dir)
    {
    case Direction::Up:
        return upTarget_;
    case Direction::Down:
        return downTarget_;
    case Direction::Left:
        return leftTarget_;
    case Direction::Right:
        return rightTarget_;
    default:
        Debug::Log("Invalid direction for UISelectable navigation target.");
        return nullptr;
    }
}

bool UISelectable::IsConfirmed() const
{
    Input* input = Input::GetInstance();

    if (!input)
        return false;

    for (const auto& key : confirmKeys_)
    {
        if (input->IsPadTriggered(static_cast<PadButton>(key))
            || input->IsKeyTriggered(key)
            || input->IsMouseTriggered(key))
        {
            return true;
        }
    }
    return false;
}

void UISelectable::OnFocusGained()
{
    // デフォルト：少し暗くする
    color_ = defaultColor_ * 0.8f;

    // カスタムコールバックがあれば実行
    if (onFocusGained_)
        onFocusGained_();
}

void UISelectable::OnFocusLost()
{
    // デフォルト：元の色に戻す
    color_ = defaultColor_;

    // カスタムコールバックがあれば実行
    if (onFocusLost_)
        onFocusLost_();
}

void UISelectable::OnFocusUpdate()
{
    // カスタムコールバックがあれば実行
    if (onFocusUpdate_)
        onFocusUpdate_();
}