#include "CollisionManager.h"
#include <Features/LineDrawer/LineDrawer.h>
#include <Debug/ImGuiDebugManager.h>
#include <Features/Collision/CollisionLayer/CollisionLayerManager.h>
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
    isDrawEnabled_ = true;
}

void CollisionManager::Finalize()
{
    colliders_.clear();
    collisionPairs_.clear();
}

void CollisionManager::Update()
{
    // 衝突判定を実行
    CheckCollisions();

    // 衝突応答を実行
    ResolveCollisions();

    // 衝突状態を更新
    UpdateCollisionStates();

    // デバッグ描画が有効なら描画
    if (isDrawEnabled_)
    {
        DrawColliders();
    }

    // 全てのコライダーをクリア（次のフレームのために）
    colliders_.clear();
    collisionPairs_.clear();
}

void CollisionManager::RegisterCollider(Collider* _collider)
{
    // nullチェック
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
    // ブロードフェーズの更新（空間分割等の最適化）
    UpdateBroadPhase();

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
#ifdef _DEBUG
            //std::string layerA = CollisionLayerManager::GetInstance()->GetLayerStr(colliderA->GetLayer());
            //std::string layerB = CollisionLayerManager::GetInstance()->GetLayerStr(colliderB->GetLayer());
            //Debug::Log("Checking collision between " + layerA + " and " + layerB + "\n");
#endif // _DEBUG

            // CollisionDetectorを使用して衝突判定を実行
            if (CollisionDetector::DetectCollision(colliderA, colliderB, info))
            {
                // 衝突情報を保存
                CollisionPair pair;
                pair.colliderA = colliderA;
                pair.colliderB = colliderB;
                pair.info = info;
                collisionPairs_.push_back(pair);

                // 各コライダーに現在の衝突を記録
                colliderA->AddCurrentCollision(colliderB, info);

                // 衝突情報を反転して相手側にも記録
                ColliderInfo reversedInfo = info;
                reversedInfo.contactNormal = -info.contactNormal;
                colliderB->AddCurrentCollision(colliderA, reversedInfo);
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

void CollisionManager::UpdateCollisionStates()
{
    // 全てのコライダーの衝突状態を更新
    for (auto collider : colliders_)
    {
        collider->UpdateCollisionState();
    }
}

void CollisionManager::DrawColliders()
{
    // デバッグ描画が無効ならスキップ
    if (!isDrawEnabled_)
        return;

    // 衝突中のコライダーを赤色で描画
    std::vector<Collider*> collidingColliders;

    // 衝突中のコライダーを抽出
    for (const auto& pair : collisionPairs_)
    {
        collidingColliders.push_back(pair.colliderA);
        collidingColliders.push_back(pair.colliderB);
    }

    // 全てのコライダーを描画
    for (auto collider : colliders_)
    {
        // コライダーが衝突中か判定
        bool isColliding = std::find(collidingColliders.begin(), collidingColliders.end(), collider) != collidingColliders.end();

        // 衝突状態に応じて色を設定
        if (isColliding)
        {
            // 衝突中は赤色
            LineDrawer::GetInstance()->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
        }
        else
        {
            // 通常時は緑色
            LineDrawer::GetInstance()->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });
        }

        // コライダーを描画
        collider->Draw();
    }

    // 衝突点と法線を描画
    LineDrawer::GetInstance()->SetColor({ 1.0f , 1.0f , 1.0f , 1.0f });
    //LineDrawer::GetInstance()->SetColor({ 0.7f, 0.7f, 0.7f, 1.0f }); // ぐれー
    for (const auto& pair : collisionPairs_)
    {
        // 衝突点を描画
        Vector3 contactPoint = pair.info.contactPoint;
        Vector3 normalEnd = contactPoint + pair.info.contactNormal * 0.5f; // 法線の長さを適当に調整

        // 法線を描画
        LineDrawer::GetInstance()->RegisterPoint(contactPoint, normalEnd);
    }
}

void CollisionManager::UnregisterCollider(Collider* _collider)
{
    if (_collider == nullptr)
        return;

    // コライダーリストから削除
    auto it = std::find(colliders_.begin(), colliders_.end(), _collider);
    if (it != colliders_.end()) {
        colliders_.erase(it);
    }

    if (collisionPairs_.empty())
        return;

    // 衝突ペアからも削除
    collisionPairs_.erase(
        std::remove_if(collisionPairs_.begin(), collisionPairs_.end(),
            [_collider](const CollisionPair& pair) {
                return pair.colliderA == _collider || pair.colliderB == _collider;
            }),
        collisionPairs_.end());
}

void CollisionManager::ImGui()
{
#ifdef _DEBUG
    ImGui::PushID(this);
    // デバッグ描画の有効/無効を設定
    ImGui::Checkbox("Draw Colliders", &isDrawEnabled_);

    ImGui::PopID();
#endif
}

void CollisionManager::ResolveCollision(const CollisionPair& _pair)
{
    // 両方のコライダーに衝突通知を送る
    if (_pair.colliderA && _pair.colliderB)
    {
        // コライダーAに衝突通知
        _pair.colliderA->OnCollision(_pair.colliderB, _pair.info);

        // コライダーBに衝突通知（法線方向を反転）
        ColliderInfo invertedInfo = _pair.info;
        invertedInfo.contactNormal = -_pair.info.contactNormal;

        _pair.colliderB->OnCollision(_pair.colliderA, invertedInfo);
    }
}

void CollisionManager::UpdateBroadPhase()
{
    // 空間分割などの最適化を行う場合はここに実装
    // 現在は単純な総当たりで実装
}

CollisionManager::CollisionManager()
{
    ImGuiDebugManager::GetInstance()->AddColliderDebugWindow("CollisionManager", [&]() {ImGui(); });
}
