#include "CollisionManager.h"
#include <Features/LineDrawer/LineDrawer.h>
#include <Debug/ImGuiDebugManager.h>
#include <Features/Collision/CollisionLayer/CollisionLayerManager.h>
#include <algorithm>
#include <future>

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
    //ResolveCollisions();

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
    // 結果バッファをクリア
    threadCollisionResults_.clear();
    collisionPairs_.clear();

    // 処理する衝突ペアの総数を計算
    size_t totalColliders = colliders_.size();
    size_t totalPairs = totalColliders * (totalColliders - 1) / 2; // 組み合わせの総数

    if (totalPairs == 0) return;

    // スレッドあたりの処理数を計算（最低でも100ペアを確保）
    size_t pairsPerThread = (std::max)(totalPairs / threadCount_, size_t(100));

    // 各スレッドの処理を非同期で実行
    std::vector<std::future<void>> futures;

    for (size_t startIdx = 0; startIdx < totalPairs; startIdx += pairsPerThread)
    {
        size_t endIdx = (std::min)(startIdx + pairsPerThread, totalPairs);

        futures.push_back(std::async(std::launch::async, [this, startIdx, endIdx, totalColliders]() {
            // ローカル結果バッファ
            std::vector<CollisionPair> localResults;

            // 範囲内のペアインデックスを処理
            for (size_t pairIdx = startIdx; pairIdx < endIdx; ++pairIdx)
            {
                // インデックスから実際のi, jを逆算（三角数の逆計算）
                size_t i, j;
                CalculateIndicesFromPairIndex(pairIdx, totalColliders, i, j);

                if (i >= colliders_.size() || j >= colliders_.size() || i == j)
                    continue;

                Collider* a = colliders_[i];
                Collider* b = colliders_[j];

                // レイヤーマスクによるフィルタリング
                if ((a->GetLayer() & b->GetLayerMask()) != 0 ||
                    (b->GetLayer() & a->GetLayerMask()) != 0)
                {
                    continue;
                }

                // 詳細な衝突判定
                ColliderInfo info;
                if (CollisionDetector::DetectCollision(a, b, info))
                {
                    // 衝突情報を記録
                    CollisionPair pair;
                    pair.colliderA = a;
                    pair.colliderB = b;
                    pair.info = info;
                    localResults.push_back(pair);
                }
            }

            // ローカル結果をグローバル結果にマージ
            if (!localResults.empty())
            {
                std::lock_guard<std::mutex> lock(collisionResultsMutex_);
                threadCollisionResults_.insert(
                    threadCollisionResults_.end(),
                    localResults.begin(),
                    localResults.end()
                );
            }
            }));
    }

    // すべてのスレッドの完了を待機
    for (auto& future : futures)
    {
        future.wait();
    }

    // 結果を確定
    collisionPairs_ = std::move(threadCollisionResults_);

    // 衝突通知を実行
    for (const auto& pair : collisionPairs_)
    {
        // 各コライダーに衝突を通知
        pair.colliderA->OnCollision(pair.colliderB, pair.info);

        ColliderInfo reversedInfo = pair.info;
        reversedInfo.contactNormal = -pair.info.contactNormal;
        pair.colliderB->OnCollision(pair.colliderA, reversedInfo);

        // 現在の衝突を記録
        pair.colliderA->AddCurrentCollision(pair.colliderB, pair.info);
        pair.colliderB->AddCurrentCollision(pair.colliderA, reversedInfo);
    }
}


// ペアインデックスからi, jを計算する補助関数
void CollisionManager::CalculateIndicesFromPairIndex(size_t pairIdx, size_t n, size_t& i, size_t& j)
{
    // 三角数列の逆算（二次方程式を解く）
    i = n - 2 - std::floor(std::sqrt(-8 * pairIdx + 4 * n * (n - 1) - 7) / 2.0 - 0.5);
    j = pairIdx + i + 1 - n * (n - 1) / 2 + (n - i) * ((n - i) - 1) / 2;
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
#endif // _DEBUG
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
#ifdef _DEBUG


    ImGuiDebugManager::GetInstance()->AddColliderDebugWindow("CollisionManager", [&]() {ImGui(); });
#endif // DEBUG
}
