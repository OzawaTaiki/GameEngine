#pragma once
#include <Math/Vector/Vector2.h>
#include <Features/UI/UIBase.h>

/// <summary>
/// UIコライダーの種類
/// </summary>
enum class ColliderType
{
    Rectangle,      // 長方形
    Circle,         // 円形
    Ellipse,        // 楕円形
    Parallelogram,  // 平行四辺形

    Max
};

/// <summary>
/// UIコライダーの抽象基底クラス
/// UI要素の衝突判定を行うためのインターフェース
/// 各形状ごとに具体的な実装クラスを作成する
/// </summary>
class IUICollider
{
public:
    virtual ~IUICollider() = default;

    /// <summary>
    /// 点がコライダー内にあるかどうかを判定する
    /// </summary>
    /// <param name="_point">判定する点の座標（ワールド座標）</param>
    /// <returns>点がコライダー内にある場合はtrue、そうでない場合はfalse</returns>
    virtual bool IsPointInside(const Vector2& _point) const = 0;

    /// <summary>
    /// UIBaseの情報からコライダーのキャッシュを更新する
    /// Transform変更時に呼び出されることを想定
    /// ワールド座標での頂点や中心位置などを事前計算する
    /// </summary>
    /// <param name="_uiBase">親となるUIBase</param>
    virtual void UpdateCache(const UIBase* _uiBase) = 0;

    /// <summary>
    /// ImGuiでコライダーのパラメータを表示・編集する
    /// デバッグ用の関数
    /// </summary>
    virtual void ImGui() {};

    /// <summary>
    /// コライダーの形状をデバッグ描画する
    /// </summary>
    virtual void DrawDebug() const {};
};