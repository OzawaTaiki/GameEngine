#pragma once
#include <Math/Vector/Vector2.h>

// 前方宣言
class UIElement;  // UIBase → UIElement

/// <summary>
/// UIコライダーの種類
/// </summary>
enum class ColliderType
{
    Rectangle,      // 長方形
    Circle,         // 円形
    Ellipse,        // 楕円形
    // Parallelogram,  // 平行四辺形（回転必要なので削除）
    Quad,           // 四角形（4頂点自由設定）
    ConvexPolygon,  // 凸多角形

    Max
};

/// <summary>
/// UIコライダーの抽象基底クラス
/// </summary>
class IUICollider
{
public:
    /// <summary>
    /// トランスフォームモード
    /// UI依存：UIElementのパラメータ（位置、サイズ）を使用
    /// 独立：コライダー独自のパラメータを使用
    /// </summary>
    enum class TransformMode
    {
        UIDependent,    // UI依存モード
        Independent     // 独立モード
    };

    virtual ~IUICollider() = default;

    /// <summary>
    /// 点がコライダー内にあるかどうかを判定する
    /// </summary>
    virtual bool IsPointInside(const Vector2& _point) const = 0;

    /// <summary>
    /// UIElementの情報からコライダーのキャッシュを更新する
    /// </summary>
    /// <param name="_uiElement">親となるUIElement</param>
    virtual void UpdateCache(const UIElement* _uiElement) = 0;

    /// <summary>
    /// ImGuiでコライダーのパラメータを表示・編集する
    /// </summary>
    virtual void ImGui() {};

    /// <summary>
    /// コライダーの形状をデバッグ描画する
    /// </summary>
    virtual void DrawDebug() const {};

    void SetTransformMode(TransformMode _mode) { transformMode_ = _mode; }
    TransformMode GetTransformMode() const { return transformMode_; }

    bool GetIsHit() const { return isHit_; }
    void SetHit(bool _isHit) { isHit_ = _isHit; }

protected:
    TransformMode transformMode_ = TransformMode::UIDependent;
    bool isHit_ = false;
};