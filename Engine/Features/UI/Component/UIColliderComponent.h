#pragma once
#include <Features/UI/Component/UIComponent.h>
#include <Features/UI/Collider/Interface/IUICollider.h>
#include <Features/UI/Collider/UIColliderFactory.h>
#include <memory>

/// <summary>
/// Collider機能を提供するComponent
/// 既存のIUIColliderをラップして使用
/// </summary>
class UIColliderComponent : public UIComponent
{
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="type">Colliderの種類</param>
    UIColliderComponent(ColliderType type = ColliderType::Rectangle);
    ~UIColliderComponent() override;

    void Initialize() override;
    void Update() override;
    void DrawImGui() override;

    /// <summary>
    /// 点が内側にあるかを判定
    /// </summary>
    bool IsHit(const Vector2& worldPos) const;

    /// <summary>
    /// サイズ設定（Rectangle用）
    /// </summary>
    void SetSize(const Vector2& size);
    Vector2 GetSize() const;

    /// <summary>
    /// オフセット設定
    /// </summary>
    void SetOffset(const Vector2& offset);
    Vector2 GetOffset() const;

    /// <summary>
    /// Collider種類を変更
    /// </summary>
    void SetColliderType(ColliderType type);
    ColliderType GetColliderType() const { return colliderType_; }

    /// <summary>
    /// デバッグ描画の有効/無効
    /// </summary>
    void SetDebugDraw(bool enable) { debugDraw_ = enable; }
    bool IsDebugDraw() const { return debugDraw_; }

    /// <summary>
    /// 既存Colliderへの直接アクセス（高度な設定用）
    /// </summary>
    IUICollider* GetCollider() { return collider_.get(); }
    const IUICollider* GetCollider() const { return collider_.get(); }
    void SetCollider(std::unique_ptr<IUICollider> collider);


    void Save() override;

private:
    std::unique_ptr<IUICollider> collider_;
    ColliderType colliderType_;
    bool debugDraw_ = true;

    /// <summary>
    /// Colliderのサイズを自動設定
    /// </summary>
    void InitializeColliderSize();
};
