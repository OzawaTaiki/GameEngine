#include "CollisionManager.h"
#include <Features/LineDrawer/LineDrawer.h>
#include <Debug/ImGuiDebugManager.h>
#include <Features/Collision/CollisionLayer/CollisionLayerManager.h>
#include <algorithm>
#include <future>
#include <chrono>

CollisionManager* CollisionManager::GetInstance()
{
    static CollisionManager instance;
    return &instance;
}

void CollisionManager::Initialize(const Vector2& _fieldSize, uint32_t _level, const Vector2& _leftBotom, float _gridSize)
{
    colliders_.clear();
    collisionPairs_.clear();
    isDrawEnabled_.store(true);
    isCollisionPhase_.store(false);

    // OnCollision呼び出しキューをクリア
    {
        std::lock_guard<std::mutex> lock(collisionCallQueueMutex_);
        while (!collisionCallQueue_.empty()) {
            collisionCallQueue_.pop();
        }
    }

    // 遅延削除キューをクリア
    {
        std::lock_guard<std::mutex> lock(pendingUnregisterMutex_);
        pendingUnregister_.clear();
    }

    // QuadTreeの初期化
    quadTree_ = std::make_unique<QuadTree>();
    quadTree_->Initialize(_fieldSize, _level, _leftBotom);

    // スパイラルハッシュグリッドの初期化
    spiralHashGrid_ = std::make_unique<SpiralHashGrid>(_gridSize);
    spiralHashGrid_->Clear();
}

void CollisionManager::Finalize()
{
    // 衝突判定フェーズを終了
    isCollisionPhase_.store(false);

    colliders_.clear();
    collisionPairs_.clear();

    // OnCollision呼び出しキューをクリア
    {
        std::lock_guard<std::mutex> lock(collisionCallQueueMutex_);
        while (!collisionCallQueue_.empty()) {
            collisionCallQueue_.pop();
        }
    }

    // 遅延削除キューをクリア
    {
        std::lock_guard<std::mutex> lock(pendingUnregisterMutex_);
        pendingUnregister_.clear();
    }
}

void CollisionManager::Update()
{
    // 遅延削除の処理（衝突判定前）
    ProcessPendingUnregistrations();

    // 衝突判定フェーズ開始
    isCollisionPhase_.store(true);

    // 衝突判定を実行（マルチスレッド）
    CheckCollisions();

    // 衝突判定フェーズ終了
    isCollisionPhase_.store(false);

    // キューに溜まったOnCollision呼び出しを処理
    ProcessCollisionCallbacks();

    // 衝突応答を実行
    //ResolveCollisions();

    // 衝突状態を更新
    UpdateCollisionStates();

    // デバッグ描画が有効なら描画
    if (isDrawEnabled_.load())
    {
        DrawColliders();
    }

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

    // 衝突判定中の登録はスキップ（警告出力）
    if (isCollisionPhase_.load()) {
#ifdef _DEBUG
        // Debug::Log("Warning: RegisterCollider called during collision phase\n");
#endif
        return;
    }

    // 既に登録されているかチェック
    auto it = std::find(colliders_.begin(), colliders_.end(), _collider);
    if (it != colliders_.end())
        return;

    // コライダーを登録（高速 - ミューテックス不使用）
    colliders_.push_back(_collider);
}

void CollisionManager::RegisterStaticCollider(Collider* _collider)
{
    // nullチェック
    if (_collider == nullptr)
        return;
    // 衝突判定中の登録はスキップ（警告出力）
    if (isCollisionPhase_.load()) {
#ifdef _DEBUG
        // Debug::Log("Warning: RegisterStaticCollider called during collision phase\n");
#endif
        return;
    }

    // 既に登録されているかチェック
    auto it = std::find(staticColliders_.begin(), staticColliders_.end(), _collider);
    if (it != staticColliders_.end())
        return;

    // コライダーを登録（高速 - ミューテックス不使用）
    staticColliders_.push_back(_collider);

    spiralHashGrid_->AddCollider(_collider);

}

