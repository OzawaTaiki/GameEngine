#include "UISliderWithInput.h"

namespace
{
struct DefaultBaseParam
{
    Vector2 pos = { 100.0f,100.0f };
    Vector2 size = { 0.0f,0.0f };
}defaultBase;

struct DefaultTextBoxParam
{
    Vector2 pos = { 128.0f,0.0f }; // スライダーサイズ100想定，28のマージン
    Vector2 size = { 100.0f,48.0f };
    Vector4 defaultColor = { 0.25f,0.25f ,0.25f ,1.0f };
    Vector4 hoverColor = defaultColor;
    Vector4 pressedColor = defaultColor;
    Vector4 selectedColor= defaultColor;
    Vector4 foucusedColor= defaultColor;
} defaultTextBox;
struct DefaultSliderParam
{
    Vector2 pos = { 128.0f,0.0f }; // スライダーサイズ100想定，28のマージン
    Vector2 size = { 100.0f,0.0f };

}defaultSlider;
};

void UISliderWithInput::Initialize(const std::string& label, float minValue, float maxValue, float initialValue)
{
    inputTextType_ = InputTextType::Float;
    InitCommon(label, minValue, maxValue, initialValue);
}

void UISliderWithInput::Initialize(const std::string& label, int32_t minValue, int32_t maxValue, int32_t initialValue)
{
    inputTextType_ = InputTextType::Integer;
    InitCommon(label, static_cast<float>(minValue), static_cast<float>(maxValue), static_cast<float>(initialValue));

    slider_->SetStepSize(1.0f);
    slider_->SetUseStep(true);
}

void UISliderWithInput::UpdateSelf()
{
    UISelectable::UpdateSelf();
    slider_->UpdateSelf();
    inputBox_->UpdateSelf();
}

void UISliderWithInput::Draw()
{
    UISelectable::Draw();
}

void UISliderWithInput::ImGuiContent()
{
    slider_->ImGuiContent();
    inputBox_->ImGuiContent();
}

void UISliderWithInput::SetOnValueChanged(std::function<void(float)> callback)
{
    slider_->SetOnValueChanged(callback);
}

void UISliderWithInput::InitCommon(const std::string& label, float min, float max, float initialValue)
{
    minValue_ = min;
    maxValue_ = max;

    UISelectable::Initialize(label, true);
    SetPos(defaultBase.pos);
    SetSize(defaultBase.size);

    // スライダー初期化
    slider_ = std::make_shared<UISlider>();

    slider_->Initialize(label + "_slider", min, max, false);
    slider_->SetValue(initialValue);
    slider_->SetSize(Vector2(defaultSlider.size.x, 1.0f));
    AddChild(slider_);

    // 入力ボックス初期化
    inputBox_ = std::make_shared<UITextBox>(inputTextType_);

    inputBox_->SetPos(defaultTextBox.pos);
    inputBox_->SetSize(defaultTextBox.size);
    inputBox_->SetDefaultColor(defaultTextBox.defaultColor);
    inputBox_->SetHoverColor(defaultTextBox.hoverColor);
    inputBox_->SetPressedColor(defaultTextBox.pressedColor);
    inputBox_->SetSelectedColor(defaultTextBox.selectedColor);
    inputBox_->SetFocusedColor(defaultTextBox.foucusedColor);

    inputBox_->Initialize(label + "_inputBox", false);
    inputBox_->SetValue(initialValue);
    AddChild(inputBox_);

    if(inputBox_->GetSize().y == 100.0f)
        inputBox_->SetSize(Vector2(inputBox_->GetSize().x, defaultTextBox.size.y));


    // スライダーの値が変更されたとき
    slider_->SetOnValueChanged([this](float value)
                           {
                               // 入力ボックスの値を更新
                               inputBox_->SetValue(value);
                           });

    // 入力ボックスの値が変更されたとき
    inputBox_->SetOnConfirmed([this]() { OnConfirmedInputBox(); });

}

void UISliderWithInput::OnConfirmedInputBox()
{
    if (inputTextType_ == InputTextType::Float)
    {
        float value;
        if (inputBox_->GetValueAs(value))
        {
            // 値をクランプしてスライダーに設定
            float clampedValue = std::clamp(value, minValue_, maxValue_);
            slider_->SetValue(clampedValue);
            inputBox_->SetValue(clampedValue);
        }
        else
        {
            // 変換失敗ならスライダーの現在値で上書き
            inputBox_->SetValue(slider_->GetValue());
        }
    }
    else
    {
        int32_t value;
        if (inputBox_->GetValueAs(value))
        {
            // 値をクランプしてスライダーに設定
            int32_t clampedValue = static_cast<int32_t>(std::clamp(static_cast<float>(value), minValue_, maxValue_));
            slider_->SetValue(static_cast<float>(clampedValue));
            inputBox_->SetValue(clampedValue);
        }
        else
        {
            // 変換失敗ならスライダーの現在値で上書き
            inputBox_->SetValue(slider_->GetValue());
        }
    }
}
