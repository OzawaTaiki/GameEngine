#include "Stopwatch.h"

#include <Debug/ImGuiManager.h>


namespace Engine {

Stopwatch::Stopwatch(bool _useGameTime, const std::string& _channelName) :
    useGameTime_(_useGameTime),
    channelName_(_channelName),
    elapsedTime_(0.0),
    isRunning_(false)
{
}

void Stopwatch::Start()
{
    isRunning_ = true;
    elapsedTime_ = 0.0;
}

void Stopwatch::Stop()
{
    isRunning_ = false;
}

void Stopwatch::Resume()
{
    isRunning_ = true;
}

void Stopwatch::Reset()
{
    elapsedTime_ = 0.0;
}

void Stopwatch::Update()
{
    if (!isRunning_)
        return;

    if (useGameTime_)
    {
        // 指定されたチャンネルのデルタタイムを加算
        elapsedTime_ += GameTime::GetChannel(channelName_).GetDeltaTime<double>();
    }
    else
    {
        elapsedTime_ += GameTime::GetInstance()->GetDeltaTime();
    }
}

void Stopwatch::ShowDebugWindow()
{
#ifdef _DEBUG

    ImGui::PushID(this);

    if (ImGui::Button("Start"))
    {
        Start();
    }
    if (ImGui::Button("Stop"))
    {
        Stop();
    }
    if (ImGui::Button("Resume"))
    {
        Resume();
    }
    if (ImGui::Button("Reset"))
    {
        Reset();
    }

    ImGui::Text("Elapsed Time: %.2f", elapsedTime_);

    ImGui::PopID();

#endif // _DEBUG
}

} // namespace Engine
