#include "UISlider.h"
#include <System/Input/Input.h>
#include <Features/UI/UIGroup.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Debug/Debug.h>

#include <algorithm>

void UISlider::Initialize(const std::string& _label, float _minValue, float _maxValue)
{
    UIBase::Initialize(_label);

    minValue_ = _minValue;
    maxValue_ = _maxValue;
    value_ = _minValue;
    prevValue_ = value_;

    // JsonBinderに登録
    jsonBinder_->RegisterVariable(_label + "_value", &value_);
    jsonBinder_->RegisterVariable(_label + "_minValue", &minValue_);
    jsonBinder_->RegisterVariable(_label + "_maxValue", &maxValue_);
    jsonBinder_->RegisterVariable(_label + "_stepSize", &stepSize_);
    jsonBinder_->RegisterVariable(_label + "_useStep", &useStep_);
    jsonBinder_->RegisterVariable(_label + "_trackColor", &trackColor_);
    jsonBinder_->RegisterVariable(_label + "_fillColor", &fillColor_);
    jsonBinder_->RegisterVariable(_label + "_handleColor", &handleColor_);
    jsonBinder_->RegisterVariable(_label + "_handleSize", &handleSize_);
    jsonBinder_->RegisterVariable(_label + "_trackHeight", &trackHeight_);

    // Track UI作成（表示のみ）
    track_ = std::make_shared<UIBase>();
    track_->Initialize(_label + "_track");
    track_->SetTextureNameAndLoad("white.png");
    AddChild(track_);

    // Fill UI作成（表示のみ）
    fill_ = std::make_shared<UIBase>();
    fill_->Initialize(_label + "_fill");
    fill_->SetTextureNameAndLoad("white.png");
    AddChild(fill_);

    // Handle UI作成（UISelectable - ドラッグ可能）
    handle_ = std::make_shared<UISelectable>();
    handle_->Initialize(_label + "_handle");
    handle_->SetTextureNameAndLoad("white.png");
    handle_->SetAnchor({ 0.5f, 0.5f });
    handle_->SetDefaultColor(handleColor_);
    AddChild(handle_);

    // Handleのドラッグイベント設定
    handle_->SetOnDragStart([this]()
                            {
                                // ドラッグ開始時の処理
                                Debug::Log("Drag Start");
                            });

    handle_->SetOnDragging([this]()
                           {
                               // ドラッグ中はマウス位置から値を更新
                               Vector2 mousePos = Input::GetInstance()->GetMousePosition();
                               UpdateValueFromMousePosition(mousePos);
                           });

    handle_->SetOnDragEnd([this]()
                          {
                              // ドラッグ終了時の処理
                              Debug::Log("Drag End");
                          });

    // TODO : colorタグなどenumを用意
    // SetColor(color,Tag::Default);みたいにひとつの関数にしたい
    // 背景は基本不要なので 透明に設定
    SetDefaultColor (Vector4{ 0.0f,0.0f,0.0f,0.0f });
    SetHoverColor   (Vector4{ 0.0f,0.0f,0.0f,0.0f });
    SetPressedColor (Vector4{ 0.0f,0.0f,0.0f,0.0f });
    SetSelectedColor(Vector4{ 0.0f,0.0f,0.0f,0.0f });
    SetFocusedColor (Vector4{ 0.0f,0.0f,0.0f,0.0f });


}

void UISlider::UpdateSelf()
{
    UISelectable::UpdateSelf();

    // 値が変化したらコールバックを呼び出す
    if (prevValue_ != value_)
    {
        if (onValueChanged_)
        {
            onValueChanged_(value_);
        }
        prevValue_ = value_;
    }

    // キーボード入力でスライダー操作（フォーカス時）
    if (isFocused_)
    {
        float step = useStep_ ? stepSize_ : (maxValue_ - minValue_) * 0.01f;

        if (direction_ == SliderDirection::Horizontal)
        {
            /*if (IsActionTriggered(UIAction::Left))
            {
                SetValue(value_ - step);
            }
            if (IsActionTriggered(UIAction::Right))
            {
                SetValue(value_ + step);
            }*/
        }
        else // Vertical
        {
            /*if (IsActionTriggered(UIAction::Up))
            {
                SetValue(value_ + step);
            }
            if (IsActionTriggered(UIAction::Down))
            {
                SetValue(value_ - step);
            }*/
        }
    }

    // Handleの位置を更新
    UpdateHandlePosition();
}

void UISlider::Draw()
{
    if (!isVisible_)
        return;

    if (direction_ == SliderDirection::Horizontal)
    {
        // トラック設定
        track_->SetPos({ 0, size_.y * 0.5f - trackHeight_ * 0.5f });
        track_->SetSize({ size_.x, trackHeight_ });
        track_->SetColor(trackColor_);

        // フィル設定
        float fillWidth = size_.x * GetNormalizedValue();
        fill_->SetPos({ 0, size_.y * 0.5f - trackHeight_ * 0.5f });
        fill_->SetSize({ fillWidth, trackHeight_ });
        fill_->SetColor(fillColor_);

        // ハンドル設定
        float handleX = size_.x * GetNormalizedValue();
        handle_->SetPos({ handleX, size_.y * 0.5f });
        handle_->SetSize(handleSize_);
    }
    else // Vertical
    {
        // トラック設定
        track_->SetPos({ size_.x * 0.5f - trackHeight_ * 0.5f, 0 });
        track_->SetSize({ trackHeight_, size_.y });
        track_->SetColor(trackColor_);

        // フィル設定
        float fillHeight = size_.y * GetNormalizedValue();
        track_->SetPos({ size_.x * 0.5f - trackHeight_ * 0.5f, size_.y - fillHeight });
        track_->SetSize({ trackHeight_, fillHeight });
        fill_->SetColor(fillColor_);

        // ハンドル設定
        float handleY = size_.y * (1.0f - GetNormalizedValue());
        handle_->SetPos({ size_.x * 0.5f, handleY });
        handle_->SetSize(handleSize_);
    }

    // 親クラスのDraw()で子要素(track, fill, handle)を自動描画
    UIBase::Draw();
}

