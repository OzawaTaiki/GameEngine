#pragma once
#include <Features/Scene/Interface/BaseScene.h>
#include <Features/Scene/ISceneFactory.h>
#include <Features/Scene/Interface/ISceneTransition.h>
#include <Features/Scene/SceneData.h>

#include <cstdint>
#include <string>
#include <memory>
#include <queue>


class SceneManager
{
public:
    static SceneManager* GetInstance();

    // 初期化
    // _name : 初期化するシーンの名前 ファクトリ参照
    void Initialize(const std::string& _name);
    // 更新
    void Update();
    // 描画
    void Draw();
    // 影の描画
    void DrawShadow();

    // シーンの遷移を設定
    void SetTransition(std::unique_ptr<ISceneTransition> _transition);

    // transitionの有効化設定
    void EnableTransition(bool _enable) { enableTransition_ = _enable; }

    // シーンファクトリの設定
    void SetSceneFactory(std::unique_ptr<ISceneFactory> _sceneFactory);

    // シーンの予約
    // _name : 予約するシーンの名前 ファクトリ参照
    static void ReserveScene(const std::string& _name, std::unique_ptr<SceneData> _sceneData);

    // シーンの変更
    static void ChangeScene();

    // 終了処理
    void Finalize();

private:
    // シーンファクトリ
    std::unique_ptr<ISceneFactory> sceneFactory_ = nullptr;

    // 現在のシーン
    std::unique_ptr<BaseScene> currentScene_ = nullptr;

    // シーンの遷移
    std::unique_ptr<ISceneTransition> transition_ = nullptr;

    // transitionを無効化するためのフラグ
    bool enableTransition_ = true;

    // シーン間の受け渡しデータ
    std::unique_ptr<SceneData> sceneData_ = nullptr;

    // シーンの遷移中かどうか
    bool isTransition_ = false;

    // シーンの遷移を待機するキュー
    std::queue<std::unique_ptr<ISceneTransition>> transitionQueue_ = {};

    // 現在のシーン名
    std::string currentSceneName_ = {};
    // 次のシーン名
    std::string nextSceneName_ = {};

#ifdef _DEBUG
    void ImGui(bool* _open);
#endif // _DEBUG


private:
    // コンストラクタとデストラクタ
    SceneManager();
    ~SceneManager();

    // コピー禁止
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;
    SceneManager(SceneManager&&) = delete;
    SceneManager& operator=(SceneManager&&) = delete;

};
