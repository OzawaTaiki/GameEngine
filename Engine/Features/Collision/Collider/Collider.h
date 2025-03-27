#pragma once

#include <Math/Vector/Vector3.h>
#include <Math/Matrix/Matrix4x4.h>

#include <Features/Collision/CollisionLayer/CollisionLayer.h>
#include <Features/Model/Transform/WorldTransform.h>

#include <string>
#include <cstdint>
#include <initializer_list>
#include <functional>
#include <variant>
#include <cassert>

class Collider;

struct ColliderInfo
{
    bool hasCollision = false;      // 衝突しているか
    Vector3 contactPoint;           // 衝突点
    Vector3 contactNormal;          // 衝突面の法線
    float penetration = 0.0f;       // めりこみ量

    ColliderInfo() = default;
};

struct CollisionPair
{
    Collider* colliderA = nullptr;
    Collider* colliderB = nullptr;
    ColliderInfo info;
};

enum class BoundingBox
{
    NONE = 0,
    Sphere_3D,
    AABB_3D,
    OBB_3D,
    Capsule_3D
};

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


class Collider
{
public:

    // コンストラクタ
    Collider() = default;
    // デストラクタ
    virtual ~Collider() = default;


    ///-----------------------------------------------
    /// コールバック関数
    ///

    // 衝突時の処理を行う関数を設定する
    void SetOnCollisionCallback(const std::function<void(Collider*, const ColliderInfo&)>& _fOnCollision) { fOnCollision_ = _fOnCollision; }

    ///-----------------------------------------------
    /// Layer周り
    ///

    // 自身のlayerを取得する
    uint32_t GetLayer() const { return collisionLayer_.GetLayer(); }

    // 衝突しないlayerを取得する
    uint32_t GetLayerMask() const { return collisionLayer_.GetLayerMask(); }

    //自身のlayerをセット設定する
    void SetLayer(const std::string& _layer) { collisionLayer_.SetLayer(_layer); }

    //自身のlayerMaskをセット設定する
    void SetLayerMask(const std::string& _layer) { collisionLayer_.SetLayerMask(_layer); }



    // バウンディングボックスを取得する
    BoundingBox GetBoundingBox() const { return boundingBox_; }

    // バウンディングボックスを設定する
    void SetBoundingBox(BoundingBox _boundingBox) { boundingBox_ = _boundingBox; }


    void SetWorldTransform(WorldTransform* _worldTransform) { worldTransform_ = _worldTransform; }

    WorldTransform* GetWorldTransform() const { return worldTransform_; }

    ///-----------------------------------------------
    /// 仮想関数
    ///

    // _pointが内部に含まれているか
    virtual bool Contains(const Vector3& _point) const = 0;

    // _pointから最も近い点を求める。
    virtual Vector3 GetClosestPoint(const Vector3& _point) const = 0;

private:

    bool isStatic_ = false; // 静的かどうか 動かない物体

    CollisionLayer collisionLayer_; // 衝突判定の属性

    BoundingBox boundingBox_ = BoundingBox::NONE; // 衝突判定の形状

    WorldTransform* worldTransform_ = nullptr; // ワールド行列

    std::function<void(Collider*,const ColliderInfo&)> fOnCollision_; // 衝突時の処理を行う関数
};


class SphereCollider : public Collider
{
public:
    // コンストラクタ
    SphereCollider() : Collider(), radius_(0.0f) { SetBoundingBox(BoundingBox::Sphere_3D); }
    // デストラクタ
    ~SphereCollider() = default;

    // 球の半径を設定する
    void SetRadius(float _radius) { radius_ = _radius; }
    // 球の半径を取得する
    float GetRadius() const { return radius_; }

    // 球の中に_pointが含まれているか
    bool Contains(const Vector3& _point) const override;
    // _pointから最も近い点を求める
    Vector3 GetClosestPoint(const Vector3& _point) const override;

private:

    float radius_ = 0.0f; // 球の半径
};

