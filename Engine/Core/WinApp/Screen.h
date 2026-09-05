#pragma once

#include <Math/Vector/Vector2.h>

#include <cstdint>


namespace Engine {

/// <summary>
/// ゲームの描画解像度
/// レンダーターゲット・カメラのアスペクト比・2Dの正射影がこれを基準にするので，
/// Debug と Release で同じ値にすること (変えると見え方がずれる)
/// Windows.h を引かないので，どこからでも軽く include できる
/// </summary>
namespace Screen {

    /// <summary>
    /// 描画解像度を設定する
    /// 初期化順の都合があるので Framework 以外から呼ばないこと
    /// </summary>
    void SetSize(uint32_t _width, uint32_t _height);

    /// <summary> 横幅(ピクセル) </summary>
    uint32_t Width();
    /// <summary> 高さ(ピクセル) </summary>
    uint32_t Height();

    /// <summary> 横幅(float) </summary>
    float WidthF();
    /// <summary> 高さ(float) </summary>
    float HeightF();

    /// <summary> サイズ(float) </summary>
    const Vector2& Size();

    /// <summary> アスペクト比 (横 / 縦) </summary>
    float Aspect();

    /// <summary> 画面中央の座標 </summary>
    Vector2 Center();

    /// <summary> ピクセル座標を 0.0 ~ 1.0 に正規化する </summary>
    Vector2 ToNormalized(const Vector2& _position);

    /// <summary> 0.0 ~ 1.0 の座標をピクセル座標に戻す </summary>
    Vector2 FromNormalized(const Vector2& _normalized);

} // namespace Screen


/// <summary>
/// OSウィンドウのクライアント領域 (= スワップチェインのバックバッファ) のサイズ
/// Debug ではエディタのウィンドウを並べる余白のぶん Screen より大きいことがある
/// </summary>
namespace Window {

    /// <summary> サイズを設定する。WinApp 以外から呼ばないこと </summary>
    void SetSize(uint32_t _width, uint32_t _height);

    /// <summary> 横幅(ピクセル) </summary>
    uint32_t Width();
    /// <summary> 高さ(ピクセル) </summary>
    uint32_t Height();

    /// <summary> サイズ(float) </summary>
    const Vector2& Size();

    /// <summary> アスペクト比 (横 / 縦) </summary>
    float Aspect();

} // namespace Window

} // namespace Engine
