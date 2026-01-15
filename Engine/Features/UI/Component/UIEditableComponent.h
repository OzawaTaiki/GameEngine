#pragma once

#include "UIComponent.h"
#include <functional>


namespace Engine {

class Input;

/// <summary>
/// 編集可能コンポーネント
/// UIが編集モードかどうかを管理する
/// </summary>
class UIEditableComponent : public UIComponent
{
public:
    UIEditableComponent();
    ~UIEditableComponent() override = default;

    void Initialize() override {}
    void Update() override;
    void Draw() override {}

    void HandleKeyInput(Input* input);

    void SetOnEditStartCallback(const std::function<void()>& callback) { onEditStart_ = callback; }
    void SetOnEditEndCallback(const std::function<void()>& callback) { onEditEnd_ = callback; }
    void SetOnEditingInputCallback(const std::function<void(Input*)>& callback) { onEditingInput_ = callback; }


    // 編集中か取得
    bool IsEditing() const { return isEditing_; }
    // 編集中か設定
    void SetEditing(bool isEditing) { isEditing_ = isEditing; }

private:

    bool isEditing_ = false;
    bool wasEditing_ = false;

    // 編集開始時のコールバック
    std::function<void()> onEditStart_ = nullptr;
    // 編集終了時のコールバック
    std::function<void()> onEditEnd_ = nullptr;
    // 編集入力時のコールバック
    std::function<void(Input*)> onEditingInput_ = nullptr;

};

} // namespace Engine