class AABBCollider : public Collider
{
public:
    // コンストラクタ
    AABBCollider() : Collider() { SetBoundingBox(BoundingBox::AABB_3D); }
    // デストラクタ
    ~AABBCollider() = default;

    // AABBの最小値と最大値を設定する
    void SetMinMax(const Vector3& _min, const Vector3& _max) { min_ = _min; max_ = _max; }

    // AABBの最小値を取得する
    Vector3 GetMin() const { return min_; }
    // AABBの最大値を取得する
    Vector3 GetMax() const { return max_; }

    // AABBの中に_pointが含まれているか
    bool Contains(const Vector3& _point) const override;
    // _pointから最も近い点を求める
    Vector3 GetClosestPoint(const Vector3& _point) const override;

private:
    Vector3 min_; // AABBの最小値
    Vector3 max_; // AABBの最大値
};


class OBBCollider : public Collider
{
public:
    // コンストラクタ
    OBBCollider() : Collider() { SetBoundingBox(BoundingBox::OBB_3D); }
    // デストラクタ
    ~OBBCollider() = default;

    // OBBの半分の大きさを設定する
    void SetHalfExtents(const Vector3& _halfExtents) { halfExtents_ = _halfExtents; }
    // OBBの半分の大きさを取得する
    Vector3 GetHalfExtents() const { return halfExtents_; }

    // OBBの基準点を設定する
    void SetLocalPivot(const Vector3& _localPivot) { localPivot_ = _localPivot; }
    // OBBの基準点を取得する
    Vector3 GetLocalPivot() const { return localPivot_; }

    // OBBの中に_pointが含まれているか
    bool Contains(const Vector3& _point) const override;
    // _pointから最も近い点を求める
    Vector3 GetClosestPoint(const Vector3& _point) const override;

    // OBBの頂点を取得する
    std::vector<Vector3> GetVertices() const;
    // OBBの中心を取得する
    Vector3 GetCenter() const;

private:
    Vector3 halfExtents_; // OBBの半分の大きさ
    Vector3 localPivot_; // OBBの基準点
};

class CapsuleCollider : public Collider
{
public:
    // コンストラクタ
    CapsuleCollider() : Collider(), direction_({ 0,1,0 }) { SetBoundingBox(BoundingBox::Capsule_3D); }
    // デストラクタ
    ~CapsuleCollider() = default;

    // カプセルの半径を設定する
    void SetRadius(float _radius) { radius_ = _radius; }
    // カプセルの半径を取得する
    float GetRadius() const { return radius_; }

    // カプセルの向きを設定する
    void SetDirection(const Vector3& _direction) { direction_ = _direction.Normalize(); }
    // カプセルの向きを取得する
    Vector3 GetDirection() const { return direction_; }

    // カプセルの基準点を設定する
    void SetLocalPivot(const Vector3& _localPivot) { localPivot_ = _localPivot; }
    // カプセルの基準点を取得する
    Vector3 GetLocalPivot() const { return localPivot_; }

    // カプセルの高さを設定する
    void SetHeight(float _height) { height_ = _height; }
    // カプセルの高さを取得する
    float GetHeight() const { return height_; }

    // カプセルの中に_pointが含まれているか
    bool Contains(const Vector3& _point) const override;
    // _pointから最も近い点を求める
    Vector3 GetClosestPoint(const Vector3& _point) const override;

    // カプセルの中心を取得する
    Vector3 GetCenter() const;

    // カプセルの中心線の両端を取得する
    void GetCapsuleSegment(Vector3& _start, Vector3& _end) const ;

    // 点と線分間の最近接点を計算
    Vector3 ClosestPointOnSegment(const Vector3& _point, const Vector3& _start, const Vector3& _end) const;


private:
    float radius_ = 0.0f; // カプセルの半径
    float height_ = 0.0f; // カプセルの高さ(半球含めた高さ)
    Vector3 direction_; // カプセルの向き
    Vector3 localPivot_; // カプセルの基準点
};