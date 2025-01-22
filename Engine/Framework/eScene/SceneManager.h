#pragma once
#include "BaseScene.h"
#include <ISceneFactory.h>

#include <iostream>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>


class SceneManager
{
public:
    static SceneManager* GetInstance();

    ~SceneManager();

    // シーンの登録
    // _name : シーンの名前
    // _scene : シーンの生成関数
    // 例 : SceneManager::RegisterScene("game", Game::Create);
    //static void RegisterScene(const std::string& _name, SceneFactory _scene);

    // シーンファクトリの設定
    void SetSceneFactory(ISceneFactory* _sceneFactory);

    // 初期化
    // _name : 初期化するシーンの名前
    void Initialize(const std::string& _name);
    // 更新
    void Update();
    // 描画
    void Draw();

    // シーンの予約
    // _name : 予約するシーンの名前
    static void ReserveScene(const std::string& _name);

    // シーンの変更
    static void ChangeScene();

private:
    ISceneFactory* sceneFactory_ = nullptr;

    // シーンのリスト
    //std::unordered_map<std::string, SceneFactory>   scenes_ = {};
    //SceneFactory nextScene_ = nullptr;

    // 現在のシーン
    std::unique_ptr<BaseScene> currentScene_ = nullptr;

    // 現在のシーン名
    std::string currentSceneName_ = {};
    // 次のシーン名
    std::string nextSceneName_ = {};

#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG


    SceneManager() = default;
    // コピー禁止
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;
    SceneManager(SceneManager&&) = delete;
    SceneManager& operator=(SceneManager&&) = delete;

};
