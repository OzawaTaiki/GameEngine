#include "UIBase.h"

#include <System/Input/Input.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Debug/ImGuiManager.h>
#include <Debug/ImGuiDebugManager.h>


namespace Engine {

namespace
{
Vector2 defaultPosition = { 100.0f, 100.0f };
Vector2 defaultSize     = { 100.0f, 100.0f };
}

UIBase::UIBase()
{
}

UIBase::~UIBase()
{
#ifdef _DEBUG
    ImGuiDebugManager::GetInstance()->RemoveDebugWindow("UI_" + label_);
#endif // _DEBUG
    RemoveAllChildren();
}

void UIBase::Initialize(const std::string& _label, [[maybe_unused]] bool _regsterDebugWindow)
{
#ifdef _DEBUG
    if (_regsterDebugWindow)
        ImGuiDebugManager::GetInstance()->AddDebugWindow("UI_" + _label, [this]() { this->ImGui(); });
#endif // _DEBUG

    label_ = _label;

    jsonBinder_ = std::make_unique<JsonBinder>(_label, "Resources/Data/UI/");

    color_ = { 1.0f,1.0f ,1.0f ,1.0f };
    position_ = defaultPosition;
    size_ = defaultSize;

    jsonBinder_->RegisterVariable(label_ + "_pos", &position_);
    jsonBinder_->RegisterVariable(label_ + "_size", &size_);
    jsonBinder_->RegisterVariable(label_ + "_rotate", &rotate_);
    jsonBinder_->RegisterVariable(label_ + "_anchor", &anchor_);
    jsonBinder_->RegisterVariable(label_ + "_isActive", reinterpret_cast<uint32_t*>(&isActive_));
    jsonBinder_->RegisterVariable(label_ + "_isVisible", reinterpret_cast<uint32_t*>(&isVisible_));
    jsonBinder_->RegisterVariable(label_ + "_color", &color_);
    jsonBinder_->RegisterVariable(label_ + "_textureName", &textureName_);
    jsonBinder_->RegisterVariable(label_ + "_directoryPath", &directoryPath_);
    jsonBinder_->RegisterVariable(label_ + "_label", &label_);

    int16_t order = 0;
    jsonBinder_->GetVariableValue(label_ + "_order", order);

    if (textureName_ == "")
        textureName_ = "white.png";

    textureHandle_ = TextureManager::GetInstance()->Load(textureName_, directoryPath_);
    sprite_ = Sprite::Create(_label, textureHandle_);
    sprite_->Initialize();
    sprite_->translate_ = position_;

    if (size_.x == 0 && size_.y == 0)
        size_ = TextureManager::GetInstance()->GetTextureSize(textureHandle_);

    sprite_->SetSize(size_);
    sprite_->rotate_ = rotate_;
    sprite_->SetAnchor(anchor_);
    sprite_->SetOrder(order);
}

void UIBase::Update()
{
    if (!isActive_)
    {
        // 非アクティブなら更新しない
        return;
    }

    // 自身の更新処理
    UpdateSelf();

    // 子要素の更新
    for (auto& child : children_)
    {
        if (child)
            child->Update();
    }

}

void UIBase::Draw()
{
    if (!isVisible_)
    {
        return;
    }

    DrawSelf();

    for (auto& child : children_)
    {
        if (child && child->IsVisible())
        {
            child->Draw();
        }
    }
}

void UIBase::DrawSelf()
{
    if (!sprite_)
        return;

    sprite_->translate_ = GetWorldPos();
    sprite_->SetSize(size_);
    sprite_->rotate_ = rotate_;
    sprite_->SetAnchor(anchor_);
    sprite_->SetTextureHandle(textureHandle_);

    // orderの設定: 自身がUINT16_MAXなら親のorderを使用、親がいない場合は0
    int16_t currentOrder = sprite_->GetOrder();
    if (currentOrder == 0)
    {
        if (parent_)
        {
            sprite_->SetOrder(parent_->GetOrder());
        }
    }

    sprite_->Draw(color_);
}

void UIBase::AddChild(std::shared_ptr<UIBase> _child)
{
    if (!_child)
        return;

    if (_child->parent_)
    {
        // すでに親がいる場合は外す
        _child->parent_->RemoveChild(_child);
    }

    // 親子関係を設定
    _child->parent_ = this;
    children_.push_back(_child);
}

void UIBase::RemoveChild(std::shared_ptr<UIBase> _child)
{
    if (!_child)
        return;

    auto it = std::find(children_.begin(), children_.end(), _child);
    if (it != children_.end())
    {
        (*it)->parent_ = nullptr; // 親を解除
        children_.erase(it);
    }
}

void UIBase::RemoveAllChildren()
{
    for (auto& child : children_)
    {
        if (child)
            child->parent_ = nullptr;
    }
    children_.clear();
}

Vector2 UIBase::GetWorldPos() const
{
    Vector2 pos = position_;
    if (parent_)
        pos +=parent_->GetWorldPos();

    return pos;
}

bool UIBase::IsPointInside(const Vector2& _point) const
{
    // アンカーを考慮した四頂点の計算
    Vector2 leftTop = GetWorldPos() - size_ * anchor_;
    Vector2 rightBottom = GetWorldPos() + size_ * (Vector2{ 1,1 } - anchor_);
    Vector2 rightTop = { rightBottom.x, leftTop.y };
    Vector2 leftBottom = { leftTop.x, rightBottom.y };

    // 四角形の内側にいるかどうか
    if (_point.x > leftTop.x && _point.x < rightBottom.x &&
        _point.y > leftTop.y && _point.y < rightBottom.y)
    {
        return true;
    }

    return false;
}

void UIBase::SetTextureNameAndLoad(const std::string& _textureName)
{
    textureName_ = _textureName;
    textureHandle_ = TextureManager::GetInstance()->Load(textureName_);
    sprite_->SetTextureHandle(textureHandle_);
}

void UIBase::Save()
{
    if (jsonBinder_)
    {
        jsonBinder_->SendVariable(label_ + "_order", sprite_->GetOrder());
        jsonBinder_->Save();
    }
}

void UIBase::ImGui()
{
#ifdef _DEBUG

    ImGui::PushID(this);
    // UIBase独自の項目
    if (ImGui::CollapsingHeader(label_.c_str()))
    {
        if (ImGui::TreeNode("UIBase"))
        {
            ImGui::DragFloat2("position", &position_.x, 1.0f);
            ImGui::DragFloat2("size", &size_.x, 1.0f);
            ImGui::DragFloat("rotate", &rotate_, 0.01f);
            ImGui::DragFloat2("anchor", &anchor_.x, 0.01f);
            ImGui::Checkbox("isActive", &isActive_);
            ImGui::Checkbox("isVisible", &isVisible_);
            int order = sprite_->GetOrder();
            if (ImGui::InputInt("order", &order))
                sprite_->SetOrder(static_cast<int16_t>(order));

            ImGui::ColorEdit4("color", &color_.x);

            char buf1[255];
            strcpy_s(buf1, directoryPath_.c_str());
            if (ImGui::InputText("directoryPath", buf1, 255))
            {
                directoryPath_ = buf1;
            }

            char buf2[255];
            strcpy_s(buf2, textureName_.c_str());   
            if (ImGui::InputText("textureName", buf2, 255))
            {
                textureName_ = buf2;
            }

            if (ImGui::Button("Apply"))
            {
                // さいごに"/"がついていない場合はつける
                if (directoryPath_.back() != '/')
                {
                    directoryPath_ += "/";
                }

                //ConfigManager::GetInstance()->SaveData("UI");
                textureHandle_ = TextureManager::GetInstance()->Load(textureName_, directoryPath_);
                sprite_->SetTextureHandle(textureHandle_);
            }

            if (ImGui::Button("Save"))
            {
                Save();
            }

            ImGui::TreePop();
        }

        // 継承先のImGui項目
        ImGuiContent();
    }

    // 子要素のImGui
    if (!children_.empty() && ImGui::CollapsingHeader("Children"))
    {
        for (size_t i = 0; i < children_.size(); ++i)
        {
            if (children_[i])
            {
                children_[i]->ImGui();
            }
        }
    }

    ImGui::PopID();

#endif // _DEBUG
}

void UIBase::UpdateSelf()
{
    sprite_->uvTranslate_ = uvTransform_.GetOffset();
    sprite_->uvScale_ = uvTransform_.GetScale();
    sprite_->uvRotate_ = uvTransform_.GetRotation();
}

} // namespace Engine
