#pragma once

#include <Features/Collision/Collider/Collider.h>
#include <Features/Collision/Detector/CollisionDetector.h>
#include <Features/Collision/Tree/QuadTree.h>
#include <Features/Collision/SpiralHashGird/SpiralHashGrid.h>

#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <thread>
#include <queue>
#include <atomic>

// OnCollision呼び出し情報を保存する構造体
struct CollisionCallInfo
{
    Collider* caller;
    Collider* other;
    ColliderInfo info;
};

// 衝突判定の管理を行うクラス（完全スレッドセーフ版）
class CollisionManager
{
public:
    // インスタンスを取得する
    static CollisionManager* GetInstance();

    // 初期化
    void Initialize(const Vector2& _fieldSize, uint32_t _level, const Vector2& _leftBottom, float _gridSize = 1.0f);

    // 終了処理
    void Finalize();

    // 毎フレームの更新
    void Update();

    // コライダーを登録する（高速版 - 衝突判定中は無視）
    void RegisterCollider(Collider* _collider);

    // コライダーを登録する（静的コライダー用 地形など)
    void RegisterStaticCollider(Collider* _collider);

    // 衝突判定を実行する（マルチスレッド対応）
    void CheckCollisions();

    // 衝突応答を実行する
    void ResolveCollisions();

    // 衝突状態を更新する
    void UpdateCollisionStates();

    // コライダーを描画する
    void DrawColliders();

    // デバッグ描画の有効/無効を設定
    void SetDrawEnabled(bool _enabled) { isDrawEnabled_.store(_enabled); }

    // デバッグ描画の有効/無効を取得
    bool IsDrawEnabled() const { return isDrawEnabled_.load(); }

    // コライダーの登録解除（安全版 - 遅延削除対応）
    void UnregisterCollider(Collider* _collider);

    // スレッド数を設定（デフォルトはハードウェア並行性）
    void SetThreadCount(uint32_t _threadCount) { threadCount_ = _threadCount; }

    // デバッグUI
    void ImGui();

private:
    // 衝突応答を実行する
    void ResolveCollision(const CollisionPair& _pair);

    // 空間分割などの最適化のためのユーティリティ
    void UpdateBroadPhase();

    // スレッドで実行される衝突判定処理
    void CheckCollisionsRange(size_t _startIndex, size_t _endIndex);

    // OnCollisionキューを処理する
    void ProcessCollisionCallbacks();

    // 遅延削除処理
    void ProcessPendingUnregistrations();

    // 即座削除の内部メソッド
    void RemoveColliderImmediate(Collider* _collider);

private:
    // コライダーのリスト
    std::vector<Collider*> colliders_;

    std::vector<Collider*> staticColliders_; // 静的コライダーのリスト

    // 衝突ペアのリスト
    std::vector<CollisionPair> collisionPairs_;

    // デバッグ描画の有効/無効
    std::atomic<bool> isDrawEnabled_;

    // 衝突ペアのリスト（衝突する可能性があるpair）
    std::vector<std::pair<Collider*, Collider*>> potentialCollisions_;

    std::unique_ptr<QuadTree> quadTree_; // 空間分割のための四分木

    std::unique_ptr<SpiralHashGrid> spiralHashGrid_; // スパイラルハッシュグリッド

    // マルチスレッド関連
    uint32_t threadCount_;
    std::mutex collisionPairsMutex_;      // collisionPairs_への書き込み保護
    std::mutex collisionCallQueueMutex_;  // OnCollision呼び出しキューの保護
    std::queue<CollisionCallInfo> collisionCallQueue_; // OnCollision呼び出し情報のキュー

    // フェーズ管理
    std::atomic<bool> isCollisionPhase_{ false };  // 衝突判定フェーズ中かどうか

    // 遅延削除システム
    std::mutex pendingUnregisterMutex_;          // 削除待ちキューの保護
    std::vector<Collider*> pendingUnregister_;   // 削除待ちのコライダー


private:
    // コンストラクタ
    CollisionManager();

    // デストラクタ
    ~CollisionManager() = default;

    // コピーコンストラクタとコピー代入演算子を禁止
    CollisionManager(const CollisionManager&) = delete;
    CollisionManager& operator=(const CollisionManager&) = delete;
};