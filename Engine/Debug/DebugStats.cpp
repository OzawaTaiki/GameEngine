#include <Debug/DebugStats.h>

#ifdef _DEBUG

#include <Core/WinApp/WinApp.h>
#include <Debug/GameViewportWindow.h>
#include <System/Time/Time.h>

#include <imgui.h>

#include <algorithm>
#include <numeric>

#endif // _DEBUG


namespace Engine {

DebugStats* DebugStats::GetInstance()
{
    static DebugStats instance;
    return &instance;
}

void DebugStats::Update()
{
#ifdef _DEBUG

    // ImGuiのDeltaTimeは実時間。Time::GetDeltaTime() は固定値の場合があるのでこちらを使う
    const float frameTime = ImGui::GetIO().DeltaTime * 1000.0f;

    frameTimeHistory_[historyOffset_] = frameTime;
    historyOffset_ = (historyOffset_ + 1) % kHistorySize_;

    smoothedFrameTime_ = smoothedFrameTime_ * 0.9f + frameTime * 0.1f;

    const auto [minIt, maxIt] = std::minmax_element(frameTimeHistory_.begin(), frameTimeHistory_.end());
    minFrameTime_ = *minIt;
    maxFrameTime_ = *maxIt;
    averageFrameTime_ = std::accumulate(frameTimeHistory_.begin(), frameTimeHistory_.end(), 0.0f)
        / static_cast<float>(kHistorySize_);

#endif // _DEBUG
}

void DebugStats::Show([[maybe_unused]] bool* _open)
{
#ifdef _DEBUG

    if (!ImGui::Begin("Stats", _open))
    {
        ImGui::End();
        return;
    }

    const ImGuiIO& io = ImGui::GetIO();

    ImGui::SeparatorText("Frame");

    ImGui::Text("FPS          : %6.1f", smoothedFrameTime_ > 0.0f ? 1000.0f / smoothedFrameTime_ : 0.0f);
    ImGui::Text("Frame Time   : %6.2f ms", smoothedFrameTime_);
    ImGui::Text("min / avg / max : %.2f / %.2f / %.2f ms",
                minFrameTime_, averageFrameTime_, maxFrameTime_);

    // 上限は直近の最大値に合わせる。潰れて見えないように下限を設ける
    const float plotMax = (std::max)(maxFrameTime_ * 1.1f, 20.0f);
    ImGui::PlotLines("##FrameTime", frameTimeHistory_.data(),
                     static_cast<int>(kHistorySize_), static_cast<int>(historyOffset_),
                     nullptr, 0.0f, plotMax, ImVec2(0.0f, 60.0f));

    ImGui::SeparatorText("Game Time");

    ImGui::Text("Delta Time   : %6.2f ms %s",
                Time::GetDeltaTime<float>() * 1000.0f,
                Time::IsDeltaTimeFixed() ? "(fixed)" : "");
    ImGui::Text("Total Time   : %8.2f s", Time::GetTotalTime());
    ImGui::Text("Framerate    : %6.1f", Time::GetFramerate());

    ImGui::SeparatorText("Screen");

    ImGui::Text("Game         : %u x %u", WinApp::kWindowWidth_, WinApp::kWindowHeight_);

    const GameViewportWindow* viewport = GameViewportWindow::GetInstance();
    const Vector2 viewportSize = viewport->GetImageSize();
    ImGui::Text("Viewport     : %.0f x %.0f%s",
                viewportSize.x, viewportSize.y,
                viewport->IsEnabled() ? "" : " (fullscreen)");
    if (viewportSize.x > 0.0f)
        ImGui::Text("Scale        : %.3f", viewportSize.x / WinApp::kWindowSize_.x);

    ImGui::SeparatorText("ImGui");

    ImGui::Text("Vertices     : %d", io.MetricsRenderVertices);
    ImGui::Text("Indices      : %d", io.MetricsRenderIndices);
    ImGui::Text("Windows      : %d (visible %d)", io.MetricsRenderWindows, io.MetricsActiveWindows);

    ImGui::End();

#endif // _DEBUG
}

} // namespace Engine
