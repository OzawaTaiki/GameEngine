#pragma once

#include <Features/UI/UISelectable.h>
#include <Features/UI/UIButton.h>

#include <functional>

class UIGroup;
class UISlider : public UISelectable
{
public:

    UISlider(UIGroup* _group = nullptr);
    ~UISlider();

    // 初期化
    void Initialize(const std::string& _label) override;
    void Initialize(const std::string& _label, const std::wstring& _text) override;
    void Initialize(const std::string& _label, const std::wstring& _text, const FontConfig& _config) override;

    // 更新
    void Update() override;

    // 描画
    void Draw() override;

    // 入力処理
    bool HandleInput() override;

    // 値を設定する (0.0f ~ 1.0f)
    void SetValue(float _value);

    // 値を取得する
    float GetValue() const { return value_; }

    // 最小値・最大値を設定する
    void SetRange(float _min, float _max);

    // 実際の値を取得する (min~max範囲)
    float GetRealValue() const;

    // 実際の値を設定する (min~max範囲)
    void SetRealValue(float _realValue);

    // スライダーハンドルの色を設定する
    void SetHandleColor(const Vector4& _color) { handleColor_ = _color; }

    // ハンドルのサイズを設定する
    void SetHandleSize(const Vector2& _size) { handleSize_ = _size; }

    // 値変更時のコールバックを設定する
    void SetOnValueChanged(std::function<void(float)> _callback) { onValueChanged_ = _callback; }

    // キーボード操作での変更量を設定する
    void SetStepSize(float _stepSize) { stepSize_ = _stepSize; }

    // 色を設定する（背景色）
    void SetColor(const Vector4& _color) override;

protected:

    // フォーカス時の処理をオーバーライド
    void OnFocusGained() override;
    void OnFocusLost() override;

private:

    void InitilizeHandle(const std::string& _label);

    // マウス/パッド操作による値の更新
    void UpdateValueFromMouse();

    // キーボード操作による値の更新
    void UpdateValueFromKeyboard();

    // ハンドルの位置を計算する
    Vector2 CalculateHandlePosition() const;

    // マウス位置から値を計算する
    float CalculateValueFromMousePosition(const Vector2& _mousePos) const;

    // 値が変更された時の処理
    void OnValueChanged();

private:

    float value_ = 0.5f;        // スライダーの値 (0.0f ~ 1.0f)
    float minValue_ = 0.0f;     // 最小値
    float maxValue_ = 1.0f;     // 最大値
    float stepSize_ = 0.01f;    // キーボード操作での変更量

    bool isDragging_ = false;   // ドラッグ中かどうか

    // ハンドル関連
    UIGroup* group_ = nullptr;  // グループに所属しているかどうか
    UIButton* handle_ = nullptr;
    bool isInGroup_ = false; // グループ内で使用されるかどうか
    uint32_t handleTextureHandle_ = 0;
    Vector2 handleSize_ = { 20.0f, 20.0f };
    Vector4 handleColor_ = { 0.8f, 0.8f, 0.8f, 1.0f };
    Vector4 handleDefaultColor_ = { 0.8f, 0.8f, 0.8f, 1.0f };

    // コールバック
    std::function<void(float)> onValueChanged_ = nullptr;
};