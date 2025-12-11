#pragma once

#include "Interface/IUICollider.h"

/// <summary>
/// 楕円形UIコライダー
/// 楕円の方程式を用いて衝突判定を行う
/// UI依存モード：UIのサイズから自動的にX軸とY軸の半径を計算する
/// 独立モード：独自に設定された中心と半径を使用
/// </summary>
class UIEllipseCollider : public IUICollider
{
public:
    UIEllipseCollider() = default;
    ~UIEllipseCollider() override = default;

    /// <summary>
    /// 点が楕円の内側にあるかを判定
    /// 楕円の方程式 (x/a)² + (y/b)² <= 1 を使用
    /// </summary>
    /// <param name="_point">判定する点の座標</param>
    /// <returns>楕円の内側ならtrue</returns>
    bool IsPointInside(const Vector2& _point) const override;

    /// <summary>
    /// UIElementの情報から楕円の中心と半径をキャッシュする
    /// UI依存モード：X軸の半径 = width / 2、Y軸の半径 = height / 2
    /// 独立モード：独自パラメータを使用（UIElementは無視）
    /// </summary>
    /// <param name="_uiElement">親となるUIElement</param>
    void UpdateCache(const UIElement* _uiElement) override;

    /// <summary>
    /// ImGuiでパラメータを表示・編集する
    /// </summary>
    void ImGui() override;

    /// <summary>
    /// 独立モード用：楕円の中心オフセットを設定（UIローカル座標系）
    /// </summary>
    /// <param name="_offset">UI中心からのオフセット</param>
    void SetLocalOffset(const Vector2& _offset) { localOffset_ = _offset; }

    /// <summary>
    /// 独立モード用：楕円の半径を設定
    /// </summary>
    /// <param name="_radius">楕円のX軸とY軸の半径</param>
    void SetRadius(const Vector2& _radius) { independentRadius_ = _radius; }

    /// <summary>
    /// 独立モード用：楕円の中心オフセットを取得
    /// </summary>
    /// <returns>UI中心からのオフセット</returns>
    Vector2 GetLocalOffset() const { return localOffset_; }

    /// <summary>
    /// 独立モード用：楕円の半径を取得
    /// </summary>
    /// <returns>楕円のX軸とY軸の半径</returns>
    Vector2 GetIndependentRadius() const { return independentRadius_; }

private:
    // 実際の判定に使用するキャッシュ
    Vector2 center_ = { 0, 0 };         // 楕円の中心（ワールド座標）
    Vector2 radius_ = { 0.0f, 0.0f };   // X軸とY軸の半径

    // 独立モード用パラメータ（UIローカル座標系）
    Vector2 localOffset_ = { 0, 0 };            // UI中心からのオフセット
    Vector2 independentRadius_ = { 100, 50 };   // 独立モードでのX軸とY軸の半径
};