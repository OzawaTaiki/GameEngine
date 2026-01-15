#pragma once

#include <vector>

// 前方宣言

namespace Engine {

class UIElement;
class UINavigationComponent;

/// <summary>
/// UI要素のナビゲーションとフォーカス管理を行うマネージャー
/// </summary>
class UINavigationManager
{
public:
    static UINavigationManager* GetInstance();

    // 要素の登録/登録解除
    void Register(UIElement* element);
    void Unregister(UIElement* element);

    // 入力処理
    void HandleInput();

    // フォーカス管理
    void SetFocus(UIElement* element);
    UIElement* GetFocus() const { return currentFocus_; }
    void ClearFocus();

    // 決定キー（Enter/A）処理
    void ExecuteCurrentElement();

private:
    UINavigationManager() = default;
    ~UINavigationManager() = default;
    UINavigationManager(const UINavigationManager&) = delete;
    UINavigationManager& operator=(const UINavigationManager&) = delete;

    std::vector<UIElement*> focusableElements_;
    UIElement* currentFocus_ = nullptr;

    // ヘルパー関数
    UINavigationComponent* GetNavigationComponent(UIElement* element) const;
};

} // namespace Engine
