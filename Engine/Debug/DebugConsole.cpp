#include <Debug/DebugConsole.h>

#ifdef _DEBUG
#include <System/Time/Time.h>
#include <format>
#endif // _DEBUG


namespace Engine {

namespace {

#ifdef _DEBUG
// レベルごとの表示色
const ImVec4 kLevelColors[static_cast<size_t>(LogLevel::Count)] =
{
    ImVec4(0.80f, 0.80f, 0.80f, 1.00f),  // Log
    ImVec4(1.00f, 0.80f, 0.30f, 1.00f),  // Warning
    ImVec4(1.00f, 0.40f, 0.40f, 1.00f),  // Error
};

const char* kLevelLabels[static_cast<size_t>(LogLevel::Count)] =
{
    "Log", "Warning", "Error"
};
#endif // _DEBUG

} // namespace

DebugConsole* DebugConsole::GetInstance()
{
    static DebugConsole instance;
    return &instance;
}

void DebugConsole::AddLog([[maybe_unused]] LogLevel _level, [[maybe_unused]] const std::string& _message)
{
#ifdef _DEBUG

    std::lock_guard<std::mutex> lock(mutex_);

    Entry entry;
    entry.level = _level;
    entry.message = _message;
    entry.time = Time::GetTotalTime();
    entry.frame = frameCounter_;

    // 末尾の改行は表示時に不要なので落とす
    while (!entry.message.empty() &&
           (entry.message.back() == '\n' || entry.message.back() == '\r'))
    {
        entry.message.pop_back();
    }

    entries_.push_back(std::move(entry));
    ++counts_[static_cast<size_t>(_level)];

    while (entries_.size() > kMaxEntries_)
    {
        --counts_[static_cast<size_t>(entries_.front().level)];
        entries_.pop_front();
    }

#endif // _DEBUG
}

void DebugConsole::Clear()
{
    std::lock_guard<std::mutex> lock(mutex_);

    entries_.clear();
    for (auto& count : counts_)
        count = 0;
}

uint32_t DebugConsole::GetCount(LogLevel _level) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return counts_[static_cast<size_t>(_level)];
}

void DebugConsole::Show([[maybe_unused]] bool* _open)
{
#ifdef _DEBUG

    ++frameCounter_;

    if (!ImGui::Begin("Console", _open))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Clear"))
    {
        ImGui::End();
        Clear();
        return;
    }

    ImGui::SameLine();
    ImGui::Checkbox("Auto Scroll", &autoScroll_);
    ImGui::SameLine();
    ImGui::Checkbox("Time", &showTime_);

    // レベルごとの表示切り替え。ラベルに件数を出す
    for (size_t i = 0; i < static_cast<size_t>(LogLevel::Count); ++i)
    {
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, kLevelColors[i]);
        std::string label = std::string(kLevelLabels[i]) + " " + std::to_string(counts_[i]);
        ImGui::Checkbox(label.c_str(), &showLevel_[i]);
        ImGui::PopStyleColor();
    }

    filter_.Draw("Filter", 180.0f);

    ImGui::Separator();

    if (ImGui::BeginChild("ConsoleScroll", ImVec2(0.0f, 0.0f),
                          ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
    {
        std::lock_guard<std::mutex> lock(mutex_);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 1.0f));

        for (const auto& entry : entries_)
        {
            if (!showLevel_[static_cast<size_t>(entry.level)])
                continue;

            if (!filter_.PassFilter(entry.message.c_str()))
                continue;

            ImGui::PushStyleColor(ImGuiCol_Text, kLevelColors[static_cast<size_t>(entry.level)]);

            if (showTime_)
                ImGui::TextUnformatted(std::format("[{:8.2f}] {}", entry.time, entry.message).c_str());
            else
                ImGui::TextUnformatted(entry.message.c_str());

            ImGui::PopStyleColor();
        }

        ImGui::PopStyleVar();

        // 末尾にいるときだけ追従する
        if (autoScroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();

    ImGui::End();

#endif // _DEBUG
}

} // namespace Engine
