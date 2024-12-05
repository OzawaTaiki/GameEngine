#pragma once

#include "Shapes.h"
#include <list>
#include <unordered_map>
#include <string>
#include <memory>


class Collider;
class CollisionManager
{
public:

    // インスタンスを取得する
    static CollisionManager* GetInstance();

    // コライダーのリストをリセットする
    void ResetColliderList() { colliders_.clear(); }
    // コライダーをリストに登録する
    void RegisterCollider(Collider* _collider);
    // 全てのコライダーの衝突判定を行う
    void CheckAllCollision();

    // 属性を取得する
    uint32_t GetAtttibute(const std::string& _atrribute);
    // マスクを取得する
    uint32_t GetMask(const std::string& _atrribute);

private:

    // 衝突判定を行う
    void CheckCollisionPair(Collider* _colliderA, Collider* _colliderB);

    bool IsCollision(const Sphere& _sphere1, const Sphere& _sphere2);

    bool IsCollision(const Sphere& _sphere, const AABB& _aabb);

    bool IsCollision(const Sphere& _sphere, const OBB& _obb);

    bool IsCollision(const AABB& _aabb1, const AABB& _aabb2);

    bool IsCollision(const AABB& _aabb, const OBB& _obb);

    bool IsCollision(const OBB& _obb1, const OBB& _obb2);

    void CalculateProjectionRange(const OBB& _obb, const Vector3& _axis, float& _min, float& _max);

    OBB AABBConvertToOBB(const AABB& _aabb);

    // コライダーのリスト
    std::list<Collider*> colliders_;
    // 属性のマップ
    std::unordered_map<std::string, uint32_t> atrributeMap_;


//----------------禁止-----------------//
    // コンストラクタ
    CollisionManager() = default;
    // デストラクタ
    ~CollisionManager() = default;
    // コピーコンストラクタ
    CollisionManager(const CollisionManager&) = delete;
    // ムーブコンストラクタ
    CollisionManager(CollisionManager&&) = delete;
    // コピー代入演算子
    CollisionManager& operator=(const CollisionManager&) = delete;
    // ムーブ代入演算子
    CollisionManager& operator=(CollisionManager&&) = delete;
};