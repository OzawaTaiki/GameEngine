#pragma once

#include "UISelectable.h"
#include <Features/TextRenderer/TextGenerator.h>
#include <Features/UI/UIText.h>
enum class PadButton;

class UIButton final : public UISelectable
{
public:

    UIButton();
    ~UIButton() = default;

    // 初期化
    void Initialize(const std::string& _label);
    void Initialize(const std::string& _label, const std::wstring& _text);
    void Initialize(const std::string& _label, const std::wstring& _text, const FontConfig& _config);

    // 更新
    void UpdateSelf() override;
    // 描画
    void Draw() override;

    UIText* GetTextObject() { return text_.get(); }

    // テキスト操作
    void SetText(const std::wstring& _text);
    const std::wstring& GetText() const;

    void SetOnClickStart(std::function<void()> _callback);
    void SetOnClickEnd(std::function<void()> _callback);
    void SetOnFocusGained(std::function<void()> _callback) { onFocusGainedCallback_ = _callback; }
    void SetOnFocusLost(std::function<void()> _callback) { onFocusLostCallback_ = _callback; }

    void SetBackgroundColor(const Vector4& _color);

    void ImGuiContent() override;

protected:

    // UIInteractiveの仮想関数のオーバーライド
    void OnClick() override;
    void OnMouseDown() override;
    void OnMouseUp() override;

    // UISelectableの仮想関数のオーバーライド
    void OnFocusGained() override;
    void OnFocusLost() override;

    // デフォルトコールバック
    virtual void OnClickStart();
    virtual void OnClickEnd();



private:

    void CraeteUIText(const std::wstring& _text, const FontConfig& _config);


private:
    // クリック状態
    bool isClickProcessing_ = false;

    // クリック関連コールバック
    std::function<void()> onClickStart_ = nullptr;
    std::function<void()> onClickEnd_ = nullptr;

    // フォーカス関連コールバック
    std::function<void()> onFocusGainedCallback_ = nullptr;
    std::function<void()> onFocusLostCallback_ = nullptr;

    //TextGenerator textGenerator_;
    //std::wstring text_ = L"";
    //TextParam textParam_;
    //Vector2 textOffset_ = { 0,0 };
    std::shared_ptr<UIText> text_;

    // ビジュアル
    Vector4 backgroundColor_ = { 0.8f, 0.8f, 0.8f, 1.0f };
    //Vector4 textColor_ = { 0.0f, 0.0f, 0.0f, 1.0f };
};