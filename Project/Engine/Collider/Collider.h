#pragma once

#include "Vector3.h"
#include "Matrix4x4.h"
#include "Shapes.h"

#include <string>
#include <cstdint>
#include <initializer_list>
#include <functional>
#include <variant>
#include <cassert>

/*
    衝突判定のクラス
使い方
Initilaizeで
    Colliderのインスタンスを作成し、SetBoundingBoxで衝突判定の形状を設定する
    SetShapeで形状のデータを設定する
    SetAtrributeで自身の属性を設定する
    SetMaskで当たらない属性を設定する
    SetGetWorldMatrixFuncでワールド行列を取得する関数を設定する
    SetOnCollisionFuncで衝突時の処理を行う関数を設定する
    SetReferencePointで基準点を設定する(初期は{0,0,0})

Updateで
    CollisionManagerのRegisterColliderでColliderを登録する
    ※必ず毎フレーム登録すること

*/
class Collider final
{
public:
    enum class BoundingBox : uint8_t
    {
        Sphere_3D = 0,
        AABB_3D = 1,
        OBB_3D = 2,

        NONE
    };

    void Update();
    void Draw();


    void SetBoundingBox(BoundingBox _set) { boundingBox_ = _set; }
    BoundingBox GetBoundingBox() const { return boundingBox_; }

    // 衝突判定の形状を設定する
    // _shape->形状
    void SetShape(float _radius);
    void SetShape(const Vector3& _min, const Vector3& _max);


    // 衝突判定の形状を取得する
    template <typename T>
    T GetShape() const;

    // ワールド行列を取得する
    Matrix4x4 GetWorldMatrix()const { return fGetWorldMatrix_(); }
    // 衝突時の処理を行う
    void OnCollision(const Collider* _other);

    // 判定属性を設定する
    // _atrribute->自信の属性
    void SetAtrribute(const std::string& _atrribute);
    // 判定マスクを設定する
    // _atrribute->当たらない属性
    void SetMask(const std::string& _atrribute);
    // 判定マスクを設定する
    // _atrribute->当たらない属性
    void SetMask(std::initializer_list<std::string> _atrribute);

    // ワールド行列を取得する関数を設定する
    void SetGetWorldMatrixFunc(std::function<Matrix4x4(void)> _f) { fGetWorldMatrix_ = _f; }
    // 衝突時の処理を行う関数を設定する
    void SetOnCollisionFunc(std::function<void(const Collider*)> _f) { fOnCollision_ = _f; }

    void SetReferencePoint(const Vector3& _referencePoint);

    // 判定属性を取得する
    uint32_t GetMask()const { return mask_; }
    // 判定マスクを取得する
    uint32_t GetAtrribute_()const { return atrribute_; }

    std::string GetName()const { return name_; }

    void NotHit() { preIsHit_ = isHit_; isHit_ = false; }

    // 衝突した瞬間
    bool IsCollisionEnter()const { return isHit_ && !preIsHit_; }

    // 衝突中
    bool IsCollisionStay()const { return isHit_; }

    // 衝突から離れた瞬間
    bool IsCollisionExit()const { return !isHit_ && preIsHit_; }

    void RegsterCollider();

private:

    // 衝突判定の形状とそのデータ
    std::variant<Sphere, AABB, OBB> shape_;
    // 衝突判定のサイズ
    Vector3 size_ = {};

    bool isHit_ = false;
    bool preIsHit_ = false;

    BoundingBox boundingBox_ = BoundingBox::NONE;
    uint32_t atrribute_ = 0x0;
    uint32_t mask_ = 0x1;

    std::string name_ = "";

    std::function<Matrix4x4(void)> fGetWorldMatrix_ = nullptr;
    std::function<void(const Collider*)> fOnCollision_ = nullptr;
};

template<typename T>
inline T Collider::GetShape() const
{
    if (std::holds_alternative<T>(shape_))
    {
        return std::get<T>(shape_);
    }
    else
    {
        assert(false && "has not this type");
        return T();
    }

}
