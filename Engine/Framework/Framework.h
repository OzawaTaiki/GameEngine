#pragma once

#include <Core/WinApp/WinApp.h>
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Debug/ImGuiManager.h>
#include <Features/Effect/Manager/ParticleSystem.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <System/Input/Input.h>
#include <Features/Scene/Manager/SceneManager.h>
#include <Features/Collision/Manager/CollisionManager.h>
#include <Core/DXCommon/RTV/RTVManager.h>
#include <System/Audio/Audio.h>
#include <System/Time/Time_MT.h>
#include <Features/TextRenderer/TextRenderer.h>

class Framework
{
public:

    void Run();

    virtual ~Framework() = default;

protected:

    virtual void Initialize(const std::wstring& _winTitle);

    virtual void Update();

    void PreDraw();

    void PostDraw();

    virtual void Draw() = 0;

    virtual void Finalize();

    virtual bool IsEndRequested() const { return endRequest_; }

    WinApp*                 winApp_                 = nullptr;//
    DXCommon*               dxCommon_               = nullptr;//
    SRVManager*             srvManager_             = nullptr;//
    RTVManager*             rtvManager_             = nullptr;//
    ImGuiManager*           imguiManager_           = nullptr;//
    ParticleSystem*        particleManager_        = nullptr;//
    LineDrawer*             lineDrawer_             = nullptr;//
    Input*                  input_                  = nullptr;//
    SceneManager*           sceneManager_           = nullptr;//
    CollisionManager*       collisionManager_       = nullptr;//
    GameTime*               gameTime_               = nullptr;//
    Audio* audio_ = nullptr;//
    TextRenderer* textRenderer_ = nullptr;//





    bool endRequest_ = false;


};
