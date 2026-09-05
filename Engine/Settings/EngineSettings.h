#pragma once

#include <cstdint>
#include <string>

/// <summary>
/// エンジン設定データ構造体。
/// </summary>

namespace Engine {

struct EngineConfig
{
    std::wstring windowTitle = L"GameEngine"; // ウィンドウタイトル

    // ゲームの描画解像度。Screen:: が返す値
    // カメラのアスペクト比や2Dの正射影の基準になるので，Debug と Release で変えないこと
    uint32_t renderWidth = 1280;
    uint32_t renderHeight = 720;

    // Releaseビルドでの OSウィンドウのサイズ
    uint32_t windowWidth = 1280;
    uint32_t windowHeight = 720;

    // Debugビルドでの OSウィンドウのサイズ
    // Hierarchy や Inspector を並べても Game ビューが潰れないよう広めに取る
    uint32_t debugWindowWidth = 1920;
    uint32_t debugWindowHeight = 1080;

    bool enableVSync = true;                  // VSync有効化
    uint32_t targetFPS = 60;                  // 目標FPS

    /// <summary> 現在のビルド構成でのウィンドウ幅 </summary>
    uint32_t GetWindowWidth() const;
    /// <summary> 現在のビルド構成でのウィンドウ高さ </summary>
    uint32_t GetWindowHeight() const;
};

/// <summary>
/// エンジン設定管理クラス。
/// </summary>
class EngineSettings
{
public:
    static EngineConfig current_;   // 現在の設定
    static EngineConfig default_;   // デフォルト設定

    /// <summary>
    /// 設定をファイルから読み込む。
    /// ファイルが存在しない場合はデフォルト値で新規作成する。
    /// </summary>
    /// <param name="filePath">読み込むファイルパス（デフォルト値あり）</param>
    static void Load(const std::string& filePath = "Resources/Engine/engine_config.json");

    /// <summary>
    /// 設定をファイルへ保存する。
    /// </summary>
    /// <param name="filePath">保存先ファイルパス（デフォルト値あり）</param>
    static void Save(const std::string& filePath = "Resources/Engine/engine_config.json");
};

} // namespace Engine
