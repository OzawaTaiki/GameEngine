#include "UIEditableComponent.h"
#include <Debug/Debug.h>
#include <Features/UI/Element/UIElement.h>

namespace Engine {

UIEditableComponent::UIEditableComponent():
    isEditing_(false),
    wasEditing_(false),
    onEditStart_(nullptr),
    onEditEnd_(nullptr),
    onEditingInput_(nullptr)
{
}

void UIEditableComponent::Update()
{
    // 編集状態の変化を検出
    if (isEditing_ && !wasEditing_)
    {
        // 編集開始時の処理
        if (onEditStart_)
        {
            onEditStart_();
        }
    }
    else if (!isEditing_ && wasEditing_)
    {
        // 編集終了時の処理
        if (onEditEnd_)
        {
            onEditEnd_();
        }
    }

    // 前回の編集状態を更新
    wasEditing_ = isEditing_;
}

void UIEditableComponent::HandleKeyInput(Input* input)
{
    if (isEditing_ && onEditingInput_)
    {
        onEditingInput_(input);
    }
}

} // namespace Engine
