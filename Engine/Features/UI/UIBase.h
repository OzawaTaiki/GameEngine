#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector4.h>

#include <Features/Sprite/Sprite.h>

#include <Features/Json/JsonBinder.h>

class  UIBase
{
public:

    UIBase() = default;
    virtual ~UIBase();

    virtual void Initialize(const std::string& _label);
    virtual void Update() {};
    virtual void Draw();

    bool IsMousePointerInside() const;
    bool IsPointInside(const Vector2& _point) const;

    bool IsActive() const { return isActive_; }
    void SetActive(bool _isActive) { isActive_ = _isActive; }

    bool IsVisible() const { return isVisible_; }
    void SetVisible(bool _isVisible) { isVisible_ = _isVisible; }

    Vector4 GetColor() const { return color_; }
    void SetColor(const Vector4& _color) { color_ = _color; }

    const Vector2& GetPos() const { return position_; }
    void SetPos(const Vector2& _pos) { position_ = _pos; };

    const Vector2& GetSize() const { return size_; }
    void SetSize(const Vector2& _size) { size_ = _size; };

    void SetRotate(float _rotate) { rotate_ = _rotate; }
    float GetRotate() const { return rotate_; }
    void SetAnchor(const Vector2& _anchor) { anchor_ = _anchor; };

    void SetUVTranslate(const Vector2& _uvTranslate) { sprite_->uvTranslate_ = _uvTranslate; }
    void SetUVScale(const Vector2& _uvScale) { sprite_->uvScale_ = _uvScale; }
    void SetUVRotate(float _uvRotate) { sprite_->uvRotate_ = _uvRotate; }

    uint32_t GetTextureHandle() const { return textureHandle_; }
    void SetTextureHandle(uint32_t _textureHandle) { textureHandle_ = _textureHandle;  }

    const std::string& GetLabel() const { return label_; }
    void SetTextureName(const std::string& _textureName) { textureName_ = _textureName; }

    void SetTextureNameAndLoad(const std::string& _textureName);

    void Save();

    void ImGui();
protected:
    std::unique_ptr <Sprite> sprite_ = nullptr;

    Vector2 position_ = { 0,0 };
    Vector2 size_ = { 0,0 };

    float rotate_ = 0.0f;

    Vector2 anchor_ = { 0,0 };

    bool isActive_ = true;
    bool isVisible_ = true;

    uint32_t textureHandle_ = 0;

    Vector4 color_ = { 1,1,1,1 };

    std::string label_ = "";
    std::string textureName_ = "";
    std::string directoryPath_ = "Resources/images/";

    std::unique_ptr<JsonBinder> jsonBinder_ = nullptr;


};
