#pragma once

#include <numbers>

namespace Engine {

inline namespace MathConstants {

    // ========================================
    // 円周率関連
    // ========================================

    /// 円周率π
    constexpr float kPi = std::numbers::pi_v<float>;

    /// 2π（1周）
    constexpr float kTwoPi = kPi * 2.0f;

    /// π/2（90度）
    constexpr float kHalfPi = kPi * 0.5f;

    /// π/4（45度）
    constexpr float kQuarterPi = kPi * 0.25f;

    // ========================================
    // 角度変換
    // ========================================

    /// 角度からラジアンへの変換係数 (π/180)
    constexpr float kDegToRad = kPi / 180.0f;

    /// ラジアンから角度への変換係数 (180/π)
    constexpr float kRadToDeg = 180.0f / kPi;

    /// 360度（1周）
    constexpr float kFullCircleDegrees = 360.0f;

    /// 180度（半周）
    constexpr float kHalfCircleDegrees = 180.0f;

    /// 90度（1/4周）
    constexpr float kQuarterCircleDegrees = 90.0f;

    // ========================================
    // 許容誤差（エプシロン）
    // ========================================

    /// 浮動小数点数の比較用の非常に小さい値
    constexpr float kEpsilon = 0.0001f;

    /// より粗い許容誤差（Z-Fighting対策など）
    constexpr float kEpsilonCoarse = 0.001f;

    /// 非常に細かい許容誤差
    constexpr float kEpsilonFine = 0.00001f;

    // ========================================
    // よく使う数学定数（STLから取得）
    // ========================================

    /// 1/2（中心計算などに使用）
    constexpr float kHalf = 0.5f;

    /// 1/3
    constexpr float kOneThird = 1.0f / 3.0f;

    /// 2/3
    constexpr float kTwoThirds = 2.0f / 3.0f;

    /// √2
    constexpr float kSqrt2 = std::numbers::sqrt2_v<float>;

    /// √3
    constexpr float kSqrt3 = std::numbers::sqrt3_v<float>;

    /// 黄金比φ
    constexpr float kGoldenRatio = std::numbers::phi_v<float>;

    /// 自然対数の底e
    constexpr float kE = std::numbers::e_v<float>;

    /// log_e(2)
    constexpr float kLn2 = std::numbers::ln2_v<float>;

    /// log_e(10)
    constexpr float kLn10 = std::numbers::ln10_v<float>;

} // inline namespace MathConstants

} // namespace Engine
