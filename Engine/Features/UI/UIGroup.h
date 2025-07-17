#pragma once

#include <Features/UI/UIBase.h>
#include <Features/UI/ButtonNavigator.h>

#include <vector>
#include <memory>
#include <string>

class UISprite;
class UIButton;

class UIGroup
{
public:

    UIGroup() = default;
    ~UIGroup() = default;

    // 初期化
    void Initialize();

    // 更新
    void Update();

    // 描画
    void Draw();

    // フォーカスを持つボタンを設定
    void SetFocusedButton(UIButton* _button) { buttonNavigator_.SetFocusedButton(_button); }


    // ボタンの生成
    UIButton* CreateButton(const std::string& _label);
    // スプライトの生成
    UISprite* CreateSprite(const std::string& _label);

public: // 静的メンバ関数

    static void LinkVertical(std::initializer_list<UIButton*> _buttons);

    static void LinkHorizontal(std::initializer_list<UIButton*> _buttons);

    static void LinkGrid(std::initializer_list<std::initializer_list<UIButton*>> _buttons);

private:

    static void SetupVerticalNavigation(const std::vector<UIButton*>& _buttons);

    static void SetupHorizontalNavigation(const std::vector<UIButton*>& _buttons);

    static void SetupGridNavigation(const std::vector<std::vector<UIButton*>>& _buttons);
private:

    ButtonNavigator buttonNavigator_; // ボタンナビゲーター

    std::vector<std::unique_ptr<UIBase>> uiElements_; // UI要素のコンテナ

};