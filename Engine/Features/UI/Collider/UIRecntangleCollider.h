#pragma once
#include "Interface/IUICollider.h"

/// <summary>
/// 矩形（長方形）UIコライダー
/// 回転なしの軸平行境界ボックス（AABB）として衝突判定を行う
/// </summary>
class UIRectangleCollider : public IUICollider
{
public:
    UIRectangleCollider() = default;
    ~UIRectangleCollider() override = default;

    bool IsPointInside(const Vector2& _point) const override;
    void UpdateCache(const UIElement* _uiElement) override;
    void ImGui() override;

    void DrawDebug() const override;

    // 独立モード用
    void SetLocalSize(const Vector2& _size) { localSize_ = _size; }
    void SetLocalOffset(const Vector2& _offset) { localOffset_ = _offset; }
    Vector2 GetLocalSize() const { return localSize_; }
    Vector2 GetLocalOffset() const { return localOffset_; }

private:
    // キャッシュ
    Vector2 leftTop_ = { 0, 0 };
    Vector2 rightBottom_ = { 0, 0 };

    // 独立モード用
    Vector2 localOffset_ = { 0, 0 };
    Vector2 localSize_ = { 100, 100 };
};