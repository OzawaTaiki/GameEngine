#include "UIBase.h"

#include <System/Input/Input.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Debug/ImGuiManager.h>
#include <Debug/ImGuiDebugManager.h>
#include <Features/Json/JsonHub.h>

void UIBase::Initialize(const std::string& _label)
{
    label_ = _label;

    jsonBinder_ = std::make_unique<JsonBinder>(_label, "Resources/Data/UI/");
    JsonHub::GetInstance()->SetDirectoryPathFromRoot("Resources/Data/UI/");

    jsonBinder_->RegisterVariable(label_+"_pos", &position_);
    jsonBinder_->RegisterVariable(label_+"_size", &size_);
    jsonBinder_->RegisterVariable(label_ + "_rotate", &rotate_);
    jsonBinder_->RegisterVariable(label_+"_anchor", &anchor_);
    jsonBinder_->RegisterVariable(label_+"_isActive", reinterpret_cast<uint32_t*>(&isActive_));
    jsonBinder_->RegisterVariable(label_+"_isVisible", reinterpret_cast<uint32_t*>(&isVisible_));
    jsonBinder_->RegisterVariable(label_ + "_color", &color_);
    jsonBinder_->RegisterVariable(label_+"_textureName", &textureName_);
    jsonBinder_->RegisterVariable(label_ + "_directoryPath", &directoryPath_);
    jsonBinder_->RegisterVariable(label_+"_label", &label_);

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

    ImGuiDebugManager::GetInstance()->AddDebugWindow(_label, [&]() {ImGui(); });
}

void UIBase::Draw()
{
    if (!isVisible_)
    {
        return;
    }
    sprite_->translate_ = position_;
    sprite_->SetSize(size_);
    sprite_->rotate_ = rotate_;
    sprite_->SetAnchor(anchor_);
    sprite_->SetTextureHandle(textureHandle_);
    sprite_->Draw(color_);
}

bool UIBase::IsMousePointerInside() const
{
    Vector2 mPos = Input::GetInstance()->GetMousePosition();

    return IsPointInside(mPos);
}

bool UIBase::IsPointInside(const Vector2& _point) const
{
    // アンカーを考慮した四頂点の計算
    Vector2 leftTop = position_ - size_ * anchor_;
    Vector2 rightBottom = position_ + size_ * (Vector2{ 1,1 } - anchor_);
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

#ifdef _DEBUG
void UIBase::ImGui()
{
    ImGui::BeginTabBar("UI");
    if (ImGui::BeginTabItem(label_.c_str()))
    {
        ImGui::DragFloat2("position", &position_.x, 1.0f);
        ImGui::DragFloat2("size", &size_.x, 1.0f);
        ImGui::DragFloat("rotate", &rotate_, 0.01f);
        ImGui::DragFloat2("anchor", &anchor_.x, 0.01f);
        ImGui::Checkbox("isActive", &isActive_);
        ImGui::Checkbox("isVisible", &isVisible_);
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
        ImGui::EndTabItem();

        if (ImGui::Button("Save"))
        {
            jsonBinder_->Save();
        }
    }
    ImGui::EndTabBar();

}

#endif // _DEBUG