void UISlider::SetValue(float _value)
{
    float newValue = ClampValue(_value);

    if (useStep_)
    {
        newValue = ApplyStep(newValue);
    }

    value_ = newValue;
}

float UISlider::GetNormalizedValue() const
{
    if (maxValue_ == minValue_)
        return 0.0f;

    return (value_ - minValue_) / (maxValue_ - minValue_);
}

void UISlider::SetRange(float _min, float _max)
{
    minValue_ = _min;
    maxValue_ = _max;
    SetValue(value_); // 現在値を範囲内に収める

}

void UISlider::SetTrackColor(const Vector4& _color)
{
    trackColor_ = _color;
    if (track_)
    {
        track_->SetColor(_color);
    }
}

void UISlider::SetFillColor(const Vector4& _color)
{
    fillColor_ = _color;
    if (fill_)
    {
        fill_->SetColor(_color);
    }
}

void UISlider::SetHandleColor(const Vector4& _color)
{
    handleColor_ = _color;
    if (handle_)
    {
        handle_->SetDefaultColor(_color);
    }
}

void UISlider::SetHandleHoverColor(const Vector4& _color)
{
    if (handle_)
    {
        handle_->SetHoverColor(_color);
    }
}

void UISlider::SetHandlePressedColor(const Vector4& _color)
{
    if (handle_)
    {
        handle_->SetPressedColor(_color);
    }
}

void UISlider::SetHandleSize(const Vector2& _size)
{
    handleSize_ = _size;
    if (handle_)
    {
        handle_->SetSize(_size);
    }
}

void UISlider::SetTrackHeight(float _height)
{
    trackHeight_ = _height;
}

void UISlider::ImGui()
{
#ifdef _DEBUG
    UISelectable::ImGui();

    ImGui::PushID("Slider");

    if (ImGui::TreeNode("Slider Settings"))
    {
        ImGui::DragFloat("value", &value_, 0.1f, minValue_, maxValue_);
        ImGui::DragFloat("minValue", &minValue_);
        ImGui::DragFloat("maxValue", &maxValue_);
        ImGui::DragFloat("stepSize", &stepSize_, 0.01f);
        ImGui::Checkbox("useStep", &useStep_);

        ImGui::Separator();

        if (ImGui::ColorEdit4("trackColor", &trackColor_.x))
        {
            SetTrackColor(trackColor_);
        }
        if (ImGui::ColorEdit4("fillColor", &fillColor_.x))
        {
            SetFillColor(fillColor_);
        }
        if (ImGui::ColorEdit4("handleColor", &handleColor_.x))
        {
            SetHandleColor(handleColor_);
        }

        if (ImGui::DragFloat2("handleSize", &handleSize_.x, 0.5f))
        {
            SetHandleSize(handleSize_);
        }
        ImGui::DragFloat("trackHeight", &trackHeight_, 0.5f);

        const char* directions[] = { "Horizontal", "Vertical" };
        int currentDir = static_cast<int>(direction_);
        if (ImGui::Combo("Direction", &currentDir, directions, 2))
        {
            direction_ = static_cast<SliderDirection>(currentDir);
        }

        ImGui::TreePop();
    }

    ImGui::PopID();
#endif // _DEBUG
}

void UISlider::OnClick()
{
    UISelectable::OnClick();

    // トラック部分をクリックした場合、その位置にジャンプ
    Vector2 mousePos = Input::GetInstance()->GetMousePosition();
    UpdateValueFromMousePosition(mousePos);

}

float UISlider::ClampValue(float _value) const
{
    return std::max(minValue_, std::min(maxValue_, _value));
}

float UISlider::ApplyStep(float _value) const
{
    if (stepSize_ <= 0.0f)
        return _value;

    float steps = std::round((_value - minValue_) / stepSize_);
    return minValue_ + steps * stepSize_;
}

void UISlider::UpdateValueFromMousePosition(const Vector2& _pos)
{
    Vector2 worldPos = GetWorldPos();
    float t = 0.0f;

    if (direction_ == SliderDirection::Horizontal)
    {
        float localX = _pos.x - worldPos.x;
        t = localX / size_.x;
    }
    else // Vertical
    {
        float localY = _pos.y - worldPos.y;
        t = 1.0f - (localY / size_.y); // 上が最大値
    }

    t = std::max(0.0f, std::min(1.0f, t));
    float newValue = minValue_ + (maxValue_ - minValue_) * t;
    SetValue(newValue);
}

void UISlider::UpdateHandlePosition()
{
    if (!handle_)
        return;

    if (direction_ == SliderDirection::Horizontal)
    {
        float handleX = size_.x * GetNormalizedValue();
        handle_->SetPos({ handleX, size_.y * 0.5f });
    }
    else // Vertical
    {
        float handleY = size_.y * (1.0f - GetNormalizedValue());
        handle_->SetPos({ size_.x * 0.5f, handleY });
    }
}
