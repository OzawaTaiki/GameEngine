#include "UIGroup.h"

#include <Features/UI/UISprite.h>
#include <Features/UI/UIButton.h>

void UIGroup::Initialize()
{
    // ボタンナビゲーターの初期化
    buttonNavigator_.Initialize(nullptr); // 初期状態ではフォーカスを持つボタンはなし

    // UI要素の初期化
    uiElements_.clear();
}

void UIGroup::Update()
{
    for (const auto& element : uiElements_)
    {
        if (element)
        {
            element->Update();
        }
    }

    buttonNavigator_.Update();

}

void UIGroup::Draw()
{
    for (const auto& element : uiElements_)
    {
        if (element)
        {
            element->Draw();
        }
    }
}

UIButton* UIGroup::CreateButton(const std::string& _label, const std::wstring& _text)
{
    auto button = std::make_unique<UIButton>();
    if (_text.empty())
        button->Initialize(_label);
    else
        button->Initialize(_label, _text);
    buttonNavigator_.RegisterButton(button.get()); // ボタンをナビゲーターに登録

    uiElements_.push_back(std::move(button));

    return static_cast<UIButton*>(uiElements_.back().get());
}

UISprite* UIGroup::CreateSprite(const std::string& _label, const std::wstring& _text)
{
    auto sprite = std::make_unique<UISprite>();
    if (_text.empty())
        sprite->Initialize(_label);
    else
        sprite->Initialize(_label, _text);

    uiElements_.push_back(std::move(sprite));

    return static_cast<UISprite*>(uiElements_.back().get());
}

void UIGroup::LinkVertical(std::initializer_list<UIButton*> _buttons)
{
    std::vector<UIButton*> buttonList;
    for (auto button : _buttons)
    {
        if (button)
        {
            buttonList.push_back(button);
        }
    }
    SetupVerticalNavigation(buttonList);
}

void UIGroup::LinkHorizontal(std::initializer_list<UIButton*> _buttons)
{
    std::vector<UIButton*> buttonList;
    for (auto button : _buttons)
    {
        if (button)
        {
            buttonList.push_back(button);
        }
    }
    SetupHorizontalNavigation(buttonList);
}

void UIGroup::LinkGrid(std::initializer_list<std::initializer_list<UIButton*>> _buttons)
{
    std::vector<std::vector<UIButton*>> buttonGrid;
    for (const auto& row : _buttons)
    {
        std::vector<UIButton*> buttonRow;
        for (auto button : row)
        {
            if (button)
            {
                buttonRow.push_back(button);
            }
        }
        buttonGrid.push_back(buttonRow);
    }
    SetupGridNavigation(buttonGrid);
}

void UIGroup::SetupVerticalNavigation(const std::vector<UIButton*>& _buttons)
{
    for (size_t i = 0; i < _buttons.size(); ++i)
    {
        UIButton* currentButton = _buttons[i];
        if (!currentButton)
            continue;

        // 上のボタン
        if (i > 0)
        {
            currentButton->SetNavigationTarget(_buttons[i - 1], Direction::Up);
        }
        // 下のボタン
        if (i < _buttons.size() - 1)
        {
            currentButton->SetNavigationTarget(_buttons[i + 1], Direction::Down);
        }
    }
}

void UIGroup::SetupHorizontalNavigation(const std::vector<UIButton*>& _buttons)
{
    for (size_t i = 0; i < _buttons.size(); ++i)
    {
        UIButton* currentButton = _buttons[i];
        if (!currentButton)
            continue;

        // 左のボタン
        if (i > 0)
        {
            currentButton->SetNavigationTarget(_buttons[i - 1], Direction::Left);
        }
        // 右のボタン
        if (i < _buttons.size() - 1)
        {
            currentButton->SetNavigationTarget(_buttons[i + 1], Direction::Right);
        }
    }
}

void UIGroup::SetupGridNavigation(const std::vector<std::vector<UIButton*>>& _buttons)
{
    for (size_t i = 0; i < _buttons.size(); ++i)
    {
        for (size_t j = 0; j < _buttons[i].size(); ++j)
        {
            UIButton* currentButton = _buttons[i][j];
            if (!currentButton)
                continue;

            // 上のボタン
            if (i > 0 && j < _buttons[i - 1].size())
            {
                currentButton->SetNavigationTarget(_buttons[i - 1][j], Direction::Up);
            }
            // 下のボタン
            if (i < _buttons.size() - 1 && j < _buttons[i + 1].size())
            {
                currentButton->SetNavigationTarget(_buttons[i + 1][j], Direction::Down);
            }
            // 左のボタン
            if (j > 0)
            {
                currentButton->SetNavigationTarget(_buttons[i][j - 1], Direction::Left);
            }
            // 右のボタン
            if (j < _buttons[i].size() - 1)
            {
                currentButton->SetNavigationTarget(_buttons[i][j + 1], Direction::Right);
            }
        }
    }
}
