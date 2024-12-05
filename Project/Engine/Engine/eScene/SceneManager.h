#pragma once
#include "BaseScene.h"

#include <iostream>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>

using SceneFactory = std::unique_ptr<BaseScene>(*)();

class SceneManager
{
public:
    static SceneManager* GetInstance();

    ~SceneManager();

    // シーンの登録
    // _name : シーンの名前
    // _scene : シーンの生成関数
    // 例 : SceneManager::RegisterScene("game", Game::Create);
    static void RegisterScene(const std::string& _name, SceneFactory _scene);

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

    // シーンのリスト
    std::unordered_map<std::string, SceneFactory>   scenes_ = {};
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