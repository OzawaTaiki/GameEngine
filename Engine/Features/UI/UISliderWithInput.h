#pragma once

#include <Features/UI/UISelectable.h>
#include <Features/UI/UISlider.h>
#include <Features/UI/UITextBox.h>

class UISliderWithInput : public UISelectable
{
public:
    UISliderWithInput() = default;
    ~UISliderWithInput() override = default;

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="_label">UI識別用ラベル</param>
    /// <param name="_minValue">スライダー最小値</param>
    /// <param name="_maxValue">スライダー最大値</param>
    void Initialize(const std::string& label, float minValue = 0.0f, float maxValue = 1.0f, float initialValue = 0.0f);

    void Initialize(const std::string& label, int32_t minValue, int32_t maxValue, int32_t initialValue);

    /// <summary>
    /// 更新
    /// </summary>
    void UpdateSelf() override;
    /// <summary>
    /// 描画
    /// </summary>
    void Draw() override;

    /// <summary>
    /// デバッグ用ImGui
    /// </summary>
    void ImGuiContent() override;

    void SetOnValueChanged(std::function<void(float)> callback);

    template<typename T>
    void SetValue(T value)
    {
        slider_->SetValue(static_cast<float>(value));
        inputBox_->SetValue(value);
    }

    template<typename T>
    void SetRange(T min, T max)
    {
        minValue_ = static_cast<float>(min);
        maxValue_ = static_cast<float>(max);
        slider_->SetRange(minValue_, maxValue_);
    }

    template<typename T>
    void GetValueAs(T& outValue) const
    {
        inputBox_->GetValueAs(outValue);
    }

    float GetValue() const { return slider_->GetValue(); }

private:

    void InitCommon(const std::string& label, float min, float max, float initialValue);

    void OnConfirmedInputBox();

private:
    InputTextType inputTextType_;// 入力テキストタイプ

    float minValue_ = 0.0f; // 最小値
    float maxValue_ = 1.0f; // 最大値

    std::shared_ptr<UISlider> slider_; // スライダー
    std::shared_ptr<UITextBox> inputBox_;// 入力ボックス
};