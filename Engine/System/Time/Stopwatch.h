#pragma once

#include <System/Time/GameTime.h>


class Stopwatch
{
public:

    // コンストラクタ
    Stopwatch(bool _useGameTime = true, const std::string& _channelName = "default");
    // デストラクタ
    ~Stopwatch() = default;

    /// <summary>
    /// 経過時間をリセットしてスタートします。
    /// </summary>
    void Start();

    /// <summary>
    /// 計測を停止する（経過時間はそのまま）
    /// </summary>
    void Stop();

    /// <summary>
    /// 計測を再開する
    /// </summary>
    void Resume();

    /// <summary>
    /// 経過時間をリセットします。
    /// </summary>
    void Reset();


    /// <summary>
    /// 経過時間を更新します。
    /// </summary>
    void Update();

    /// <summary>
    ///
    /// </summary>
    template<typename T>
    T GetElapsedTime() const { return static_cast<T>(elapsedTime_); }
    


    float GetElapsedTime() const;

private:

    bool useGameTime_;          // ゲーム時間を使用するか
    std::string channelName_;   // 使用するチャンネル名
    double elapsedTime_;        // 経過時間
    bool isRunning_;            // 計測中かどうか

};
