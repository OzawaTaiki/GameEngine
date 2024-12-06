#pragma once
#ifdef _DEBUG
#include <imgui.h>
#endif // _DEBUG

class SRVManager;
class ImGuiManager
{
public:

    void Initialize();
    void Begin();
    void End();
    void Draw();
    void Finalize();

private:
#ifdef _DEBUG
    SRVManager* srvManager_ = nullptr;
#endif // _DEBUG

};