#pragma once
#include <Features/Scene/Interface/BaseScene.h>
#include <Features/Scene/ISceneFactory.h>
#include <Features/Scene/Interface/ISceneTransition.h>
#include <Features/Scene/SceneData.h>

#include <iostream>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>
#include <queue>


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

    void DrawShadow();

    void SetTransition(std::unique_ptr<ISceneTransition> _transition);

    // シーンの予約
    // _name : 予約するシーンの名前
    static void ReserveScene(const std::string& _name, std::unique_ptr<SceneData> _sceneData);

    // シーンの変更
    static void ChangeScene();

    void Finalize();

private:
    ISceneFactory* sceneFactory_ = nullptr;

    // シーンのリスト
    //std::unordered_map<std::string, SceneFactory>   scenes_ = {};
    //SceneFactory nextScene_ = nullptr;

    // 現在のシーン
    std::unique_ptr<BaseScene> currentScene_ = nullptr;

    std::unique_ptr<ISceneTransition> transition_ = nullptr;

    std::unique_ptr<SceneData> sceneData_ = nullptr;

    bool isTransition_ = false;

    std::queue<std::unique_ptr<ISceneTransition>> transitionQueue_ = {};

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
