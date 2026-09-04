#pragma once

#include <cstdint>
#include <deque>
#include <mutex>
#include <string>

#ifdef _DEBUG
#include <imgui.h>
#endif // _DEBUG


namespace Engine {

enum class LogLevel
{
    Log = 0,
    Warning,
    Error,

    Count
};

/// <summary>
/// Debug::Log の出力を蓄積して表示するコンソールウィンドウ
/// </summary>
class DebugConsole
{
public:

    static DebugConsole* GetInstance();

    /// <summary>
    /// ログを追加する
    /// Debug::Log から呼ばれる。ワーカースレッドから呼ばれても良い
    /// </summary>
    void AddLog(LogLevel _level, const std::string& _message);

    /// <summary> 蓄積したログをすべて破棄する </summary>
    void Clear();

    /// <summary> コンソールウィンドウを表示する </summary>
    void Show(bool* _open);

    /// <summary> レベルごとの件数 </summary>
    uint32_t GetCount(LogLevel _level) const;

private:

    struct Entry
    {
        LogLevel level = LogLevel::Log;
        std::string message;
        double time = 0.0;      // 起動からの経過時間
        int32_t frame = 0;      // 追加されたフレーム
    };

    // 保持する最大件数。超えた分は古いものから捨てる
    static constexpr size_t kMaxEntries_ = 4096;

    std::deque<Entry> entries_;
    mutable std::mutex mutex_;

    uint32_t counts_[static_cast<size_t>(LogLevel::Count)] = {};

    // 表示するレベル
    bool showLevel_[static_cast<size_t>(LogLevel::Count)] = { true,true,true };
    bool autoScroll_ = true;
    bool showTime_ = false;

    int32_t frameCounter_ = 0;

#ifdef _DEBUG
    ImGuiTextFilter filter_;
#endif // _DEBUG

private:
    DebugConsole() = default;
    ~DebugConsole() = default;
    // コピー禁止
    DebugConsole(const DebugConsole&) = delete;
    DebugConsole& operator=(const DebugConsole&) = delete;

};

} // namespace Engine
