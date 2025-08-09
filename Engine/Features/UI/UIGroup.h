#pragma once

#include <Features/UI/UIBase.h>
#include <Features/UI/UINavigator.h>
#include <Features/UI/UIButton.h>
#include <Features/UI/UISprite.h>
#include <Features/UI/UISlider.h>
#include <Features/UI/UISelectable.h>


#include <vector>
#include <memory>
#include <string>


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

    // フォーカスを持つ要素を設定
    void SetFocused(UISelectable* _element) { navigator_.SetFocused(_element); }
    UISelectable* GetFocused() const { return navigator_.GetFocused(); }

    // 要素の生成
    UIButton* CreateButton(const std::string& _label, const std::wstring& _text = L"");
    UISprite* CreateSprite(const std::string& _label, const std::wstring& _text = L"");
    UISlider* CreateSlider(const std::string& _label, const std::wstring& _text = L"");

    // 任意のUI要素を追加（UISelectableを継承した要素）
    template<typename T>
    T* CreateElement(const std::string& _label, const std::wstring& _text = L"")
    {
        static_assert(std::is_base_of_v<UIBase, T>, "T must inherit from UIBase");

        auto element = std::make_unique<T>();
        if (_text.empty())
            element->Initialize(_label);
        else
            element->Initialize(_label, _text);

        // UISelectableを継承している場合はNavigatorに登録
        if constexpr (std::is_base_of_v<UISelectable, T>)
        {
            navigator_.RegisterSelectable(element.get());
        }

        uiElements_.push_back(std::move(element));
        return static_cast<T*>(uiElements_.back().get());
    }

    template<>
    UISlider* CreateElement<UISlider>(const std::string& _label, const std::wstring& _text )
    {
        auto element = std::make_unique<UISlider>(this);

         uiElements_.push_back(std::move(element));
         UISlider* newSlider = static_cast<UISlider*>(uiElements_.back().get());

        navigator_.RegisterSelectable(newSlider);

        if (_text.empty())
            newSlider->Initialize(_label);
        else
            newSlider->Initialize(_label, _text);

        return newSlider;

    }

    // 要素を手動で追加
    void AddElement(std::unique_ptr<UIBase> _element);

    // 要素を削除
    void RemoveElement(UIBase* _element);

public: // 静的メンバ関数

    static void LinkVertical(std::initializer_list<UISelectable*> _elements);
    static void LinkHorizontal(std::initializer_list<UISelectable*> _elements);
    static void LinkGrid(std::initializer_list<std::initializer_list<UISelectable*>> _elements);

private:

    static void SetupVerticalNavigation(const std::vector<UISelectable*>& _elements);
    static void SetupHorizontalNavigation(const std::vector<UISelectable*>& _elements);
    static void SetupGridNavigation(const std::vector<std::vector<UISelectable*>>& _elements);

private:

    UINavigator navigator_; // UIナビゲーター

    std::vector<std::unique_ptr<UIBase>> uiElements_; // UI要素のコンテナ
};