#include "CollisionManager.h"
#include <Features/Collision/Collider/Collider.h>
#include <Features/Collision/Detector/CollisionDetector.h>
#include <algorithm>

CollisionManager* CollisionManager::GetInstance()
{
    static CollisionManager instance;
    return &instance;
}

void CollisionManager::Initialize()
{
    colliders_.clear();
    collisionPairs_.clear();
}

void CollisionManager::Finalize()
{
    colliders_.clear();
    collisionPairs_.clear();
}

void CollisionManager::Update()
{
    // 全てのコライダーをクリア
    colliders_.clear();
    collisionPairs_.clear();
}

void CollisionManager::RegisterCollider(Collider* _collider)
{ // nullチェック
    if (_collider == nullptr)
        return;

    // 既に登録されているかチェック
    auto it = std::find(colliders_.begin(), colliders_.end(), _collider);
    if (it != colliders_.end())
        return;

    // コライダーを登録
    colliders_.push_back(_collider);
}

void CollisionManager::CheckCollisions()
{

    // 全てのコライダー同士の組み合わせで衝突判定を実行
    for (size_t i = 0; i < colliders_.size(); ++i)
    {
        Collider* colliderA = colliders_[i];

        for (size_t j = i + 1; j < colliders_.size(); ++j)
        {
            Collider* colliderB = colliders_[j];

            // レイヤーマスクでフィルタリング
            if ((colliderA->GetLayer() & colliderB->GetLayerMask()) != 0 ||
                (colliderB->GetLayer() & colliderA->GetLayerMask()) != 0)
            {
                continue; // 衝突しないように設定されている
            }

            // 衝突情報
            ColliderInfo info;

            // CollisionDetectorを使用して衝突判定を実行
            if (CollisionDetector::DetectCollision(colliderA, colliderB, info))
            {
                // 衝突情報を保存
                CollisionPair pair;
                pair.colliderA = colliderA;
                pair.colliderB = colliderB;
                pair.info = info;
                collisionPairs_.push_back(pair);
            }
        }
    }
}

void CollisionManager::ResolveCollisions()
{
    // 全ての衝突ペアに対して衝突応答を実行
    for (const auto& pair : collisionPairs_)
    {
        ResolveCollision(pair);
    }
}

void CollisionManager::ResolveCollision(const CollisionPair& _pair)
{
    // 両方のコライダーで衝突時のコールバックを呼び出す
    if (_pair.colliderA && _pair.colliderB)
    {
        // コライダーAのコールバック関数を呼び出す
        if (_pair.colliderA)
        {
            // 衝突時のコールバック関数が設定されていれば呼び出す
            std::function<void(Collider*, const ColliderInfo&)> callback = [&](Collider* collider, const ColliderInfo& info) {
                // 何もしない（コライダー側でコールバックを設定している場合はそれが呼ばれる）
                };

            callback(_pair.colliderB, _pair.info);
        }

        // コライダーBのコールバック関数を呼び出す
        if (_pair.colliderB)
        {
            // 反転した衝突情報を作成
            ColliderInfo invertedInfo = _pair.info;
            invertedInfo.contactNormal = -_pair.info.contactNormal;

            // 衝突時のコールバック関数が設定されていれば呼び出す
            std::function<void(Collider*, const ColliderInfo&)> callback = [&](Collider* collider, const ColliderInfo& info) {
                // 何もしない（コライダー側でコールバックを設定している場合はそれが呼ばれる）
                };

            callback(_pair.colliderA, invertedInfo);
        }
    }
}
