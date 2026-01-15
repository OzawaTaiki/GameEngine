#pragma once

#include <Features/UI/UISelectable.h>
#include <Features/UI/UIText.h>

#include <functional>


namespace Engine {

class UIGroup;
class UISlider final : public UISelectable
{
public:

    UISlider()  = default;
    ~UISlider() = default;

    void Initialize(const std::string& _label, float _minValue = 0.0f, float _maxValue = 1.0f, bool _regsterDebugWindow = true);

    void UpdateSelf() override;
    void Draw() override;

    UIText* GetLabelText() { return labelText_.get(); }
    UIText* GetValueText() { return valueText_.get(); }

    // 値の設定
    void SetValue(float _value);
    float GetValue() const { return value_; }
    float GetNormalizedValue() const;

    void SetRange(float _min, float _max);
    float GetMinValue(float _minValue) { minValue_ = _minValue; }
    float GetMaxValue(float _maxValue) { maxValue_ = _maxValue; }

    // ステップモードの設定
    void SetStepSize(float _stepSize) { stepSize_ = _stepSize; }
    void SetUseStep(bool _useStep) { useStep_ = _useStep; }

    // コールバックの設定
    void SetOnValueChanged(std::function<void(float)> _callback) { onValueChanged_ = _callback; }

    // カラー設定
    void SetTrackColor(const Vector4& _color);
    void SetFillColor(const Vector4& _color);
    void SetHandleColor(const Vector4& _color);
    void SetHandleHoverColor(const Vector4& _color);
    void SetHandlePressedColor(const Vector4& _color);

    void SetHandleSize(const Vector2& _size);
    void SetTrackHeight(float _height);


    // 方向設定
    enum class SliderDirection
    {
        Horizontal,// 水平
        Vertical // 垂直
    };

    void SetDirection(SliderDirection _dir) { direction_ = _dir; }

    void ImGuiContent() override;

protected:

    void OnClick() override;

    float ClampValue(float _value) const;
    float ApplyStep(float _value) const;

    void UpdateValueFromMousePosition(const Vector2& _pos);
    void UpdateHandlePosition();

    void UpdateValueText();


protected:

    float value_ = 0.5f;        // スライダーの値 (0.0f ~ 1.0f)
    float minValue_ = 0.0f;     // 最小値
    float maxValue_ = 1.0f;     // 最大値
    float stepSize_ = 0.01f;    // キーボード操作での変更量
    bool useStep_ = false;   // ステップモードを使用するかどうか
    float prevValue_ = 0.0f;     // 前回の値

    // コールバック
    std::function<void(float)> onValueChanged_ = nullptr;

    // カラー
    Vector4 trackColor_     = Vector4(0.3f, 0.3f, 0.3f, 1.0f); // トラックの色
    Vector4 fillColor_      = Vector4(0.7f, 0.7f, 0.7f, 1.0f); // 塗りつぶしの色
    Vector4 handleColor_    = Vector4(0.9f, 0.9f, 0.9f, 1.0f); // ハンドルの色

    Vector2 handleSize_     = Vector2(20.0f, 40.0f); // ハンドルのサイズ
    float trackHeight_      = 10.0f;                 // トラックの高さ
    SliderDirection direction_    = SliderDirection::Horizontal; // スライダーの方向

    std::shared_ptr<UIBase> track_  = nullptr;  // トラックのUI要素
    std::shared_ptr<UISelectable> handle_ = nullptr;  // ハンドルのUI要素
    std::shared_ptr<UIBase> fill_   = nullptr;  // 塗りつぶしのUI要素

    std::shared_ptr<UIText> labelText_ = nullptr; // ラベル表示用テキスト
    std::shared_ptr<UIText> valueText_ = nullptr; // 値表示用テキスト

    bool isDraggingHandle_ = false;   // ドラッグ中かどうか

};

} // namespace Engine
