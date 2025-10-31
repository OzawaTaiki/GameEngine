#pragma once
#include <Math/Vector/Vector2.h>

// 前方宣言 (循環参照防止)
class UIBase;

/// <summary>
/// UIコライダーの種類
/// </summary>
enum class ColliderType
{
    Rectangle,      // 長方形
    Circle,         // 円形
    Ellipse,        // 楕円形
    Parallelogram,  // 平行四辺形
    Quad,           // 四角形（4頂点自由設定）

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
    /// <summary>
    /// トランスフォームモード
    /// UI依存：UIBaseのパラメータ（位置、サイズ、回転など）を使用
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

    /// <summary>
    /// トランスフォームモードを設定する
    /// </summary>
    /// <param name="_mode">設定するモード</param>
    void SetTransformMode(TransformMode _mode) { transformMode_ = _mode; }

    /// <summary>
    /// トランスフォームモードを取得する
    /// </summary>
    /// <returns>現在のトランスフォームモード</returns>
    TransformMode GetTransformMode() const { return transformMode_; }

    bool GetIsHit() const { return isHit_; }
    void SetHit(bool _isHit) { isHit_ = _isHit; }
protected:
    TransformMode transformMode_ = TransformMode::UIDependent;  // デフォルトはUI依存

    bool isHit_ = false; // 衝突中フラグ
};