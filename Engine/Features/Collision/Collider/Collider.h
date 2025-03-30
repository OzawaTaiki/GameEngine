#pragma once

#include <Math/Vector/Vector3.h>
#include <Math/Matrix/Matrix4x4.h>

#include <Features/Collision/CollisionLayer/CollisionLayer.h>
#include <Features/Model/Transform/WorldTransform.h>
#include <Features/Json/JsonBinder.h>

#include <string>
#include <cstdint>
#include <initializer_list>
#include <functional>
#include <variant>
#include <cassert>

class Collider;

enum class CollisionState
{
    None = 0, // 何もしていない
    Enter, // 衝突した瞬間
    Stay, // 衝突している間
    Exit // 離れた瞬間
};

struct ColliderInfo
{
    bool hasCollision = false;      // 衝突しているか
    Vector3 contactPoint;           // 衝突点
    Vector3 contactNormal;          // 衝突面の法線
    float penetration = 0.0f;       // めりこみ量
    CollisionState state = CollisionState::None; // 衝突状態

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

std::string ToString(BoundingBox _box);

/// Scene::Initialize(){
/// Collider* collider = new SphereCollider();
/// collider->SetLayer("Player");
/// collider->SetLayerMask("Enemy");
/// collider->SetRadius(1.0f);
/// collider->SetWorldTransform(worldTransform);
/// collider->SetOnCollisionCallback([](Collider* _self, const ColliderInfo& _info) {
///     // 衝突時の処理
/// });
/// }
///
/// Scene::Update(){
/// CollisionManager::GetInstance()->RegisterCollider(collider);
/// }
class Collider
{
public:
    // コンストラクタ
    Collider();
    // デストラクタ
    virtual ~Collider() = default;

    // 衝突イベント処理（CollisionManagerから呼ばれる）
    void OnCollision(Collider* _other, const ColliderInfo& _info);

    // 衝突状態を更新（CollisionManagerから呼ばれる）
    void UpdateCollisionState();

    // 特定のコライダーとの衝突状態を取得
    CollisionState GetCollisionState(Collider* _other) const;

    // 任意のコライダーと衝突しているかどうか
    bool IsColliding() const { return !currentCollisions_.empty(); }

    // 特定のコライダーと衝突しているかどうか
    bool IsCollidingWith(Collider* _other) const;

    // 描画メソッド
    virtual void Draw() = 0;

    // ファイルから読み込み
    virtual void Load(const std::string& _name) = 0;

    // ファイルに保存
    virtual void Save(const std::string& _name) = 0;

    // 衝突コールバック設定（状態はColliderInfoのstateフィールドで判別）
    void SetOnCollisionCallback(const std::function<void(Collider*, const ColliderInfo&)>& _callback)
    {
        fOnCollision_ = _callback;
    }

    // 自身のlayerを取得する
    uint32_t GetLayer() const { return collisionLayer_.GetLayer(); }

    // 衝突しないlayerを取得する
    uint32_t GetLayerMask() const { return collisionLayer_.GetLayerMask(); }

    // 自身のlayerをセット設定する
    void SetLayer(const std::string& _layer) { collisionLayer_.SetLayer(_layer); }

    // 自身のlayerMaskをセット設定する
    void SetLayerMask(const std::string& _layer) { collisionLayer_.SetLayerMask(_layer); }

    // バウンディングボックスを取得する
    BoundingBox GetBoundingBox() const { return boundingBox_; }
    // バウンディングボックスを設定する
    void SetBoundingBox(BoundingBox _boundingBox) { boundingBox_ = _boundingBox; }


    // ワールドトランスフォームを設定する
    void SetWorldTransform(const WorldTransform* _worldTransform) { worldTransform_ = _worldTransform; }

    // ワールドトランスフォームを取得する
    const WorldTransform* GetWorldTransform() const;

    // _pointが内部に含まれているか
    virtual bool Contains(const Vector3& _point) const = 0;

    // _pointから最も近い点を求める
    virtual Vector3 GetClosestPoint(const Vector3& _point) const = 0;

    // 現在衝突中のコライダーを追加（CollisionManagerから呼ばれる）
    void AddCurrentCollision(Collider* _other, const ColliderInfo& _info);


protected:

    bool InitJsonBinder(const std::string& _name, const std::string& _folderPath="Resources/Data/Colliders/");

    void ImGui();

    JsonBinder* jsonBinder_ = nullptr;

    std::string name_;
private:
    // 衝突状態の管理
    struct CollisionData
    {
        ColliderInfo info;
        bool wasColliding = false;
        bool isColliding = false;
    };

    bool isStatic_ = false; // 静的かどうか 動かない物体
    CollisionLayer collisionLayer_; // 衝突判定の属性
    BoundingBox boundingBox_ = BoundingBox::NONE; // 衝突判定の形状
    const WorldTransform* worldTransform_ = nullptr; // ワールド行列

    // 衝突状態の記録
    std::unordered_map<Collider*, CollisionData> collisionMap_;

    // 現在のフレームで衝突しているコライダーのリスト（UpdateCollisionStateで使用）
    std::vector<Collider*> currentCollisions_;

    // 衝突コールバック関数
    std::function<void(Collider*, const ColliderInfo&)> fOnCollision_;

    WorldTransform defaultTransform_;

};



class SphereCollider : public Collider
{
public:
    // コンストラクタ
    SphereCollider(const std::string& _name = "Collider");
    // デストラクタ
    ~SphereCollider() = default;

    void Draw() override;

    void Load(const std::string& _name) override;

    void Save(const std::string& _name) override;

    // 球の半径を設定する
    void SetRadius(float _radius) { radius_ = _radius; }
    // 球の半径を取得する
    float GetRadius() const { return radius_; }

    // 球の中に_pointが含まれているか
    bool Contains(const Vector3& _point) const override;
    // _pointから最も近い点を求める
    Vector3 GetClosestPoint(const Vector3& _point) const override;


    void ImGui();
private:

    float radius_ = 0.0f; // 球の半径
};

class AABBCollider : public Collider
{
public:
    // コンストラクタ
    AABBCollider(const std::string& _name = "Collider");
    // デストラクタ
    ~AABBCollider() = default;

    void Draw() override;

    void Load(const std::string& _name) override;

    void Save(const std::string& _name) override;


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


    void ImGui();
private:
    Vector3 min_; // AABBの最小値
    Vector3 max_; // AABBの最大値
};


class OBBCollider : public Collider
{
public:
    // コンストラクタ
    OBBCollider(const std::string& _name = "Collider");
    // デストラクタ
    ~OBBCollider() = default;

    void Draw() override;

    void Load(const std::string& _name) override;

    void Save(const std::string& _name) override;

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

    void ImGui();
private:
    Vector3 halfExtents_; // OBBの半分の大きさ
    Vector3 localPivot_; // OBBの基準点
};

class CapsuleCollider : public Collider
{
public:
    // コンストラクタ
    CapsuleCollider(const std::string& _name = "Collider");
    // デストラクタ
    ~CapsuleCollider() = default;

    void Draw() override;

    void Load(const std::string& _name) override;

    void Save(const std::string& _name) override;

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

    void ImGui();

private:
    float radius_ = 0.0f; // カプセルの半径
    float height_ = 0.0f; // カプセルの高さ(半球含めた高さ)
    Vector3 direction_; // カプセルの向き
    Vector3 localPivot_; // カプセルの基準点
};