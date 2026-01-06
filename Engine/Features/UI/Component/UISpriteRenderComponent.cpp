#include "UISpriteRenderComponent.h"
#include <Features/UI/Element/UIElement.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Debug/ImGuiDebugManager.h>

UISpriteRenderComponent::UISpriteRenderComponent(UIElement* owner, const std::string& texturePath):
    UIComponent(),
    texturePath_(texturePath)
{
    SetOwner(owner);
}

UISpriteRenderComponent::~UISpriteRenderComponent()
{
}


void UISpriteRenderComponent::Initialize()
{
    owner_->RegisterVariable("texturePath", &texturePath_);
    owner_->RegisterVariable("color", &color_);

    textureHandle_ = TextureManager::GetInstance()->Load(texturePath_);

    sprite_ = Sprite::Create("UISpriteRenderComponent_Sprite", textureHandle_, Vector2(0.5f, 0.5f));
    sprite_->SetColor(color_);
}

void UISpriteRenderComponent::Update()
{
    // UIElementの位置・サイズと同期
    sprite_->translate_ = owner_->GetWorldPosition();
    sprite_->SetSize(owner_->GetSize());

    sprite_->SetOrder(owner_->GetOrder());
    sprite_->SetColor(color_);
    sprite_->Update();
}

void UISpriteRenderComponent::Draw()
{
    sprite_->Draw();
}

void UISpriteRenderComponent::DrawImGui()
{
#ifdef _DEBUG

    ImGui::PushID(this);
    char buffer[256];

    if (ImGui::TreeNode("UISpriteRenderComponent"))
    {
        ImGui::Text("directory: %s", "Resources/images/");
        strcpy_s(buffer, texturePath_.c_str());
        if(ImGui::InputText("Texture Path: %s", buffer, sizeof(buffer)))
        {
            texturePath_ = std::string(buffer);
        }
        if (ImGui::Button("Load Texture"))
        {
            LoadAndSetTexture(texturePath_);
        }
        ImGui::ColorEdit4("Color", &color_.x);

        ImGui::TreePop();
    }
    ImGui::PopID();
#endif
}

void UISpriteRenderComponent::SetTexture(uint32_t textureHandle)
{
    textureHandle_ = textureHandle;
    sprite_->SetTextureHandle(textureHandle_);
}

void UISpriteRenderComponent::LoadAndSetTexture(const std::string& filePath)
{
    textureHandle_ = TextureManager::GetInstance()->Load(filePath);
    sprite_->SetTextureHandle(textureHandle_);
}
