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

void CollisionManager::Initialize(const Vector2& _fieldSize, uint32_t _level, const Vector2& _leftBotom, float _gridSize)
{
    colliders_.clear();
    collisionPairs_.clear();

    // QuadTreeの初期化
    quadTree_ = std::make_unique<QuadTree>();
    quadTree_->Initialize(_fieldSize, _level, _leftBotom);

    // スパイラルハッシュグリッドの初期化
    spiralHashGrid_ = std::make_unique<SpatialHashGrid>(_gridSize);
    spiralHashGrid_->Clear();
}

void CollisionManager::Finalize()
{
    colliders_.clear();
    collisionPairs_.clear();
}

void CollisionManager::Update()
{
    // 衝突判定を実行
    if(enableBroadPhase_)
        CheckCollisionsWithBroadPhase();
    else
        CheckCollisions();

    // 衝突応答を実行
    //ResolveCollisions();

    // 衝突状態を更新
    UpdateCollisionStates();

    DrawColliders();

#ifdef _DEBUG
    colliderCount_ = static_cast<int32_t>(colliders_.size());
    collisionPairCount_ = static_cast<int32_t>(collisionPairs_.size());
#endif // _DEBUG

    // 全てのコライダーをクリア（次のフレームのために）
    colliders_.clear();
    collisionPairs_.clear();

    quadTree_->Reset();
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

void CollisionManager::RegisterStaticCollider(Collider* _collider)
{
    // nullチェック
    if (_collider == nullptr)
        return;

    // 既に登録されているかチェック
    auto it = std::find(staticColliders_.begin(), staticColliders_.end(), _collider);
    if (it != staticColliders_.end())
        return;

    // コライダーを登録
    staticColliders_.push_back(_collider);

    spiralHashGrid_->AddCollider(_collider);
}

void CollisionManager::CheckCollisionsWithBroadPhase()
{
    // ブロードフェーズの更新（空間分割等の最適化）
    UpdateBroadPhase();

    // 判定対象がない場合は早期リターン
    if (potentialCollisions_.empty())
        return;

    CheckCollisionsRange();
}

void CollisionManager::CheckCollisions()
{
    potentialCollisions_.clear();

    for (size_t i = 0; i < colliders_.size(); ++i)
    {
        Collider* colliderA = colliders_[i];
        for (size_t j = i + 1; j < colliders_.size(); ++j)
        {
            Collider* colliderB = colliders_[j];
            potentialCollisions_.emplace_back(colliderA, colliderB);
        }
        for (auto& staticCollider : staticColliders_)
        {
            Collider* colliderB = staticCollider;
            potentialCollisions_.emplace_back(colliderA, colliderB);
        }
    }


    // 判定対象がない場合は早期リターン
    if (potentialCollisions_.empty())
        return;

    CheckCollisionsRange();
}

void CollisionManager::CheckCollisionsRange()
{
    for (size_t i = 0; i < potentialCollisions_.size(); ++i)
    {
        const auto& pair = potentialCollisions_[i];
        Collider* colliderA = pair.first;
        Collider* colliderB = pair.second;

        // レイヤーマスクでフィルタリング
        if ((colliderA->GetLayer() & colliderB->GetLayerMask()) != 0 ||
            (colliderB->GetLayer() & colliderA->GetLayerMask()) != 0)
        {
            continue; // 衝突しないように設定されている
        }

        ColliderInfo info;

#ifdef _DEBUG
        /*std::string layerA = CollisionLayerManager::GetInstance()->GetLayerStr(colliderA->GetLayer());
        std::string layerB = CollisionLayerManager::GetInstance()->GetLayerStr(colliderB->GetLayer());
        Debug::Log("Checking collision between " + layerA + " and " + layerB + "\n");
        std::string nameA = colliderA->GetName();
        std::string nameB = colliderB->GetName();*/
        //Debug::Log("Checking collision between " + nameA + " and " + nameB + "\n");
#endif // _DEBUG

        // CollisionDetectorを使用して衝突判定を実行
        if (CollisionDetector::DetectCollision(colliderA, colliderB, info))
        {
            // 衝突情報を保存
            CollisionPair collisionPair;
            collisionPair.colliderA = colliderA;
            collisionPair.colliderB = colliderB;
            collisionPair.info = info;
            collisionPairs_.push_back(collisionPair);

            // OnCollision呼び出し（直接実行）
            colliderA->OnCollision(colliderB, info);

            // 衝突情報を反転して相手側の呼び出し
            ColliderInfo reversedInfo = info;
            reversedInfo.contactNormal = -info.contactNormal;
            colliderB->OnCollision(colliderA, reversedInfo);

            // 各コライダーに現在の衝突を記録
            colliderA->AddCurrentCollision(colliderB, info);
            colliderB->AddCurrentCollision(colliderA, reversedInfo);

            //Debug::Log("Checking collision between\t" + nameA + "\tand " + nameB + "\tHit\n");
        }
        //else
            //Debug::Log( nameA + "\tand " + nameB + "\t----------\n");
    }
}

void CollisionManager::UnregisterCollider(Collider* _collider)
{
    if (_collider == nullptr)
        return;

    RemoveColliderImmediate(_collider);
}

void CollisionManager::RemoveColliderImmediate(Collider* _collider)
{
    // コライダーリストから削除
    auto it = std::find(colliders_.begin(), colliders_.end(), _collider);
    if (it != colliders_.end()) {
        colliders_.erase(it);
    }

    auto staticIt = std::find(staticColliders_.begin(), staticColliders_.end(), _collider);
    if (staticIt != staticColliders_.end()) {
        staticColliders_.erase(staticIt);
        spiralHashGrid_->RemoveCollider(_collider); // スパイラルハッシュグリッドからも削除
    }

    // 衝突ペアからも削除
    if (!collisionPairs_.empty()) {
        collisionPairs_.erase(
            std::remove_if(collisionPairs_.begin(), collisionPairs_.end(),
                [_collider](const CollisionPair& pair) {
                    return pair.colliderA == _collider || pair.colliderB == _collider;
                }),
            collisionPairs_.end());
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

    // 全ての静的コライダーの衝突状態を更新
    for (auto collider : staticColliders_)
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

    // 静的コライダーを描画
    for (auto collider : staticColliders_)
    {
        // コライダーが衝突中か判定
        bool isColliding = std::find(collidingColliders.begin(), collidingColliders.end(), collider) != collidingColliders.end();

        // 衝突状態に応じて色を設定
        if (isColliding)
        {
            // 衝突中は赤色
            LineDrawer::GetInstance()->SetColor({ 1.0f, 0.0f, 1.0f, 1.0f });
        }
        else
        {
            // 通常時は緑色
            LineDrawer::GetInstance()->SetColor({ 0.0f, 0.0f, 1.0f, 1.0f });
        }

        // コライダーを描画
        collider->Draw();
    }

    // 衝突点と法線を描画
    LineDrawer::GetInstance()->SetColor({ 1.0f , 1.0f , 1.0f , 1.0f });
    for (const auto& pair : collisionPairs_)
    {
        // 衝突点を描画
        Vector3 contactPoint = pair.info.contactPoint;
        Vector3 normalEnd = contactPoint + pair.info.contactNormal * 0.5f; // 法線の長さを適当に調整

        // 法線を描画
        LineDrawer::GetInstance()->RegisterPoint(contactPoint, normalEnd);
    }
}

void CollisionManager::ImGui([[maybe_unused]] bool* _open)
{
#ifdef _DEBUG
    ImGui::Begin("CollisionManager", _open);

    ImGui::PushID(this);

    // デバッグ描画の有効/無効を設定
    ImGui::Checkbox("Draw Colliders", &isDrawEnabled_);

    // 統計情報表示
    ImGui::Text("Registered Colliders: %d", colliderCount_); // 登録されたコライダーの数
    ImGui::Text("Static Colliders: %zu", staticColliders_.size()); // 静的コライダーの数
    ImGui::Text("Potential Collisions: %zu", potentialCollisions_.size()); // 衝突の可能性があるペアの数
    ImGui::Text("Active Collisions: %d", collisionPairCount_); // 現在の衝突ペアの数

    ImGui::Checkbox("Enable Broad Phase", &enableBroadPhase_);

    ImGui::PopID();

    ImGui::End();
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
    for (auto& collider : colliders_)
    {
        quadTree_->RegisterObj(collider);
    }

    potentialCollisions_.clear();

    std::list<Collider*> stack;

    if (!colliders_.empty())
        // QuadTreeを使用して衝突ペアを取得
        quadTree_->GetCollisionPair(0, potentialCollisions_, stack);

    for (auto dynamicCollider : colliders_)
    {
        // SpiralHashGridを使用して動的コライダーと衝突する可能性がある静的コライダーを取得
        std::vector<Collider*> nearbyStaticColliders = spiralHashGrid_->CheckCollision(dynamicCollider);

        for (auto staticCollider : nearbyStaticColliders)
        {
            // 重複チェック（同じペアが既に存在しないか）
            bool alreadyExists = false;
            for (const auto& existingPair : potentialCollisions_)
            {
                if ((existingPair.first == dynamicCollider && existingPair.second == staticCollider) ||
                    (existingPair.first == staticCollider && existingPair.second == dynamicCollider))
                {
                    alreadyExists = true;
                    break;
                }
            }

            if (!alreadyExists)
            {
                potentialCollisions_.emplace_back(dynamicCollider, staticCollider);
            }
        }
    }
}

CollisionManager::CollisionManager()
    : isDrawEnabled_(true)
{
#ifdef _DEBUG
    ImGuiDebugManager::GetInstance()->RegisterMenuItem("CollisionManager", [this](bool* _open) { ImGui(_open); });
#endif // DEBUG
}