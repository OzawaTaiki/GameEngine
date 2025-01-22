#pragma once

#include <Core/WinApp/WinApp.h>
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Debug/ImGuiManager.h>
#include <Features/Effect/Manager/ParticleManager.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <System/Input/Input.h>
#include <Features/Scene/Manager/SceneManager.h>
#include <Features/Collision/Manager/CollisionManager.h>

class Framework
{
public:

    void Run();

    virtual ~Framework() = default;

protected:

    virtual void Initialize();

    virtual void Update();

    void PreDraw();

    void PostDraw();

    virtual void Draw() = 0;

    virtual void Finalize();

    virtual bool IsEndRequested() const { return endRequest_; }

    WinApp*                 winApp_                 = nullptr;//
    DXCommon*               dxCommon_               = nullptr;//
    SRVManager*             srvManager_             = nullptr;//
    ImGuiManager*           imguiManager_           = nullptr;//
    ParticleManager*        particleManager_        = nullptr;//
    LineDrawer*             lineDrawer_             = nullptr;//
    Input*                  input_                  = nullptr;//
    SceneManager*           sceneManager_           = nullptr;//
    CollisionManager*       collisionManager_       = nullptr;//


    bool endRequest_ = false;


};