void CollisionManager::CheckCollisions()
{
    auto now = std::chrono::high_resolution_clock::now();

    // ブロードフェーズの更新（空間分割等の最適化）
    UpdateBroadPhase();

    // 判定対象がない場合は早期リターン
    if (potentialCollisions_.empty())
        return;

    // スレッド数を決定
    const size_t totalPairs = potentialCollisions_.size();
    const size_t numThreads = (std::min)(static_cast<size_t>(threadCount_), totalPairs);

    if (numThreads <= 1)
    {
        // シングルスレッドで処理
        CheckCollisionsRange(0, totalPairs);
    }
    else
    {
        // マルチスレッドで処理
        std::vector<std::future<void>> futures;
        const size_t pairsPerThread = totalPairs / numThreads;
        const size_t remainingPairs = totalPairs % numThreads;

        for (size_t i = 0; i < numThreads; ++i)
        {
            size_t startIndex = i * pairsPerThread;
            size_t endIndex = startIndex + pairsPerThread;

            // 最後のスレッドに余りを割り当て
            if (i == numThreads - 1)
            {
                endIndex += remainingPairs;
            }

            futures.push_back(std::async(std::launch::async,
                [this, startIndex, endIndex]() {
                    CheckCollisionsRange(startIndex, endIndex);
                }));
        }

        // 全てのスレッドの完了を待機
        for (auto& future : futures)
        {
            future.wait();
        }
    }
}

void CollisionManager::CheckCollisionsRange(size_t _startIndex, size_t _endIndex)
{
    std::vector<CollisionPair> localCollisionPairs;
    std::vector<CollisionCallInfo> localCallQueue;

    for (size_t i = _startIndex; i < _endIndex; ++i)
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
            // 衝突情報を保存（ローカル）
            CollisionPair collisionPair;
            collisionPair.colliderA = colliderA;
            collisionPair.colliderB = colliderB;
            collisionPair.info = info;
            localCollisionPairs.push_back(collisionPair);

            // OnCollision呼び出し情報をローカルキューに追加
            CollisionCallInfo callInfoA;
            callInfoA.caller = colliderA;
            callInfoA.other = colliderB;
            callInfoA.info = info;
            localCallQueue.push_back(callInfoA);

            // 衝突情報を反転して相手側の呼び出し情報も追加
            CollisionCallInfo callInfoB;
            callInfoB.caller = colliderB;
            callInfoB.other = colliderA;
            callInfoB.info = info;
            callInfoB.info.contactNormal = -info.contactNormal;
            localCallQueue.push_back(callInfoB);

            // 各コライダーに現在の衝突を記録
            colliderA->AddCurrentCollision(colliderB, info);

            ColliderInfo reversedInfo = info;
            reversedInfo.contactNormal = -info.contactNormal;
            colliderB->AddCurrentCollision(colliderA, reversedInfo);

            //Debug::Log("Checking collision between\t" + nameA + "\tand " + nameB + "\tHit\n");
        }
        //else
            //Debug::Log( nameA + "\tand " + nameB + "\t----------\n");
    }

    // ローカルな結果をグローバルなコンテナに追加（ミューテックスで保護）
    if (!localCollisionPairs.empty())
    {
        std::lock_guard<std::mutex> lock(collisionPairsMutex_);
        collisionPairs_.insert(collisionPairs_.end(), localCollisionPairs.begin(), localCollisionPairs.end());
    }

    if (!localCallQueue.empty())
    {
        std::lock_guard<std::mutex> lock(collisionCallQueueMutex_);
        for (const auto& callInfo : localCallQueue)
        {
            collisionCallQueue_.push(callInfo);
        }
    }
}

void CollisionManager::ProcessCollisionCallbacks()
{
    std::lock_guard<std::mutex> lock(collisionCallQueueMutex_);

    while (!collisionCallQueue_.empty())
    {
        const auto& callInfo = collisionCallQueue_.front();
        callInfo.caller->OnCollision(callInfo.other, callInfo.info);
        collisionCallQueue_.pop();
    }
}

