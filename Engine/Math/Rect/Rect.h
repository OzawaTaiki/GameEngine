#pragma once

#include <Math/Vector/Vector2.h>

namespace Engine {

/// <summary>
/// 左上の角とサイズで定義される軸に沿った長方形
/// </summary>
struct Rect
{
    /// <summary>
    /// コンストラクタ
    /// </summary>
    Rect() : leftTop(0.0f, 0.0f), size(0.0f, 0.0f) {}
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="lt">左上の角 </param>
    /// <param name="sz">サイズ </param>
    Rect(const Vector2& lt, const Vector2& sz) : leftTop(lt), size(sz) {}
    /// <summary>
    /// コンストラクタ
    /// </summary>
    /// <param name="ltX">左上のX座標 </param>
    /// <param name="ltY">左上のY座標 </param>
    /// <param name="width">幅 </param>
    /// <param name="height">高さ </param>
    Rect(float ltX, float ltY, float width, float height) : leftTop(ltX, ltY), size(width, height) {}

    /// <summary>
    /// 左上と右下の点から長方形を作成
    /// </summary>
    /// <param name="leftTop">左上の角 </param>
    /// <param name="rightBottom">右下の角 </param>
    /// <returns> 長方形 </returns>
    static Rect FromPoints(const Vector2& leftTop, const Vector2& rightBottom);

    /// <summary>
    /// 左上の角を取得
    /// </summary>
    /// <returns>左上の角 </returns>
    Vector2 GetLeftTop() const;
    /// <summary>
    /// 右下の角を取得
    /// </summary>
    /// <returns>右下の角 </returns>
    Vector2 GetRightBottom() const;
    /// <summary>
    /// 中心点を取得
    /// </summary>
    /// <returns>中心点 </returns>
    Vector2 GetCenter() const;

    /// <summary>
    /// 正規化された位置にある点を取得
    /// </summary>
    /// <param name="normalizedPos"> 正規化された位置 (0.0～1.0) </param>
    /// <returns>点 </returns>
    Vector2 GetPointAt(const Vector2& normalizedPos) const;
    /// <summary>
    /// 正規化された位置にある点を取得
    /// </summary>
    /// <param name="normalizedX">正規化されたX位置 (0.0～1.0) </param>
    /// <param name="normalizedY">正規化されたY位置 (0.0～1.0) </param>
    /// <returns>点</returns>
    Vector2 GetPointAt(float normalizedX, float normalizedY) const;

    /// <summary>
    /// 点が長方形内に含まれているか
    /// </summary>
    /// <param name="point">確認する点座標</param>
    /// <returns> 含まれているならtrue </returns>
    bool Contains(const Vector2& point) const;

    // 左上の角
    Vector2 leftTop;
    // サイズ
    Vector2 size;
};

} // namespace Engine