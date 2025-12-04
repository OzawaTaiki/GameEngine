#pragma once

#include <Features/UI/UIBase.h>
#include <Features/UI/UINavigator.h>
#include <Features/UI/UIButton.h>
#include <Features/UI/UISprite.h>
#include <Features/UI/UISlider.h>
#include <Features/UI/UISliderWithInput.h>
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

    UIBase* GetElementByLabel(const std::string& _label) const;
    UIBase* GetElementByIndex(size_t _index) const;
    std::vector<std::shared_ptr<UIBase>> GetAllElements() const { return uiElements_; }

    // 要素の生成
    std::shared_ptr<UIButton> CreateButton(const std::string& _label, const std::wstring& _text = L"");
    std::shared_ptr<UISprite> CreateSprite(const std::string& _label, const std::wstring& _text = L"");
    std::shared_ptr<UISlider> CreateSlider(const std::string& _label, float _minV, float _maxV);
    std::shared_ptr<UISliderWithInput> CreateSliderWithInput(const std::string& _label, float _minV, float _maxV);

    // 任意のUI要素を追加（UISelectableを継承した要素）
    template<typename T>
    std::shared_ptr<T> CreateElement(const std::string& _label, const std::wstring& _text = L"")
    {
        static_assert(std::is_base_of_v<UIBase, T>, "T must inherit from UIBase");

        auto element = std::make_shared<T>();
        if (_text.empty())
            element->Initialize(_label);
        else
            element->Initialize(_label, _text);

        // UISelectableを継承している場合はNavigatorに登録
        if constexpr (std::is_base_of_v<UISelectable, T>)
        {
            navigator_.RegisterSelectable(element.get());
        }

        uiElements_.push_back(element);
        return static_cast<std::shared_ptr<T>>(element);
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

    std::shared_ptr<UISlider> CreateSliderInternal(const std::string& _label, float _minV, float _maxV);

    static void SetupVerticalNavigation(const std::vector<UISelectable*>& _elements);
    static void SetupHorizontalNavigation(const std::vector<UISelectable*>& _elements);
    static void SetupGridNavigation(const std::vector<std::vector<UISelectable*>>& _elements);

private:

    UINavigator navigator_; // UIナビゲーター

    std::vector<std::shared_ptr<UIBase>> uiElements_; // UI要素のコンテナ
};