#pragma once

#include "Vector3.h"

#include <string>
#include <cstdint>
#include <initializer_list>
class Collider
{
public:
    enum class BoundingBox : uint32_t
    {
        Sphere_3D = 0,
        AABB_3D = 1,
        OBB_3D = 2
    };

    virtual Vector3 GetWorldPosition()const = 0;
    virtual void OnCollision() = 0;

    void Draw() const;

    float GetRadius()const { return radius_; }
    Vector3 GetSize()const { return pSize_ ? *pSize_ : size_; }
    Vector3 GetRotate()const { return pRotate_ ? *pRotate_ : rotate_; }
    Vector3 GetAnchor()const { return anchor_; }
    Vector3 GetOffset()const { return offset_; }

    void SetBoundingBox(BoundingBox _set) { boundingBox_ = _set; }
    BoundingBox GetBoundingBox()const { return boundingBox_; }

    // 判定属性を設定する
    // _atrribute->自信の属性
    void SetAtrribute(const std::string& _atrribute);
    // 判定マスクを設定する
    // _atrribute->当たらない属性
    void SetMask(const std::string& _atrribute);
    // 判定マスクを設定する
    // _atrribute->当たらない属性
    void SetMask(std::initializer_list<std::string> _atrribute);

    uint32_t GetMask() const { return mask_; }
    uint32_t GetAtrribute_() const { return atrribute_; }


protected:
    // 衝突判定用変数
    float radius_;  // 球
    const Vector3* pSize_;  // AABB
    const Vector3* pRotate_;
    Vector3 size_ = { 1,1,1 };
    Vector3 rotate_ = {};
    Vector3 offset_ = {};
    Vector3 anchor_ = { 0.5f,0.5f ,0.5f };

    BoundingBox boundingBox_ = BoundingBox::Sphere_3D;
    uint32_t atrribute_ = 0x0;
    uint32_t mask_ = 0x1;

private:
};