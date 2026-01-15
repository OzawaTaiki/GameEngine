#include "Time_MT.h"


namespace Engine {

Time_MT* Time_MT::GetInstance()
{
    static Time_MT instance;
    return &instance;
}

void Time_MT::Initialize(uint32_t _updateFrequency)
{
    // すでに初期化済みの場合は何もしない
    if (isTimerThreadActive_)
        return;


    // 更新頻度の設定
    updateFrequency_ = _updateFrequency;
    updateInterval_ = std::chrono::microseconds(1000000 / updateFrequency_);

    // 初期時間の設定
    currentTime_.store(GetHighResTime());
    prevTime_.store(currentTime_.load());
    totalTime_.store(0.0);
    deltaTime_.store(1.0 / defaultFramerate_);


    // タイマースレッドの開始
    isTimerThreadActive_ = true;
    timerThread_ = std::thread(&Time_MT::TimeThreadFunc, this);

}

void Time_MT::Finalize()
{
    // スレッドが動作中なら停止する
    if (isTimerThreadActive_)
    {
        isTimerThreadActive_ = false;

        // 条件変数で待機中のスレッドを解放
        cv_.notify_one();

        // スレッドの終了を待つ
        if (timerThread_.joinable())
        {
            timerThread_.join();
        }
    }
}

double Time_MT::GetCurrentTime()
{
    return GetInstance()->GetHighResTime();
}

void Time_MT::TimeThreadFunc()
{
    using namespace std::chrono;

    // スレッド開始時間
    auto lastUpdateTime = high_resolution_clock::now();

    while (isTimerThreadActive_)
    {
        // 指定された更新間隔で実行
        auto targetTime = lastUpdateTime + updateInterval_;

        // 次の更新時間まで待機
        std::this_thread::sleep_until(targetTime);

        // 実際の時間を取得
        auto now = high_resolution_clock::now();
        lastUpdateTime = now;

        // 現在時間を更新
        currentTime_.store(GetHighResTime());

        // デルタタイムの計算
        double newDeltaTime;
        if (isDeltaTimeFixed_)
        {
            // 固定デルタタイム
            newDeltaTime = 1.0 / defaultFramerate_;
        }
        else
        {
            // 可変デルタタイム
            newDeltaTime = currentTime_.load() - prevTime_.load();

            // 異常値のチェック
            if (newDeltaTime <= 0.0 || newDeltaTime > 0.1)
            {
                newDeltaTime = 1.0 / defaultFramerate_;
            }
        }

        // 値の更新（アトミック変数で安全に更新）
        deltaTime_.store(newDeltaTime);
        totalTime_.store(totalTime_.load() + newDeltaTime);
        prevTime_.store(currentTime_.load());
        framerate_.store(1.0 / newDeltaTime);
    }
}

double Time_MT::GetHighResTime()
{
    using namespace std::chrono;

    // 高精度クロックで現在時間を取得
    auto now = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(now.time_since_epoch()).count();

    // ナノ秒を秒に変換
    return static_cast<double>(duration) / 1.0e9;
}

Time_MT::Time_MT():
    updateFrequency_(60),
    defaultFramerate_(60.0f),
    isTimerThreadActive_(false),
    isDeltaTimeFixed_(false)
{
}

} // namespace Engine