void CollisionManager::ProcessPendingUnregistrations()
{
    std::lock_guard<std::mutex> lock(pendingUnregisterMutex_);

    for (Collider* collider : pendingUnregister_) {
        RemoveColliderImmediate(collider);
    }

    pendingUnregister_.clear();
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
    if (!isDrawEnabled_.load())
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

void CollisionManager::UnregisterCollider(Collider* _collider)
{
    if (_collider == nullptr)
        return;

    if (isCollisionPhase_.load()) {
        // 衝突判定中の場合は削除を遅延
        std::lock_guard<std::mutex> lock(pendingUnregisterMutex_);

        // 重複チェック
        auto it = std::find(pendingUnregister_.begin(), pendingUnregister_.end(), _collider);
        if (it == pendingUnregister_.end()) {
            pendingUnregister_.push_back(_collider);
            return;
        }
    }

    // 通常の削除処理
    RemoveColliderImmediate(_collider);
}

void CollisionManager::ImGui()
{
#ifdef _DEBUG
    ImGui::PushID(this);

    // デバッグ描画の有効/無効を設定
    bool drawEnabled = isDrawEnabled_.load();
    if (ImGui::Checkbox("Draw Colliders", &drawEnabled))
    {
        isDrawEnabled_.store(drawEnabled);
    }

    // スレッド数の設定
    int threadCount = static_cast<int>(threadCount_);
    if (ImGui::SliderInt("Thread Count", &threadCount, 1, 16))
    {
        threadCount_ = static_cast<uint32_t>(threadCount);
    }

    // フェーズ状態表示
    ImGui::Text("Collision Phase: %s", isCollisionPhase_.load() ? "Active" : "Inactive");

    // 統計情報表示
    ImGui::Text("Registered Colliders: %zu", colliders_.size());
    ImGui::Text("Potential Collisions: %zu", potentialCollisions_.size());
    ImGui::Text("Active Collisions: %zu", collisionPairs_.size());

    // 遅延削除キューの状態
    {
        std::lock_guard<std::mutex> lock(pendingUnregisterMutex_);
        ImGui::Text("Pending Unregistrations: %zu", pendingUnregister_.size());
    }

    // OnCollisionキューの状態
    {
        std::lock_guard<std::mutex> lock(collisionCallQueueMutex_);
        ImGui::Text("Collision Callback Queue: %zu", collisionCallQueue_.size());
    }

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
    for (auto& collider : colliders_)
    {
        quadTree_->RegisterObj(collider);
    }

    potentialCollisions_.clear();

    std::list<Collider*> stac;

    // QuadTreeを使用して衝突ペアを取得
    quadTree_->GetCollisionPair(0, potentialCollisions_, stac);

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
    : threadCount_(std::thread::hardware_concurrency())
    , isDrawEnabled_(true)
    , isCollisionPhase_(false)
{
    // スレッド数が0の場合は1に設定
    if (threadCount_ == 0)
    {
        threadCount_ = 1;
    }

#ifdef _DEBUG
    ImGuiDebugManager::GetInstance()->AddColliderDebugWindow("CollisionManager", [&]() {ImGui(); });
#endif // DEBUG
}

#pragma region 既存のコード
/*
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
        std::string layerA = CollisionLayerManager::GetInstance()->GetLayerStr(colliderA->GetLayer());
        std::string layerB = CollisionLayerManager::GetInstance()->GetLayerStr(colliderB->GetLayer());
        Debug::Log("Checking collision between " + layerA + " and " + layerB + "\n");
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

            colliderA->OnCollision(colliderB, info);


            // 衝突情報を反転して相手側にも記録
            ColliderInfo reversedInfo = info;
            reversedInfo.contactNormal = -info.contactNormal;
            colliderB->OnCollision(colliderA, reversedInfo);

            // 各コライダーに現在の衝突を記録
            colliderA->AddCurrentCollision(colliderB, info);
            colliderB->AddCurrentCollision(colliderA, reversedInfo);


        }
    }
}
*/
#pragma endregion