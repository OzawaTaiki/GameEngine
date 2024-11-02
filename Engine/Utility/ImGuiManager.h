#pragma once
#include <imgui.h>

class SRVManager;
class ImGuiManager
{
public:

    void Initialize(SRVManager* _ptr);
    void Begin();
    void End();
    void Draw();
    void Finalize();

private:
    SRVManager* srvManager_ = nullptr;


};