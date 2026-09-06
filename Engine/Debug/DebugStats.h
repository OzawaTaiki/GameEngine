#pragma once

#include <array>
#include <cstdint>


namespace Engine {

/// <summary>
/// フレームレートなどの実行時情報を表示するウィンドウ
/// </summary>
class DebugStats
{
public:

    static DebugStats* GetInstance();

    /// <summary> 毎フレーム呼んで計測値を積む </summary>
    void Update();

    /// <summary> ウィンドウを表示する </summary>
    void Show(bool* _open);

private:

    // フレームタイムの履歴(ミリ秒)
    static constexpr size_t kHistorySize_ = 180;

    std::array<float, kHistorySize_> frameTimeHistory_ = {};
    size_t historyOffset_ = 0;

    // 表示のちらつきを抑えるための平滑化した値
    float smoothedFrameTime_ = 1000.0f / 60.0f;

    float minFrameTime_ = 0.0f;
    float maxFrameTime_ = 0.0f;
    float averageFrameTime_ = 0.0f;

private:
    DebugStats() { frameTimeHistory_.fill(1000.0f / 60.0f); }
    ~DebugStats() = default;
    // コピー禁止
    DebugStats(const DebugStats&) = delete;
    DebugStats& operator=(const DebugStats&) = delete;

};

} // namespace Engine
