#include "UISpriteRenderComponent.h"
#include <Features/UI/UIElement.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>

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
}

void UISpriteRenderComponent::LoadAndSetTexture(const std::string& filePath)
{
    textureHandle_ = TextureManager::GetInstance()->Load(filePath);
    sprite_->SetTextureHandle(textureHandle_);
}
