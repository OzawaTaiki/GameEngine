#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector4.h>

#include <Features/Sprite/Sprite.h>

#include <Features/Json/JsonBinder.h>

class  UIBase
{
public:

    UIBase() = default;
    virtual ~UIBase() = default;

    virtual void Initialize(const std::string& _label);
    virtual void Update() {};
    virtual void Draw();

    bool IsActive() const { return isActive_; }
    void SetActive(bool _isActive) { isActive_ = _isActive; }

    bool IsVisible() const { return isVisible_; }
    void SetVisible(bool _isVisible) { isVisible_ = _isVisible; }
    void SetColor(const Vector4& _color) { color_ = _color; }
    bool IsMousePointerInside() const;
    bool IsPointInside(const Vector2& _point) const;

    void SetPos(const Vector2& _pos) { position_ = _pos; };
    void SetSize(const Vector2& _size) { size_ = _size; };
    void SetAnchor(const Vector2& _anchor) { anchor_ = _anchor; };


    void SetTextureHandle(uint32_t _textureHandle) { textureHandle_ = _textureHandle;  }
    void SetTextureName(const std::string& _textureName) { textureName_ = _textureName; }
    void SetTextureNameAndLoad(const std::string& _textureName);


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

    void ImGui();

};
