#include "UITextRenderComponent.h"

#include <Features/UI/UIElement.h>

UITextRenderComponent::UITextRenderComponent(UIElement* owner, const std::string& text, const FontConfig& fontConfig):
    UIComponent(),
    text_(text),
    textParam_(TextParam()),
    fontConfig_(fontConfig)
{
    SetOwner(owner);
}

UITextRenderComponent::~UITextRenderComponent()
{
}

void UITextRenderComponent::Initialize()
{
    hasRect_ = false;
}

void UITextRenderComponent::Update()
{
}

void UITextRenderComponent::Draw()
{
    TextParam param = textParam_;
    param.position += owner_->GetWorldPosition();

    if (hasRect_)
    {
        TextGenerator::Draw(text_, fontConfig_, clipRect_, param, owner_->GetOrder());
    }
    else
    {
        TextGenerator::Draw(text_, fontConfig_, param, owner_->GetOrder());
    }

}

void UITextRenderComponent::DrawImGui()
{
}

void UITextRenderComponent::SetClipRect(const Rect& rect)
{
    clipRect_ = rect;
    hasRect_ = true;
}

