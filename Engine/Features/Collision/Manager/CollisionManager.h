#pragma once

#include <Features/Collision/Collider/Collider.h>

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

    // 初期化
    void Initialize();

    // 終了処理
    void Finalize();

    // 毎フレームの更新
    void Update();

    // コライダーを登録する
    void RegisterCollider(Collider* _collider);

    // 衝突判定を実行する
    void CheckCollisions();

    // 衝突応答を実行する
    void ResolveCollisions();

private:
    // 衝突応答を実行する
    void ResolveCollision(const CollisionPair& _pair);

private:
    // コライダーのリスト
    std::vector<Collider*> colliders_;

    // 衝突ペアのリスト
    std::vector<CollisionPair> collisionPairs_;

private:
    // コンストラクタ
    CollisionManager() = default;

    // デストラクタ
    ~CollisionManager() = default;

    // コピーコンストラクタとコピー代入演算子を禁止
    CollisionManager(const CollisionManager&) = delete;
    CollisionManager& operator=(const CollisionManager&) = delete;
};