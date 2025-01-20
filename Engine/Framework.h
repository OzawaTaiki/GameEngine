#pragma once

#include <Core/WinApp/WinApp.h>
#include <Core/DirectX/DXCommon.h>
#include <ResourceManagement/SRVManager.h>
#include <UI/ImGuiManager/ImGuiManager.h>
#include <Framework/Particle/ParticleManager.h>
#include <Rendering/LineDrawer/LineDrawer.h>
#include <Systems/Input/Input.h>
#include <Framework/eScene/SceneManager.h>
#include <Physics/Collision/CollisionManager.h>

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