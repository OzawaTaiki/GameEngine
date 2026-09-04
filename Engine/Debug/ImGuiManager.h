#pragma once

#include <cstdint>

#ifdef _DEBUG
#include <imgui.h>
#endif // _DEBUG


namespace Engine {

class SRVManager;
class ImGuiManager
{
public:

    void Initialize();
    void Begin();
    void End();
    void Draw();
    void Finalize();

    /// <summary> 画面全体に敷いているドックスペースのID </summary>
    uint32_t GetDockSpaceID() const { return dockSpaceID_; }

private:
    // DockSpaceOverViewport() が返すID。1フレーム目は 0
    uint32_t dockSpaceID_ = 0;

#ifdef _DEBUG
    SRVManager* srvManager_ = nullptr;
#endif // _DEBUG

};

} // namespace Engine
