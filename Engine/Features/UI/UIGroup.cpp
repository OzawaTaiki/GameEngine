#include "UIGroup.h"

#include <Features/UI/UISprite.h>
#include <Features/UI/UIButton.h>
#include <Features/UI/UISlider.h>
#include <algorithm>

void UIGroup::Initialize()
{
    // ナビゲーターの初期化
    navigator_.Initialize(nullptr); // 初期状態ではフォーカスを持つ要素はなし

    // UI要素の初期化
    uiElements_.clear();
}

void UIGroup::Update()
{
    for (const auto& element : uiElements_)
    {
        if (element && element->IsActive())
        {
            element->Update();
        }
    }

    navigator_.Update();
}

void UIGroup::Draw()
{
    for (const auto& element : uiElements_)
    {
        if (element && element->IsVisible())
        {
            element->Draw();
        }
    }
}

UIButton* UIGroup::CreateButton(const std::string& _label, const std::wstring& _text)
{
    return CreateElement<UIButton>(_label, _text);
}

UISprite* UIGroup::CreateSprite(const std::string& _label, const std::wstring& _text)
{
    return CreateElement<UISprite>(_label, _text);
}

UISlider* UIGroup::CreateSlider(const std::string& _label, const std::wstring& _text)
{
    return CreateElement<UISlider>(_label, _text);
}

void UIGroup::AddElement(std::unique_ptr<UIBase> _element)
{
    if (!_element)
        return;

    // UISelectableを継承している場合はNavigatorに登録
    if (UISelectable* selectable = dynamic_cast<UISelectable*>(_element.get()))
    {
        navigator_.RegisterSelectable(selectable);
    }

    uiElements_.push_back(std::move(_element));
}

void UIGroup::RemoveElement(UIBase* _element)
{
    if (!_element)
        return;

    auto it = std::find_if(uiElements_.begin(), uiElements_.end(),
        [_element](const std::unique_ptr<UIBase>& element) {
            return element.get() == _element;
        });

    if (it != uiElements_.end())
    {
        // UISelectableを継承している場合はNavigatorから削除
        if (UISelectable* selectable = dynamic_cast<UISelectable*>(_element))
        {
            navigator_.UnregisterSelectable(selectable);
        }

        uiElements_.erase(it);
    }
}

void UIGroup::LinkVertical(std::initializer_list<UISelectable*> _elements)
{
    std::vector<UISelectable*> elementList;
    for (auto element : _elements)
    {
        if (element)
        {
            elementList.push_back(element);
        }
    }
    SetupVerticalNavigation(elementList);
}

void UIGroup::LinkHorizontal(std::initializer_list<UISelectable*> _elements)
{
    std::vector<UISelectable*> elementList;
    for (auto element : _elements)
    {
        if (element)
        {
            elementList.push_back(element);
        }
    }
    SetupHorizontalNavigation(elementList);
}

void UIGroup::LinkGrid(std::initializer_list<std::initializer_list<UISelectable*>> _elements)
{
    std::vector<std::vector<UISelectable*>> elementGrid;
    for (const auto& row : _elements)
    {
        std::vector<UISelectable*> elementRow;
        for (auto element : row)
        {
            if (element)
            {
                elementRow.push_back(element);
            }
        }
        elementGrid.push_back(elementRow);
    }
    SetupGridNavigation(elementGrid);
}

void UIGroup::SetupVerticalNavigation(const std::vector<UISelectable*>& _elements)
{
    for (size_t i = 0; i < _elements.size(); ++i)
    {
        UISelectable* currentElement = _elements[i];
        if (!currentElement)
            continue;

        // 上の要素
        if (i > 0)
        {
            currentElement->SetNavigationTarget(_elements[i - 1], Direction::Up);
        }
        // 下の要素
        if (i < _elements.size() - 1)
        {
            currentElement->SetNavigationTarget(_elements[i + 1], Direction::Down);
        }
    }
}

void UIGroup::SetupHorizontalNavigation(const std::vector<UISelectable*>& _elements)
{
    for (size_t i = 0; i < _elements.size(); ++i)
    {
        UISelectable* currentElement = _elements[i];
        if (!currentElement)
            continue;

        // 左の要素
        if (i > 0)
        {
            currentElement->SetNavigationTarget(_elements[i - 1], Direction::Left);
        }
        // 右の要素
        if (i < _elements.size() - 1)
        {
            currentElement->SetNavigationTarget(_elements[i + 1], Direction::Right);
        }
    }
}

void UIGroup::SetupGridNavigation(const std::vector<std::vector<UISelectable*>>& _elements)
{
    for (size_t i = 0; i < _elements.size(); ++i)
    {
        for (size_t j = 0; j < _elements[i].size(); ++j)
        {
            UISelectable* currentElement = _elements[i][j];
            if (!currentElement)
                continue;

            // 上の要素
            if (i > 0 && j < _elements[i - 1].size())
            {
                currentElement->SetNavigationTarget(_elements[i - 1][j], Direction::Up);
            }
            // 下の要素
            if (i < _elements.size() - 1 && j < _elements[i + 1].size())
            {
                currentElement->SetNavigationTarget(_elements[i + 1][j], Direction::Down);
            }
            // 左の要素
            if (j > 0)
            {
                currentElement->SetNavigationTarget(_elements[i][j - 1], Direction::Left);
            }
            // 右の要素
            if (j < _elements[i].size() - 1)
            {
                currentElement->SetNavigationTarget(_elements[i][j + 1], Direction::Right);
            }
        }
    }
}