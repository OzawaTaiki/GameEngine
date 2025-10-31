#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector4.h>

#include <Features/Sprite/Sprite.h>
#include <Features/TextRenderer/TextGenerator.h>

#include <Features/Json/JsonBinder.h>

#include <vector>
#include <memory>


// 描画 トランスフォーム ラベルを持った基底クラス
class  UIBase
{
public:

    UIBase();
    virtual ~UIBase();

    // 初期化
    virtual void Initialize(const std::string& _label, bool _regsterDebugWindow);
    // 更新
    virtual void Update();
    // 描画
    virtual void Draw();

    // 親子関連
    void AddChild(std::shared_ptr<UIBase> _child);
    void RemoveChild(std::shared_ptr<UIBase> _child);
    UIBase* GetParent() const { return parent_; }
    const std::vector<std::shared_ptr<UIBase>>& GetChildren() const { return children_; }

    // Transform関連
    const Vector2& GetPos() const { return position_; }
    void SetPos(const Vector2& _pos) { position_ = _pos; };
    Vector2 GetWorldPos() const;

    Vector2 GetLeftTopPos() const { return GetWorldPos() - size_ * anchor_; }
    Vector2 GetRightBottomPos() const { return GetWorldPos() + size_ - size_ * anchor_; }
    Vector2 GetCenterPos() const { return GetWorldPos() + size_ * (Vector2{ 0.5f,0.5f } - anchor_); }

    const Vector2& GetSize() const { return size_; }
    void SetSize(const Vector2& _size) { size_ = _size; };

    void SetRotate(float _rotate) { rotate_ = _rotate; }
    float GetRotate() const { return rotate_; }

    void SetAnchor(const Vector2& _anchor) { anchor_ = _anchor; };
    const Vector2& GetAnchor() const { return anchor_; }

    uint16_t GetOrder() const { return sprite_->GetOrder(); }
    void SetOrder(int16_t _order) { sprite_->SetOrder(_order); }

    // 判定
    bool IsPointInside(const Vector2& _point) const;

    // 状態関連
    bool IsActive() const { return isActive_; }
    void SetActive(bool _isActive) { isActive_ = _isActive; }

    bool IsVisible() const { return isVisible_; }
    void SetVisible(bool _isVisible) { isVisible_ = _isVisible; }

    //bool IsMousePointerInside() const;
    //bool IsPointInside(const Vector2& _point) const;

    // 描画関連
    Vector4 GetColor() const { return color_; }
    virtual void SetColor(const Vector4& _color) { color_ = _color; }

    uint32_t GetTextureHandle() const { return textureHandle_; }
    void SetTextureHandle(uint32_t _textureHandle) { textureHandle_ = _textureHandle; }
    void SetTextureName(const std::string& _textureName) { textureName_ = _textureName; }
    void SetTextureNameAndLoad(const std::string& _textureName);

    // UV変換
    void SetUVTranslate(const Vector2& _uvTranslate) { sprite_->uvTranslate_ = _uvTranslate; }
    void SetUVScale(const Vector2& _uvScale) { sprite_->uvScale_ = _uvScale; }
    void SetUVRotate(float _uvRotate) { sprite_->uvRotate_ = _uvRotate; }


    const std::string& GetLabel() const { return label_; }

    virtual void Save();
    void ImGui();

protected:

    virtual void UpdateSelf(); // 自身の更新処理

    virtual void DrawSelf(); // 自身の描画処理

    void RemoveAllChildren();

    virtual void ImGuiContent() {}

protected:
    std::unique_ptr <Sprite> sprite_ = nullptr;

    // Transform
    Vector2 position_   = { 0,0 };
    Vector2 size_       = { 0,0 };
    float   rotate_     =   0.0f;
    Vector2 anchor_     = { 0,0 };

    // 状態
    bool isActive_ = true;
    bool isVisible_ = true;

    // 描画
    uint32_t textureHandle_ = 0;
    Vector4 color_ = { 1,1,1,1 };

    std::string textureName_ = "";
    std::string directoryPath_ = "Resources/images/";


    // 基本情報
    std::string label_ = "";
    std::unique_ptr<JsonBinder> jsonBinder_ = nullptr;

    // 親子
    UIBase* parent_ = nullptr; // 親UI要素
    std::vector<std::shared_ptr<UIBase>> children_; // 子UI要素
};
