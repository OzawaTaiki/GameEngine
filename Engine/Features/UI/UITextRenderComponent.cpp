#include "UITextRenderComponent.h"

#include <Features/UI/UIElement.h>
#include <Debug/ImGuiDebugManager.h>

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
    owner_->RegisterVariable("text", &text_);
    owner_->RegisterVariable("fontConfig", &fontConfig_);
    owner_->RegisterVariable("textParam", &textParam_);
    owner_->RegisterVariable("clipRect", &clipRect_);
    owner_->RegisterVariable("hasRect", &hasRect_);

    hasRect_ = false;
}

void UITextRenderComponent::Update()
{
}

void UITextRenderComponent::Draw()
{
    TextParam param = textParam_;
    // ワールド座標 = オーナーのワールド座標 + ローカルオフセット
    param.position = owner_->GetWorldPosition() + textParam_.position;

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
#ifdef _DEBUG

    char buffer[256];

    ImGui::PushID(this);
    if (ImGui::TreeNode("UITextRenderComponent"))
    {
        // テキスト編集
        strcpy_s(buffer, text_.c_str());
        if(ImGui::InputText("Text", buffer, sizeof(buffer)))
        {
            text_ = std::string(buffer);
        }

        // フォント情報
        ImGui::SeparatorText("Font");
        ImGui::Text("Path: %s", fontConfig_.fontFilePath.c_str());
        ImGui::Text("Size: %d", fontConfig_.fontSize);

        ImGui::Separator();
        ImGui::Text("Rect");
        if (hasRect_)
        {
            ImGui::DragFloat2("Left Top", &clipRect_.leftTop.x, 1.0f);
            ImGui::DragFloat2("Size", &clipRect_.size.x, 1.0f);
        }
        else
        {
            if(ImGui::Button("Add Clip Rect"))
            {
                hasRect_ = true;
                clipRect_ = Rect{ Vector2(0.0f,0.0f), owner_->GetSize() };
            }
        }

        // TextParam編集
        ImGui::SeparatorText("Appearance");

        textParam_.ImGui();

        ImGui::TreePop();
    }
    ImGui::PopID();
#endif
}

void UITextRenderComponent::SetClipRect(const Rect& rect)
{
    clipRect_ = rect;
    hasRect_ = true;
}

