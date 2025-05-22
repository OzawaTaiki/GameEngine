#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>
#include <mutex>


class Time_MT
{
public:

    // singleton instance
    static Time_MT* GetInstance();

    // 初期化
    // _updateFraqency : 更新頻度 (Hz)
    void Initialize(uint32_t _updateFraqency = 1000);

    // 終了処理
    void Finalize();


/// ゲッター セッター

    // デルタタイム取得
    // return : 前フレームからの経過時間 (秒)
    template<typename T>
    static T GetDeltaTime() { return static_cast<T>(GetInstance()->deltaTime_); }

    // 軌道からの経過時間取得
    // return : 総経過時間 (秒)
    static double GetCurrentTime();

    // 起動からの総経過時間取得
    // return : 総経過時間 (秒)
    static double GetTotalTime() { return GetInstance()->totalTime_; }

    // フレームレート取得
    // return : フレームレート (FPS)
    static double GetFramerate() { return GetInstance()->framerate_; }

    // デルタタイムを固定値にするかどうかを設定
    // _isFixed : trueの場合、デルタタイムを固定値にする
    static void SetDeltaTimeFixed(bool _isFixed) { GetInstance()->isDeltaTimeFixed_ = _isFixed; }

    // デフォルトのフレームレートを設定
    // _framerate : フレームレート (FPS)
    static void SetDefaultFramerate(float _framerate) { GetInstance()->defaultFramerate_ = _framerate; }

    // デルタタイムが固定値かどうかを取得
    // return : trueの場合、デルタタイムは固定値
    static bool IsDeltaTimeFixed() { return GetInstance()->isDeltaTimeFixed_; }

    // スレッドがアクティブかどうかを取得
    // return : trueの場合、スレッドはアクティブ
    static bool IsTimerThreadActive() { return GetInstance()->isTimerThreadActive_; }

private:

    // スレッド関数
    void TimeThreadFunc();

    // 時間取得
    double GetHighResTime();

private:

    // スレッド関連
    std::thread timerThread_;
    std::atomic<bool> isTimerThreadActive_;
    std::mutex mutex_;
    std::condition_variable cv_;


    // 時間関連
    std::atomic<double> deltaTime_;
    std::atomic<double> totalTime_;
    std::atomic<double> prevTime_;
    std::atomic<double> currentTime_;
    std::atomic<double> framerate_;


    float defaultFramerate_;
    std::atomic<bool> isDeltaTimeFixed_;

    // 更新頻度関連
    uint32_t updateFrequency_;
    std::chrono::microseconds updateInterval_;

private:
    Time_MT();
    ~Time_MT() = default;

public:
    Time_MT(const Time_MT&) = delete;
    Time_MT& operator=(const Time_MT&) = delete;
    Time_MT(Time_MT&&) = delete;
    Time_MT& operator=(Time_MT&&) = delete;
};