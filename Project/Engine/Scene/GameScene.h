#pragma once
#include "Scene.h"
#include "Camera.h"
#include "Input.h"
#include "LineDrawer.h"
#include "WorldTransform.h"
#include "Model.h"
#include "ObjectColor.h"
#include "DebugCamera.h"
#include "Player.h"
#include <memory>
#include "CatmulRomSpline.h"
#include "Enemy.h"
#include "RailCamera.h"
#include "SkyDome.h"

class GameScene : public Scene
{
public:
    ~GameScene();

    void Initialize();
    void Update();
    void Draw();


private:
    Input*                              input_          = nullptr;
    LineDrawer*                         lineDrawer      = nullptr;
    std::unique_ptr<Camera>             camera_         = nullptr;  //描画用カメラ
    std::unique_ptr<DebugCamera>        DebugCamera_    = nullptr;
    bool                                useDebugCamera_ = false;
    Model*                              tile_           = nullptr;
    WorldTransform                      trans_          = {};

    std::unique_ptr<SkyDome>            skyDome_        = nullptr;
    std::unique_ptr<Player>             player_         = nullptr;
    std::unique_ptr<RailCamera>         railCamera_     = nullptr;
    std::unique_ptr<CatmulRomSpline>    edit_           = nullptr;
};