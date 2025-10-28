#pragma once
#include <Math/Vector/Vector2.h>


class IUICollider
{
public:
    virtual ~IUICollider() = default;

    /// <summary>
    /// 点がコライダー内にあるかどうかを判定する
    /// </summary>
    /// <param name="_point">判定する点の座標</param>
    /// <returns>点がコライダー内にある場合はtrue、そうでない場合はfalseを返す</returns>
    virtual bool IsPointInside(const Vector2& _point) const = 0;

};