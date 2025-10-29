#pragma once

#include "Interface/IUICollider.h"

/// <summary>
/// 矩形（長方形）UIコライダー
/// 回転なしの軸平行境界ボックス（AABB）として衝突判定を行う
/// UI依存モード：UIの左上と右下座標から計算
/// 独立モード：独自に設定された左上と右下座標を使用
/// 最も基本的で高速なコライダー
/// </summary>
class UIRentangleCollider : public IUICollider
{
public:
    UIRentangleCollider() = default;
    ~UIRentangleCollider() override = default;

    /// <summary>
    /// 点が矩形の内側にあるかを判定
    /// </summary>
    /// <param name="_point">判定する点の座標</param>
    /// <returns>矩形の内側ならtrue</returns>
    bool IsPointInside(const Vector2& _point) const override;

    /// <summary>
    /// UIBaseの情報から矩形の左上と右下の座標をキャッシュする
    /// UI依存モード：UIの左上と右下座標を使用
    /// 独立モード：独自パラメータを使用（UIBaseは無視）
    /// </summary>
    /// <param name="_uiBase">親となるUIBase</param>
    void UpdateCache(const UIBase* _uiBase) override;

    /// <summary>
    /// ImGuiでパラメータを表示・編集する
    /// </summary>
    void ImGui() override;

    /// <summary>
    /// 独立モード用：矩形のサイズを設定（UIローカル座標系）
    /// </summary>
    /// <param name="_size">矩形のサイズ</param>
    void SetLocalSize(const Vector2& _size) { localSize_ = _size; }

    /// <summary>
    /// 独立モード用：矩形の中心オフセットを設定（UIローカル座標系）
    /// </summary>
    /// <param name="_offset">UI中心からのオフセット</param>
    void SetLocalOffset(const Vector2& _offset) { localOffset_ = _offset; }

    /// <summary>
    /// 独立モード用：矩形のサイズを取得
    /// </summary>
    /// <returns>矩形のサイズ</returns>
    Vector2 GetLocalSize() const { return localSize_; }

    /// <summary>
    /// 独立モード用：矩形の中心オフセットを取得
    /// </summary>
    /// <returns>UI中心からのオフセット</returns>
    Vector2 GetLocalOffset() const { return localOffset_; }

private:
    // 実際の判定に使用するキャッシュ
    Vector2 leftTop_ = { 0, 0 };      // 矩形の左上座標（ワールド座標）
    Vector2 rightBottom_ = { 0, 0 };  // 矩形の右下座標（ワールド座標）

    // 独立モード用パラメータ（UIローカル座標系）
    Vector2 localOffset_ = { 0, 0 };     // UI中心からのオフセット
    Vector2 localSize_ = { 100, 100 };   // 矩形のサイズ
};
