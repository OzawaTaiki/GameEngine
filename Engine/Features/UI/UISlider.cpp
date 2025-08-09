#include "UISlider.h"
#include <System/Input/Input.h>
#include <Features/UI/UIGroup.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Debug/Debug.h>

#include <algorithm>

UISlider::UISlider(UIGroup* _group) : UISelectable()
{
    // デフォルトのスライダー背景色を設定
    defaultColor_ = Vector4(0.3f, 0.3f, 0.3f, 1.0f);
    color_ = defaultColor_;
    handleDefaultColor_ = handleColor_;

    if(_group)
    {
        group_ = _group;
        isInGroup_ = true;
    }
    else
    {
        group_ = new UIGroup();
        isInGroup_ = false;
    }

}

UISlider::~UISlider()
{
    if (!isInGroup_)
    {
        delete handle_;
    }

    handle_ = nullptr; // ハンドルのポインタをnullptrに設定
}

void UISlider::Initialize(const std::string& _label)
{
    UISelectable::Initialize(_label);

    InitilizeHandle(_label);

    // デフォルトサイズを設定
    if (size_.x == 0) size_.x = 200.0f;
    if (size_.y == 0) size_.y = 20.0f;
}

void UISlider::Initialize(const std::string& _label, const std::wstring& _text)
{
    UISelectable::Initialize(_label, _text);

    InitilizeHandle(_label);

    // デフォルトサイズを設定
    if (size_.x == 0) size_.x = 200.0f;
    if (size_.y == 0) size_.y = 20.0f;
}

void UISlider::Initialize(const std::string& _label, const std::wstring& _text, const FontConfig& _config)
{
    UISelectable::Initialize(_label, _text, _config);

    InitilizeHandle(_label);

    // デフォルトサイズを設定
    if (size_.x == 0) size_.x = 200.0f;
    if (size_.y == 0) size_.y = 20.0f;
}

void UISlider::Update()
{
    UISelectable::Update();
    //handle_->Update();
    if (!isActive_)
        return;

    UpdateValueFromMouse();

    if (!isInGroup_)
        group_->Update();
}

void UISlider::Draw()
{
    if (!isVisible_)
        return;

    // スライダー背景を描画
    UISelectable::Draw();

    // ハンドルを描画
    Vector2 handlePos = CalculateHandlePosition();
    handle_->SetPos(handlePos);
    handle_->SetSize(handleSize_);
    handle_->SetColor(handleColor_);

    if (!isInGroup_)
        group_->Draw();
}

bool UISlider::HandleInput()
{
    if (!isFocused_)
        return false;

    UpdateValueFromKeyboard();
    return true; // スライダーは常に入力を処理したとみなす
}

void UISlider::SetValue(float _value)
{
    float newValue = std::clamp(_value, 0.0f, 1.0f);

    if (value_ != newValue)
    {
        value_ = newValue;
        OnValueChanged();
    }
}

void UISlider::SetRange(float _min, float _max)
{
    minValue_ = _min;
    maxValue_ = _max;
}

float UISlider::GetRealValue() const
{
    return minValue_ + (maxValue_ - minValue_) * value_;
}

void UISlider::SetRealValue(float _realValue)
{
    float normalizedValue = (maxValue_ - minValue_ != 0.0f) ?
        (_realValue - minValue_) / (maxValue_ - minValue_) : 0.0f;
    SetValue(normalizedValue);
}

void UISlider::SetColor(const Vector4& _color)
{
    UISelectable::SetColor(_color);
    defaultColor_ = _color;
}

void UISlider::OnFocusGained()
{
    // 背景を少し明るく
    color_ = defaultColor_ * 1.2f;
    color_.w = defaultColor_.w; // アルファは維持

    // ハンドルも少し明るく
    handleColor_ = handleDefaultColor_ * 1.2f;
    handleColor_.w = handleDefaultColor_.w; // アルファは維持

    // 親クラスのコールバックも呼び出し
    if (onFocusGained_)
        onFocusGained_();
}

void UISlider::OnFocusLost()
{
    // 元の色に戻す
    color_ = defaultColor_;
    handleColor_ = handleDefaultColor_;

    // ドラッグ状態も解除
    isDragging_ = false;

    // 親クラスのコールバックも呼び出し
    if (onFocusLost_)
        onFocusLost_();
}

void UISlider::InitilizeHandle(const std::string& _label)
{
    // ハンドル用のスプライトを作成
    handleTextureHandle_ = TextureManager::GetInstance()->Load("white.png");

    handle_ = group_->CreateElement<UIButton>(_label + "_handle");

    handle_->Initialize(_label + "_handle");
    handle_->SetParent(this);
    handle_->SetSize(handleSize_);
    handle_->SetAnchor({ 0.5f, 0.5f }); // 中央アンカー

    handle_->SetCallBackOnClickStart([&]()
        {
            isDragging_ = true;
            //isActive_ = false;
            handle_->SetColor(handleColor_ * 1.2f); // ドラッグ開始時に色を明るく
            Debug::Log("Slider handle clicked: " + handle_->GetLabel() + "\n");
        });
}

void UISlider::UpdateValueFromMouse()
{
    Input* input = Input::GetInstance();
    Vector2 mousePos = input->GetMousePosition();

    // マウスクリックでドラッグ開始
    if (IsMousePointerInside() && input->IsMouseTriggered(0))
    {
        isDragging_ = true;
        SetValue(CalculateValueFromMousePosition(mousePos));
    }

    // ドラッグ中の処理
    if (isDragging_)
    {
        if (input->IsMousePressed(0))
        {
            SetValue(CalculateValueFromMousePosition(mousePos));
        }
        else
        {
            isDragging_ = false;
            //isActive_ = true;
            Debug::Log("Slider handle Released: " + handle_->GetLabel() + "\n");
        }
    }
}

void UISlider::UpdateValueFromKeyboard()
{
    Input* input = Input::GetInstance();

    // 左キーまたはAキーで値を減少
    if (input->IsKeyTriggered(DIK_LEFT) || input->IsKeyTriggered(DIK_A))
    {
        SetValue(value_ - stepSize_);
    }
    // 右キーまたはDキーで値を増加
    else if (input->IsKeyTriggered(DIK_RIGHT) || input->IsKeyTriggered(DIK_D))
    {
        SetValue(value_ + stepSize_);
    }

    // パッド入力
    if (input->IsPadTriggered(static_cast<PadButton>(PadButton::iPad_Left)))
    {
        SetValue(value_ - stepSize_);
    }

    else if (input->IsPadTriggered(static_cast<PadButton>(PadButton::iPad_Right)))
    {
        SetValue(value_ + stepSize_);
    }
}
//TODO:
Vector2 UISlider::CalculateHandlePosition() const
{
    Vector2 worldPos = GetWorldPos();
    Vector2 leftTop = worldPos - size_ * anchor_;

    // スライダーの左端から右端までの範囲でハンドル位置を計算
    float handleX = (size_.x * value_);
    float handleY = worldPos.y; // 縦方向は中央

    return Vector2(handleX, 0);
}

float UISlider::CalculateValueFromMousePosition(const Vector2& _mousePos) const
{
    Vector2 worldPos = GetWorldPos();
    Vector2 leftTop = worldPos - size_ * anchor_;

    // マウス位置をスライダーの範囲内で正規化
    float relativeX = _mousePos.x - leftTop.x;
    float normalizedValue = relativeX / size_.x;

    return std::clamp(normalizedValue, 0.0f, 1.0f);
}

void UISlider::OnValueChanged()
{
    if (onValueChanged_)
    {
        onValueChanged_(GetRealValue());
    }
}